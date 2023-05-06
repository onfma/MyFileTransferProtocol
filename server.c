#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h> 
#include <fcntl.h>   
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <stdbool.h>

#define BAZA_DE_DATE "users.txt" // STATUS  PERMISIUNI  USERNAME  PASSWORD
#define PORT 9981
#define BUFFER_SIZE 1000

#define WHITELIST 1
#define BLACKLIST 0 

#define ADMIN 1
#define NORMAL_USER 0 

struct utilizator
{
    int id; // unic pt fiecare client; reprezinta linia din fisier la care se afla datele despre el
    int status; // orice client nou are statut de WHITELIST
    int permisiuni; // orice client nou are permisiune de NORMAL USER
    int parola;
    char username[BUFFER_SIZE];

}clienti[500];

int count_clienti = 0;

void initializare_utilizatori_existenti()
{
    count_clienti = 0;

    FILE *bd = fopen(BAZA_DE_DATE, "r"); // dechidere BD cu drept de r only
    if(bd != NULL)
    {
        char info[BUFFER_SIZE];

        while(fgets(info, BUFFER_SIZE, bd) != NULL)
        {
            count_clienti++;
            clienti[count_clienti].id = count_clienti;

            char information[BUFFER_SIZE];
            strcpy(information,info);
            int nr_space = 0, poz_start = 0, poz_finish = 0;
            for(int i = 0; i < strlen(information); i++)
            {
                if(information[i] == ' ')
                {
                    poz_finish = i-1;
                    nr_space++;
                    switch(nr_space)
                    {
                    case 1: 
                        if(information[i-1] == '1') clienti[count_clienti].status = WHITELIST;
                        else clienti[count_clienti].status = BLACKLIST;
                        break;

                    case 2:
                        if(information[i-1] == '1') clienti[count_clienti].permisiuni = ADMIN;
                        else clienti[count_clienti].permisiuni = NORMAL_USER;
                        break;

                    case 3:
                    {
                        char index = 0;
                        for(int j = poz_start; j <= poz_finish; j++)
                        {
                            clienti[count_clienti].username[index++] = information[j];
                        }
                        clienti[count_clienti].username[index] = '\0';

                        index = 0;
                        clienti[count_clienti].parola = 0;
                        char aux[BUFFER_SIZE]; int nr = 0;
                        for(int j = poz_finish + 2; j < strlen(information); j++)
                        {
                            aux[index++] = information[j];
                        }
                        aux[index-1] = '\0';
                        int ii;
                        for(ii = 0; ii < strlen(aux) - 1; ii++)
                        {
                            nr = nr + aux[ii] - '0';
                            nr = nr * 10;
                        }
                        nr = nr + aux[ii] - '0';
                        clienti[count_clienti].parola = nr;
                        break;
                    } 

                    default:
                        break;
                    }
                    poz_start=i+1;
                }
            }
            //printf("%d, %d, %d, %s, %d,\n",clienti[count_clienti].id, clienti[count_clienti].status, clienti[count_clienti].permisiuni, clienti[count_clienti].username, clienti[count_clienti].parola);
        }
    }
    //printf("%d\n",count_clienti);
    fclose(bd);
}

void register_client_nou(int client)
{
    char msg[BUFFER_SIZE];
    char username[100];
    int password = 0;

    int id_client = ++count_clienti;
    clienti[id_client].id =id_client;
    clienti[id_client].status = WHITELIST; // orice client nou are WHITELIST
    clienti[id_client].permisiuni = NORMAL_USER; // orice client nou are NORMAL_USER
        
    // se trimite "creati username" -- (3)
    bzero(msg, BUFFER_SIZE);
    strcpy(msg,"va rugam creati un username: ");
    if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}

    // se primeste raspunsul "username" -- (4)
    bzero(username, 100);
    if(read(client, username, 100) <= 0) {perror("[server]Eroare la read() de la client.\n");close(client);}
    printf("[server]Username-ul a fost receptionat...%s\n", username); username[strlen(username)-1]='\0';
    strcpy(clienti[id_client].username,username);

    // se trimite "creati password" -- (5)
    bzero(msg, BUFFER_SIZE);
    strcpy(msg,"va rugam creati o parola: ");
    if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}

    // se primeste raspunsul "password" -- (6)
    //bzero(password, 100);
    if(read(client, &password, sizeof(password)) <= 0) {perror("[server]Eroare la read() de la client.\n");close(client);}
    printf("[server]Password-ul a fost receptionat...%d\n", password);
    clienti[id_client].parola = password;

    // COD TRANSMITERE INFORMATII IN BAZA_DE_DATE
    FILE *bd = fopen(BAZA_DE_DATE, "a+"); // dechidere BD cu drept de wr
    if(bd != NULL)
    {
        fprintf(bd,"%d %d %s %d\n", clienti[id_client].status, clienti[id_client].permisiuni, clienti[id_client].username, clienti[id_client].parola);
    }
    else
    {
        perror("[server]Eroare la deschidere baza de date\n"); exit(1);
    }
    fclose(bd);
    // FINAL COD TRANSMITERE INFORMATII IN BAZA_DE_DATE

    // se trimite "succes + doriti sa va logati? [y/n]" -- (7)
    bzero(msg, BUFFER_SIZE);
    strcpy(msg,"cont creat cu SUCCES\ndoriti sa va logati? [y/n]\n");
    if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
}

int login(int client)
{
    int id_client;
    char msg[BUFFER_SIZE];
    char username[100];
    int password1 = 0;

    // se trimite "introduceti username" -- (9)
    bzero(msg, BUFFER_SIZE);
    strcpy(msg,">>LOGARE<<\nva rugam introduceti username-ul: ");
    if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}

    // se primeste raspunsul "username" -- (10)
    bzero(username, 100);
    if(read(client, username, 100) <= 0) {perror("[server]Eroare la read() de la client.\n");close(client);}
    printf("[server]Username-ul a fost receptionat...%s\n", username); username[strlen(username)-1]='\0';

    // se trimite "introduceti password" -- (11)
    bzero(msg, BUFFER_SIZE);
    strcpy(msg,"va rugam introduceti parola: ");
    if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}

    // se primeste raspunsul "password" -- (12)
    //bzero(password, 100);
    if(read(client, &password1, sizeof(password1)) <= 0) {perror("[server]Eroare la read() de la client.\n");close(client);}
    printf("[server]Password-ul a fost receptionat...%d,\n", password1);

    // COD VERIFICARE VALIDARE USERNAME + PASSWORD
    bool client_valid = false;
    for(int i = 1; i <= count_clienti; i++)
    {
        if(strcmp(clienti[i].username,username) == 0 && clienti[i].parola == password1) client_valid = true, id_client = i;
    }
    // FINAL COD VERIFICARE VALIDARE USERNAME + PASSWORD

    if(client_valid)
    {
        // se trimite "succes + introduceti comanda" -- (13)
        bzero(msg, BUFFER_SIZE);
        strcpy(msg,"cont logat cu SUCCES\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
        if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
        return id_client;
    }
    else
    {
        // se trimite "logare nereusita" -- (13)
        bzero(msg, BUFFER_SIZE);
        strcpy(msg,"logare NEREUSITA\nusername si parola incorecte\ndoriti sa reincercati logarea? [y/n]\n");
        if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}

        // se primeste raspunsul "y/n" -- (14)
        bzero(msg, BUFFER_SIZE);
        if(read(client, msg, BUFFER_SIZE) <= 0) {perror("[server]Eroare la read() de la client.\n");close(client);}
        printf("[server]Mesajul a fost receptionat...%s\n", msg);

        if(strncmp(msg,"n",1) == 0) close(client);
        else if(strncmp(msg,"y",1) == 0) login(client);
    }
}

void comenzi_posibile(int client) // BLACKLIST
{
    char msg[BUFFER_SIZE];

    // se trimite "comenzi posibile + introduceti comanda" -- (comenzi posibile 1)
    bzero(msg, BUFFER_SIZE);
    strcpy(msg,"\n\n>>COMENZI POSIBILE<<\nproprietati\nshow dir\nread file\ndelete file\ndelete dir\ncreate file\ncreate dir\nlogout");
    strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
    if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
}

void create_dir(int client, int statut)
{
    char msg[BUFFER_SIZE];

    if(statut == WHITELIST)
    {
        char path[BUFFER_SIZE];

        // se trimite "introduceti path/nume pt creare director" -- (create dir 1)
        bzero(msg, BUFFER_SIZE);
        strcpy(msg,"introduceti path-ul/numele directorului pe care vreti sa il creati\n");
        if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}

        // se primeste path-ul -- (create dir 2)
        bzero(path, BUFFER_SIZE);
        if(read(client, path, BUFFER_SIZE) <= 0) {perror("[server]Eroare la read() de la client.\n");close(client);}
        printf("[server]Mesajul a fost receptionat...%s\n", path);
        path[strlen(path)-1]='\0';

        //            CREARE DIRECTOR            //
        if(mkdir(path, 0777) == -1)
        {
            // se trimite "eroare la creare + introd comanda" -- (create dir 3)
            bzero(msg, BUFFER_SIZE);
            strcpy(msg,"EROARE la crearea directorului");
            strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
            if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
        }
        else
        {
            // se trimite "succes la creare + introd comanda" -- (create dir 3)
            bzero(msg, BUFFER_SIZE);
            strcpy(msg,"director creat cu SUCCES");
            strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
            if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
        }
    }
    else if(statut == BLACKLIST)
    {
        // se trimite "blacklist" -- (create dir 1)
        bzero(msg, BUFFER_SIZE);
        strcpy(msg,"\n\nBLACKLIST\nnu puteti executa aceasta comanda");
        strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
        if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
    }
}

void create_file(int client, int statut)
{
    char msg[BUFFER_SIZE];

    if(statut == WHITELIST)
    {
        char nume[BUFFER_SIZE];
        char path[BUFFER_SIZE];

        // se trimite "introduceti numele pt creare fisier" -- (create file 1)
        bzero(msg, BUFFER_SIZE);
        strcpy(msg,"introduceti numele fisierului pe care vreti sa il creati\n");
        if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}

        // se primeste numele -- (create file 2)
        bzero(nume, BUFFER_SIZE);
        if(read(client, nume, BUFFER_SIZE) <= 0) {perror("[server]Eroare la read() de la client.\n");close(client);}
        printf("[server]Mesajul a fost receptionat...%s\n", nume);

        // se trimite "introduceti path pt creare director" -- (create file 3)
        bzero(msg, BUFFER_SIZE);
        strcpy(msg,"introduceti path-ul directorului pe care vreti sa il creati\n");
        if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}

        // se primeste path-ul -- (create file 4)
        bzero(path, BUFFER_SIZE);
        if(read(client, path, BUFFER_SIZE) <= 0) {perror("[server]Eroare la read() de la client.\n");close(client);}
        printf("[server]Mesajul a fost receptionat...%s\n", path);
        path[strlen(path)-1]='\0';
        strcat(path, "/"); strncat(path, nume, strlen(nume)-1); // se pune cap la cap comanda

        //            CREARE FILE            //
        if(creat(path, 0644) == -1)
        {
            // se trimite "eroare la creare + introd comanda" -- (create file 5)
            bzero(msg, BUFFER_SIZE);
            strcpy(msg,"EROARE la crearea fisierului");
            strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
            if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
        }
        else
        {
            // se trimite "succes la creare + introd comanda" -- (create file 5)
            bzero(msg, BUFFER_SIZE);
            strcpy(msg,"fisier creat cu SUCCES");
            strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
            if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
        }
    }
    else if(statut == BLACKLIST)
    {
        // se trimite "blacklist" -- (create file 1)
        bzero(msg, BUFFER_SIZE);
        strcpy(msg,"\n\nBLACKLIST\nnu puteti executa aceasta comanda");
        strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
        if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
    }
}

void delete_file(int client, int statut)
{
    char msg[BUFFER_SIZE];

    // COD VERIFICARE STATUS WHITELIST

    if(statut == WHITELIST)
    {
        char path[BUFFER_SIZE];

        // se trimite "introduceti path pt delete director/fisier" -- (delete 1)
        bzero(msg, BUFFER_SIZE);
        strcpy(msg,"introduceti path-ul fisierului pe care vreti sa il stergeti\n");
        if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}

        // se primeste path-ul -- (delete 2)
        bzero(path, BUFFER_SIZE);
        if(read(client, path, BUFFER_SIZE) <= 0) {perror("[server]Eroare la read() de la client.\n");close(client);}
        printf("[server]Mesajul a fost receptionat...%s\n", path);
        path[strlen(path)-1]='\0';

        //            DELETE            //
        if(remove(path) == -1)
        {
            // se trimite "eroare la creare + introd comanda" -- (delete 3)
            bzero(msg, BUFFER_SIZE);
            strcpy(msg,"EROARE la stergere");
            strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
            if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
        }
        else
        {
            // se trimite "succes la creare + introd comanda" -- (delete 3)
            bzero(msg, BUFFER_SIZE);
            strcpy(msg,"stergere cu SUCCES");
            strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
            if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
        }
    }
    else if(statut == BLACKLIST)
    {
        // se trimite "blacklist" -- (create dir 1)
        bzero(msg, BUFFER_SIZE);
        strcpy(msg,"\n\nBLACKLIST\nnu puteti executa aceasta comanda");
        strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
        if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
    }
}

void delete_dir(int client, int statut)
{
    char msg[BUFFER_SIZE];

    if(statut == WHITELIST)
    {
        char path[BUFFER_SIZE];

        // se trimite "introduceti path pt delete director/fisier" -- (delete 1)
        bzero(msg, BUFFER_SIZE);
        strcpy(msg,"introduceti path-ul directorului pe care vreti sa il stergeti, nu uitati ca trebuie sa fie GOL\n");
        if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}

        // se primeste path-ul -- (delete 2)
        bzero(path, BUFFER_SIZE);
        if(read(client, path, BUFFER_SIZE) <= 0) {perror("[server]Eroare la read() de la client.\n");close(client);}
        printf("[server]Mesajul a fost receptionat...%s\n", path);
        path[strlen(path)-1]='\0';

        //            DELETE            //
        if(rmdir(path) == -1)
        {
            // se trimite "eroare la creare + introd comanda" -- (delete 3)
            bzero(msg, BUFFER_SIZE);
            strcpy(msg,"EROARE la stergere");
            strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
            if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
        }
        else
        {
            // se trimite "succes la creare + introd comanda" -- (delete 3)
            bzero(msg, BUFFER_SIZE);
            strcpy(msg,"stergere cu SUCCES");
            strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
            if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
        }
    }
    else if(statut == BLACKLIST)
    {
        // se trimite "blacklist" -- (create dir 1)
        bzero(msg, BUFFER_SIZE);
        strcpy(msg,"\n\nBLACKLIST\nnu puteti executa aceasta comanda");
        strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
        if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
    }
}

void properties_of(int client) // BLACKLIST
{
    char msg[BUFFER_SIZE];
    char path[BUFFER_SIZE];

    // se trimite "introduceti path-ul pt director/fisier" -- (prop 1)
    bzero(msg, BUFFER_SIZE);
    strcpy(msg,"introduceti path-ul directorului/fisierului pe care vreti sa il sergeti\n");
    if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}

    // se primeste path-ul -- (prop 2)
    bzero(path, BUFFER_SIZE);
    if(read(client, path, BUFFER_SIZE) <= 0) {perror("[server]Eroare la read() de la client.\n");close(client);}
    printf("[server]Mesajul a fost receptionat...%s\n", path);
    path[strlen(path)-1]='\0';

    //            PROPERTIES            //
    struct stat str;

    if(stat(path, &str) == -1)
    {
        // se trimite eroare -- (prop 3)
        bzero(msg, BUFFER_SIZE);
        strcpy(msg,"EROARE la generarea de proprietati\n");
        strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
        if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
    }
    else
    {
        char aux[BUFFER_SIZE];

        // se trimite prop -- (prop 3)
        bzero(msg, BUFFER_SIZE);
        strcpy(msg, "\n\n");

        strcat(msg, "size: ");
        bzero(aux, BUFFER_SIZE); sprintf(aux, "%ld", str.st_size); // cast din long int in char
        strcat(msg, aux);
        strcat(msg, "\n");

        strcat(msg, "id owner: ");
        bzero(aux, BUFFER_SIZE); sprintf(aux, "%ld", (long)str.st_uid);
        strcat(msg, aux);
        strcat(msg, "\n");

        strcat(msg, "group id owner: ");
        bzero(aux, BUFFER_SIZE); sprintf(aux, "%ld", (long)str.st_gid);
        strcat(msg, aux);
        strcat(msg, "\n");

        strcat(msg, "blocks: "); 
        bzero(aux, BUFFER_SIZE); sprintf(aux, "%ld", (long)str.st_blocks);
        strcat(msg, aux);
        strcat(msg, "\n");

        strcat(msg, "device: ");
        bzero(aux, BUFFER_SIZE); sprintf(aux, "%ld", (long)str.st_dev);
        strcat(msg, aux);
        strcat(msg, "\n");

        strcat(msg, "links: "); 
        bzero(aux, BUFFER_SIZE); sprintf(aux, "%ld", (long)str.st_nlink);
        strcat(msg, aux);
        strcat(msg, "\n");

        strcat(msg, "IO block: "); 
        bzero(aux, BUFFER_SIZE); sprintf(aux, "%ld", str.st_blksize);
        strcat(msg, aux);
        strcat(msg, "\n");

        strcat(msg, "inode: "); 
        bzero(aux, BUFFER_SIZE); sprintf(aux, "%ld", (long)str.st_ino);
        strcat(msg, aux);
        strcat(msg, "\n");

        strcat(msg, "ultimul acces: ");
        strcat(msg, ctime(&str.st_atime));

        strcat(msg, "ultimul modificare: ");
        strcat(msg, ctime(&str.st_mtime));

        //tipul de fisier
        if((str.st_mode & S_IFMT) == S_IFBLK)
        {
            strcat(msg, "tipul fisierului este block device."); 
            strcat(msg, "\n");
        }
        if((str.st_mode & S_IFMT) == S_IFCHR)
        {
            strcat(msg, "tipul fisierului este character device.");
            strcat(msg, "\n");
        }
        if((str.st_mode & S_IFMT) == S_IFDIR)
        {
            strcat(msg, "tipul fisierului este directory.");
            strcat(msg, "\n");
        }
        if((str.st_mode & S_IFMT) == S_IFIFO)
        {
            strcat(msg, "tipul fisierului este FIFO sau pipe.");
            strcat(msg, "\n");
        }
        if((str.st_mode & S_IFMT) == S_IFLNK)
        {
            strcat(msg, "tipul fisierului este symlink.");
            strcat(msg, "\n");
        }
        if((str.st_mode & S_IFMT) == S_IFREG)
        {
            strcat(msg, "tipul fisierului este regular file.");
            strcat(msg, "\n");
        }
        if((str.st_mode & S_IFMT) == S_IFSOCK)
        {
            strcat(msg, "tipul fisierului este socket.");
            strcat(msg, "\n");
        }

        strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
        if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
    }
}

void read_file(int client) // BLACKLIST
{
    char msg[BUFFER_SIZE];
    char path[BUFFER_SIZE];

    // se trimite "introduceti path fisier" -- (read file 1)
    bzero(msg, BUFFER_SIZE);
    strcpy(msg,"introduceti path-ul fisierului pe care vreti sa il deschideti\n");
    if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}

    // se primeste path-ul -- (read file 2)
    bzero(path, BUFFER_SIZE);
    if(read(client, path, BUFFER_SIZE) <= 0) {perror("[server]Eroare la read() de la client.\n");close(client);}
    printf("[server]Mesajul a fost receptionat...%s\n", path);
    path[strlen(path)-1]='\0';

    //            READ FILE            //
    FILE *file = fopen(path, "r"); // dechidere file cu drept de r only
    if(file == NULL)
    {
        // se trimite "eroare la creare + introd comanda" -- (read file 3)
        bzero(msg, BUFFER_SIZE);
        strcpy(msg,"EROARE la deschiderea fisierului");
        strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
        if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
    }
    else
    {
        char continut[BUFFER_SIZE]; 

        // se trimite "CONTINUT + introd comanda" -- (read file 3)
        bzero(msg, BUFFER_SIZE);
        while(fgets(continut, BUFFER_SIZE, file) != NULL)
        {
            strcat(msg,continut);
        }
        strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
        if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
        
    }
    fclose(file);
    
}

void show_dir(int client) // BLACKLIST
{
    char msg[BUFFER_SIZE];
    char path[BUFFER_SIZE];

    // se trimite "introduceti path-ul pt director" -- (show dir 1)
    bzero(msg, BUFFER_SIZE);
    strcpy(msg,"introduceti path-ul directorului pe care vreti sa il afisati ca si continut\n");
    if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}

    // se primeste path-ul -- (show dir 2)
    bzero(path, BUFFER_SIZE);
    if(read(client, path, BUFFER_SIZE) <= 0) {perror("[server]Eroare la read() de la client.\n");close(client);}
    printf("[server]Mesajul a fost receptionat...%s\n", path);
    path[strlen(path)-1]='\0';

    //            CONTENT DIR            //
    struct dirent *directory; 
    DIR *dir = opendir(path); // se dechide directorul cerut de client

    if(dir == NULL)
    {
        // se trimite eroare -- (show dir 3)
        bzero(msg, BUFFER_SIZE);
        strcpy(msg,"EROARE la deschiderea directorului\n");
        strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
        if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
    }
    else
    {
        // se trimite continutul directorului -- (show dir 3)
        bzero(msg, BUFFER_SIZE);
        while((directory = readdir(dir)) != NULL) // --------------------- VEZI CARE E TREABA PE AICI
        {
            if (strcmp(directory->d_name, ".") != 0 && strcmp(directory->d_name, "..") != 0)
            {
                strcat(msg, "\n");
                strcat(msg, directory->d_name);
            }
        }
        closedir(dir);
        strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
        if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}

    }
}

void move_file(int client, int statut)
{
    char msg[BUFFER_SIZE];

    if(statut == WHITELIST)
    {
        char nume[BUFFER_SIZE];
        char path[BUFFER_SIZE];

        // se trimite "introduceti numele fisierului pt mutare " -- (move file 1)
        bzero(msg, BUFFER_SIZE);
        strcpy(msg,"introduceti numele fisierului pe care vreti sa il mutati\n");
        if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}

        // se primeste numele -- (create file 2)
        bzero(nume, BUFFER_SIZE);
        if(read(client, nume, BUFFER_SIZE) <= 0) {perror("[server]Eroare la read() de la client.\n");close(client);}
        printf("[server]Mesajul a fost receptionat...%s\n", nume);
        nume[strlen(nume)-1]='\0';

        // se trimite "introduceti path pt mutare fisier" -- (create file 3)
        bzero(msg, BUFFER_SIZE);
        strcpy(msg,"introduceti path-ul directorului in care vreti sa mutati fisierul\n");
        if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}

        // se primeste path-ul -- (create file 4)
        bzero(path, BUFFER_SIZE);
        if(read(client, path, BUFFER_SIZE) <= 0) {perror("[server]Eroare la read() de la client.\n");close(client);}
        printf("[server]Mesajul a fost receptionat...%s\n", path);
        path[strlen(path)-1]='\0';

        //            MUTARE FILE            //
        char c;
        bool erori = false;

        FILE *sursa = fopen(nume,"r");
        if(sursa == NULL) erori = true, printf("\neroare la deschidere file initial\n");

        FILE *destinatie = fopen(path, "w+");
        if(destinatie == NULL) erori = true, printf("\neroare la deschidere file destinatie\n");

        strcat(path, "/"); strncat(path, nume, strlen(nume)-1); // se pune cap la cap comanda

        int copie = creat(path, 0644);
        if(copie == -1) erori = true, printf("\neroare la creare copie\n");

        int stergere = remove(nume);
        if(stergere != 0) erori = true, printf("\neroare la stergere original\n");

        if(!erori)
        {
            while ((c = fgetc(sursa)) != EOF) fputc(c, destinatie);

            // se trimite "succes la mutare + introd comanda" -- (create file 5)
            bzero(msg, BUFFER_SIZE);
            strcpy(msg,"fisier mutat cu SUCCES");
            strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
            if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
        }
        else
        {
            // se trimite "EROARE + introd comanda" -- (create file 5)
            bzero(msg, BUFFER_SIZE);
            strcpy(msg,"EROARE mutarea fisierului nu a reusit");
            strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
            if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
        }
    }
    else if(statut == BLACKLIST)
    {
        // se trimite "blacklist" -- (move file 1)
        bzero(msg, BUFFER_SIZE);
        strcpy(msg,"\n\nBLACKLIST\nnu puteti executa aceasta comanda");
        strcat(msg,"\n\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
        if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");}
    }
}

int main()
{
    struct sockaddr_in server; // structura specifica server specifica fam AF_INET
    struct sockaddr_in from;   // structura specifica client
    char msg[BUFFER_SIZE];           // mesajul primit de la client
    char msgrasp[BUFFER_SIZE] = " "; // mesaj de raspuns pentru client

    int socketfd;                  

    // crearea unui socket - mijloc de comunicare bidirectional
    // socket(familia de protocoale,type,protocol)
    // SOCK_STREAM - dem faptul ca e TCP
    // SOCK_DGRAM - specifica UDP
    if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {perror("[server] eroare la creare socket\n"); exit(1);} 

    // pregatirea structurilor de date
    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    server.sin_family = AF_INET;                // familia socketului - familia specifica IPv4
    server.sin_addr.s_addr = htonl(INADDR_ANY); // accepta orice adresa IP - transforma o val long de la ord. host la ord. network
    server.sin_port = htons(PORT);              // asigneare port - little/big endian => obligatoriu big endian

    // se leaga socketul cu IP-ul 
    if(bind(socketfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {perror("[server] eroare la binding\n"); exit(1);}

    // serverul asculta dupa clienti
    // def: trimite socket-ul intr-o stare de asteptare a conectarii clientilor, conexiunile (max 5) vor fi plasate intr-o coada pana la accept
    if(listen(socketfd, 5) == -1) {perror("[server] eroare la listen\n"); exit(1);}

    initializare_utilizatori_existenti(); // se salveaza in count_clienti cati utilizatori au conturi deja create si gata de logare directa

    //se servesc clientii in mod CONCURENT
    while(1)
    {
        int client;
        int lungime = sizeof(from);
        int id_client = -1;

        // acceptare client - se creaza un nou socket pt transferul efectiv de date
        if((client = accept(socketfd, (struct sockaddr *)&from, &lungime)) < 0) {perror("[server] eroare la acceptare\n"); exit(1);}

        int pid;
        if ((pid = fork()) == -1) {perror("[server] eroare la fork\n"); close(client); continue;}
        else if (pid > 0) // PARINTE
        {
            close(client);
            while (waitpid(-1, NULL, WNOHANG)); // NU blocheaza celelalte cereri pana cand procestul copil este gata
            continue;
        }
        else if (pid == 0) // COPIL
        {
            close(socketfd);

            // se trimite "cont existent?" -- (1)
            bzero(msg, BUFFER_SIZE);
            strcpy(msg,"cont existent? [y/n]\n");
            if(write(client, msg, strlen(msg)) <= 0) {perror("[server]Eroare la write() catre client.\n");continue;}

            // se primeste raspunsul "y/n" -- (2)
            bzero(msg, BUFFER_SIZE);
            if(read(client, msg, BUFFER_SIZE) <= 0) {perror("[server]Eroare la read() de la client.\n");close(client);continue;}
            printf("[server]Mesajul a fost receptionat...%s\n", msg);

            //            REGISTER client nou            //
            if(strncmp(msg,"n",1) == 0)
            {
                register_client_nou(client);
                
                // se primeste raspunsul "y/n" de la "doriti sa va logati?" -- (8)
                bzero(msg, BUFFER_SIZE);
                if(read(client, msg, BUFFER_SIZE) <= 0) {perror("[server]Eroare la read() de la client.\n");close(client);continue;}
                printf("[server]Mesajul a fost receptionat...%s\n", msg);
                if(strncmp(msg,"n",1) == 0) {close(client); continue;} // daca clientul zice nu atunci este deconectat
            }

            //            LOGARE            //
            initializare_utilizatori_existenti();
            id_client = login(client);

            //            EXECUTARE COMENZI            //
            char comanda[BUFFER_SIZE];

            while(1)
            {
                // se primeste o comanda -- (15)
                bzero(comanda, BUFFER_SIZE);
                if(read(client, comanda, BUFFER_SIZE) <= 0) {perror("[server]Eroare la read() de la client.\n");close(client);continue;}
                printf("[server]Mesajul a fost receptionat...%s\n", comanda);

                if(strncmp(comanda, "proprietati", strlen("proprietati")) == 0) properties_of(client); 
                else if(strncmp(comanda, "read file", strlen("read file")) == 0) read_file(client); 
                else if(strncmp(comanda, "show dir", strlen("show dir")) == 0) show_dir(client); 
                else if(strncmp(comanda, "delete file", strlen("delete file")) == 0) delete_file(client, clienti[id_client].status);
                else if(strncmp(comanda, "delete dir", strlen("delete dir")) == 0) delete_dir(client, clienti[id_client].status);
                else if(strncmp(comanda, "create file", strlen("create file")) == 0) create_file(client, clienti[id_client].status); 
                else if(strncmp(comanda, "create dir", strlen("create dir")) == 0) create_dir(client, clienti[id_client].status); 
                else if(strncmp(comanda, "move file", strlen("move file")) == 0) move_file(client, clienti[id_client].status);
                else if(strncmp(comanda, "comenzi posibile", strlen("comenzi posibile")) == 0) comenzi_posibile(client); 
                else if(strncmp(comanda, "logout", strlen("logout")) == 0)
                {
                    // se inchide socketul dupa ce se deconecteaza clientul
                    close(client); 
                    break;
                }
                else // comanda invalida
                {
                    bzero(msgrasp, BUFFER_SIZE);
                    strcat(msgrasp, "COMANDA INVALIDA\nComanda sugerata: << comenzi posibile >>\nINTRODUCETI O COMANDA: ");
                    if(send(client, msgrasp, BUFFER_SIZE, 0) <= 0) {perror("[server] eroare la write() catre client.\n"); continue;}
                    else printf("[server] mesajul a fost trasmis cu succes.\n");
                }
            }
        }
    }
}

/*Sa se implementeze o aplicatie client/server ce permite transferul de fisiere intre clienti si server. 
Serverul va pune la dispozitia clientilor un numar minim de comenzi ce permit autentificarea, 
operarea cu directoare si cu fisiere. De asemenea, va trebui implementat un mecanism de 
autorizare (whitelist/blacklist) pentru conturile utilizatorilor si un mecanism de transmitere 
securizata a parolei la autentificare.*/