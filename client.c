#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h> 
#include <sys/socket.h> 
#include <termio.h>
#include <termios.h>
#include <stdbool.h>

#define PORT 9981
#define BUFFER_SIZE 1000

void transmitere_parola(int socketfd)
{
    char password[100];

    // se citeste de la tastatura si se trimite "password" -- (12)
    bzero(password, 100);
    fflush(stdout);
    read(0, password, 100);

    // SECURIZARE PAROLA
    int hash_password = 0;
    int salt = 15665841;
    for(int i = 0; i <= strlen(password); i++)
    {
        hash_password = hash_password ^ password[i]; // disjunctie exclusivă pe biti
        hash_password = hash_password * salt;
    }
    if(hash_password < 0) hash_password = hash_password * (0-1);
    // FINAL SECURIZARE PAROLA
    if(send(socketfd, &hash_password, sizeof(hash_password), 0) <= 0) {perror("[client] eroare la write() spre server\n"); exit(1);}
}

void register_client_nou(int socketfd)
{
    char msg[BUFFER_SIZE];
    char msgrasp[BUFFER_SIZE];
    char username[100];
    char password[100];

    // se primeste "creati username" -- (3)
    bzero(msg, BUFFER_SIZE);
    if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
    printf("%s", msg);

    // se citeste de la tastatura si se trimite "username" -- (4)
    bzero(username, 100);
    fflush(stdout);
    read(0, username, 100);
    if(send(socketfd, username, strlen(username), 0) <= 0) {perror("[client] eroare la write() spre server\n"); exit(1);}

    // se primeste "creati password" -- (5)
    bzero(msg, BUFFER_SIZE);
    if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
    printf("%s", msg);

    transmitere_parola(socketfd);
    /*
    // se citeste de la tastatura si se trimite "password" -- (6)
    bzero(password, 100);
    fflush(stdout);
    read(0, password, 100);
    if(send(socketfd, password, strlen(password), 0) <= 0) {perror("[client] eroare la write() spre server\n"); exit(1);}
    */

    // se primeste "succes + doriti sa va logati" -- (7)
    bzero(msg, BUFFER_SIZE);
    if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
    printf("%s", msg);

    // se citeste de la tastatura si se trimite "y/n" -- (8)
    bzero(msgrasp, BUFFER_SIZE);
    fflush(stdout);
    read(0, msgrasp, BUFFER_SIZE);
    if(send(socketfd, msgrasp, strlen(msgrasp), 0) <= 0) {perror("[client] eroare la write() spre server\n"); exit(1);}

    if(strncmp(msgrasp,"n",1) == 0) close(socketfd), exit(0);
}

void login(int socketfd)
{
    char msg[BUFFER_SIZE];
    char msgrasp[BUFFER_SIZE];
    char username[100];

    // se primeste "introduceti username" -- (9)
    bzero(msg, BUFFER_SIZE);
    if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
    printf("%s", msg);

    // se citeste de la tastatura si se trimite "username" -- (10)
    bzero(username, 100);
    fflush(stdout);
    read(0, username, 100);
    if(send(socketfd, username, strlen(username), 0) <= 0) {perror("[client] eroare la write() spre server\n"); exit(1);}

    // se primeste "introduceti password" -- (11)
    bzero(msg, BUFFER_SIZE);
    if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
    printf("%s", msg);

    transmitere_parola(socketfd);
    /*
    // se citeste de la tastatura si se trimite "password" -- (12)
    bzero(password, 100);
    fflush(stdout);
    read(0, password, 100);
    if(send(socketfd, password, strlen(password), 0) <= 0) {perror("[client] eroare la write() spre server\n"); exit(1);}
    */

    // se primeste "logare nereusita / succes + introduceti comanda" -- (13)
    bzero(msg, BUFFER_SIZE);
    if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
    printf("%s", msg);

    if(strncmp(msg,"logare NEREUSITA",strlen("logare NEREUSITA")) == 0)
    {
        // se citeste de la tastatura si se trimite "y/n" -- (14)
        bzero(msgrasp, BUFFER_SIZE);
        fflush(stdout);
        read(0, msgrasp, BUFFER_SIZE);
        if(send(socketfd, msgrasp, strlen(msgrasp), 0) <= 0) {perror("[client] eroare la write() spre server\n"); exit(1);}

        if(strncmp(msgrasp,"n",1) == 0) close(socketfd), exit(0);
        else if(strncmp(msgrasp,"y",1) == 0) login(socketfd);
    }
}

void comenzi_posibile(int socketfd)
{
    char msg[BUFFER_SIZE];

    // se primeste "comenzi posibile + introduceti comanda" -- (comenzi posibile 1)
    bzero(msg, BUFFER_SIZE);
    if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
    printf("%s", msg);
}

void create_dir(int socketfd)
{
    char msg[BUFFER_SIZE];

    // se primeste "BLACKLIST / introduceti path pt creare director" -- (create dir 1)
    bzero(msg, BUFFER_SIZE);
    if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
    printf("%s", msg);

    if(strncmp(msg,"introduceti path-ul",strlen("introduceti path-ul")) == 0)
    {
        char path[BUFFER_SIZE];

        // se citeste de la tastatura si se trimite path-ul -- (create dir 2)
        bzero(path, BUFFER_SIZE);
        fflush(stdout);
        read(0, path, BUFFER_SIZE);
        if(send(socketfd, path, strlen(path), 0) <= 0) {perror("[client] eroare la write() spre server\n"); exit(1);}

        // se primeste "EROARE / SUCCES + introduceti comanda" -- (create dir 3)
        bzero(msg, BUFFER_SIZE);
        if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
        printf("%s", msg);
    }
}

void create_file(int socketfd)
{
    char msg[BUFFER_SIZE];

    // se primeste "BLACKLIST / introduceti numele pt creare fisier" -- (create file 1)
    bzero(msg, BUFFER_SIZE);
    if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
    printf("%s", msg);

    if(strncmp(msg,"introduceti numele",strlen("introduceti numele")) == 0)
    {
        char nume[BUFFER_SIZE];
        char path[BUFFER_SIZE];

        // se citeste de la tastatura si se trimite numele -- (create file 2)
        bzero(nume, BUFFER_SIZE);
        fflush(stdout);
        read(0, nume, BUFFER_SIZE);
        if(send(socketfd, nume, strlen(nume), 0) <= 0) {perror("[client] eroare la write() spre server\n"); exit(1);}

        // se primeste "introduceti path pt creare file" -- (create file 3)
        bzero(msg, BUFFER_SIZE);
        if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
        printf("%s", msg);

        // se citeste de la tastatura si se trimite path-ul -- (create file 4)
        bzero(path, BUFFER_SIZE);
        fflush(stdout);
        read(0, path, BUFFER_SIZE);
        if(send(socketfd, path, strlen(path), 0) <= 0) {perror("[client] eroare la write() spre server\n"); exit(1);}

        // se primeste "EROARE / SUCCES + introduceti comanda" -- (create file 5)
        bzero(msg, BUFFER_SIZE);
        if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
        printf("%s", msg);
    }
}

void delete_file(int socketfd)
{
    char msg[BUFFER_SIZE];

    // se primeste "BLACKLIST / introduceti path pt delete director/fisier" -- (delete 1)
    bzero(msg, BUFFER_SIZE);
    if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
    printf("%s", msg);

    if(strncmp(msg,"introduceti path-ul",strlen("introduceti path-ul")) == 0)
    {
        char path[BUFFER_SIZE];

        // se citeste de la tastatura si se trimite path-ul -- (delete 2)
        bzero(path, BUFFER_SIZE);
        fflush(stdout);
        read(0, path, BUFFER_SIZE);
        if(send(socketfd, path, strlen(path), 0) <= 0) {perror("[client] eroare la write() spre server\n"); exit(1);}

        // se primeste "EROARE / SUCCES + introduceti comanda" -- (delete 3)
        bzero(msg, BUFFER_SIZE);
        if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
        printf("%s", msg);
    }
}

void delete_dir(int socketfd)
{
    char msg[BUFFER_SIZE];

    // se primeste "BLACKLIST / introduceti path pt delete director" -- (delete 1)
    bzero(msg, BUFFER_SIZE);
    if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
    printf("%s", msg);

    if(strncmp(msg,"introduceti path-ul",strlen("introduceti path-ul")) == 0)
    {
        char path[BUFFER_SIZE];

        // se citeste de la tastatura si se trimite path-ul -- (delete 2)
        bzero(path, BUFFER_SIZE);
        fflush(stdout);
        read(0, path, BUFFER_SIZE);
        if(send(socketfd, path, strlen(path), 0) <= 0) {perror("[client] eroare la write() spre server\n"); exit(1);}

        // se primeste "EROARE / SUCCES + introduceti comanda" -- (delete 3)
        bzero(msg, BUFFER_SIZE);
        if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
        printf("%s", msg);
    }
}

void properties_of(int socketfd)
{
    char msg[BUFFER_SIZE];
    char path[BUFFER_SIZE];

    // se primeste "introduceti path pt director/fisier" -- (prop 1)
    bzero(msg, BUFFER_SIZE);
    if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
    printf("%s", msg);

    // se citeste de la tastatura si se trimite path-ul -- (prop 2)
    bzero(path, BUFFER_SIZE);
    fflush(stdout);
    read(0, path, BUFFER_SIZE);
    if(send(socketfd, path, strlen(path), 0) <= 0) {perror("[client] eroare la write() spre server\n"); exit(1);}

    // se primeste "EROARE / prop + introduceti comanda" -- (prop 3)
    bzero(msg, BUFFER_SIZE);
    if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
    printf("%s", msg);
}

void read_file(int socketfd)
{
    char msg[BUFFER_SIZE];
    char path[BUFFER_SIZE];

    // se primeste "introduceti path fisier" -- (read file 1)
    bzero(msg, BUFFER_SIZE);
    if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
    printf("%s", msg);

    // se citeste de la tastatura si se trimite path-ul -- (read file 2)
    bzero(path, BUFFER_SIZE);
    fflush(stdout);
    read(0, path, BUFFER_SIZE);
    if(send(socketfd, path, strlen(path), 0) <= 0) {perror("[client] eroare la write() spre server\n"); exit(1);}

    // se primeste "EROARE / continut file + introduceti comanda" -- (read file 3)
    bzero(msg, BUFFER_SIZE);
    if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
    printf("%s", msg);
}

void show_dir(int socketfd)
{
    char msg[BUFFER_SIZE];
    char path[BUFFER_SIZE];

    // se primeste "introduceti path pt director" -- (show dir 1)
    bzero(msg, BUFFER_SIZE);
    if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
    printf("%s", msg);

    // se citeste de la tastatura si se trimite path-ul -- (show dir 2)
    bzero(path, BUFFER_SIZE);
    fflush(stdout);
    read(0, path, BUFFER_SIZE);
    if(send(socketfd, path, strlen(path), 0) <= 0) {perror("[client] eroare la write() spre server\n"); exit(1);}

    // se primeste "EROARE / content + introduceti comanda" -- (show dir 3)
    bzero(msg, BUFFER_SIZE);
    if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
    printf("%s", msg);
}

void move_file(int socketfd)
{
    char msg[BUFFER_SIZE];
    char nume[BUFFER_SIZE];
    char path[BUFFER_SIZE];

    // se primeste "BLACKLIST / introduceti nume pt director/fisier" -- (move file 1)
    bzero(msg, BUFFER_SIZE);
    if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
    printf("%s", msg);

    if(strncmp(msg,"introduceti numele", strlen("introduceti numele")) == 0)
    {
        // se citeste de la tastatura si se trimite numele -- (move file 2)
        bzero(nume, BUFFER_SIZE);
        fflush(stdout);
        read(0, nume, BUFFER_SIZE);
        if(send(socketfd, nume, strlen(nume), 0) <= 0) {perror("[client] eroare la write() spre server\n"); exit(1);}

        // se primeste "introduceti path pt mutare fisier" -- (create file 3)
        bzero(msg, BUFFER_SIZE);
        if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
        printf("%s", msg);

        // se citeste de la tastatura si se trimite path-ul -- (move file 4)
        bzero(path, BUFFER_SIZE);
        fflush(stdout);
        read(0, path, BUFFER_SIZE);
        if(send(socketfd, path, strlen(path), 0) <= 0) {perror("[client] eroare la write() spre server\n"); exit(1);}

        // se primeste "EROARE / SUCCES + introduceti comanda" -- (move file 5)
        bzero(msg, BUFFER_SIZE);
        if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
        printf("%s", msg);
    }
}

int main()
{
    int socketfd;                   
    struct sockaddr_in server; 
    char msg[BUFFER_SIZE];           // mesajul primit de la server
    char msgrasp[BUFFER_SIZE] = " "; // mesaj de raspuns pentru server

    // crearea unui socket
    if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {perror("[server] eroare la creare socket\n"); exit(1);} 

    server.sin_family = AF_INET; // familia socketului 
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // asignarea adresa IP
    server.sin_port = htons(PORT); // asigneare port

    // conecatre client la server
    if(connect(socketfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {perror("[client] eroare la connect()\n");exit(1);}


    //            INCEPE COMUNICAREA CU SERVERUL           //
    
    // se primeste "cont existent? [y/n]" -- (1)
    bzero(msg, BUFFER_SIZE);
    if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
    printf("%s", msg);

    // se citeste de la tastatura si se trimite "y/n" -- (2)
    bzero(msgrasp, BUFFER_SIZE);
    fflush(stdout);
    read(0, msgrasp, BUFFER_SIZE);
    if(send(socketfd, msgrasp, strlen(msgrasp), 0) <= 0) {perror("[client] eroare la write() spre server\n"); exit(1);}

    //            REGISTER client nou            //
    if(strncmp(msgrasp,"n",1) == 0)
    {
        register_client_nou(socketfd);
    }

    //            LOGARE            //
    login(socketfd);

    //            EXECUTARE COMENZI            //
    char comanda[BUFFER_SIZE];

    while(1)
    {
        // se citeste de la tastatura si se trimite o comanda -- (15)
        bzero(comanda, BUFFER_SIZE);
        fflush(stdout);
        read(0, comanda, BUFFER_SIZE);
        if(send(socketfd, comanda, strlen(comanda), 0) <= 0) {perror("[client] eroare la write() spre server\n"); exit(1);}

        if(strncmp(comanda, "proprietati", strlen("proprietati")) == 0) properties_of(socketfd); 
        else if(strncmp(comanda, "show dir", strlen("show dir")) == 0) show_dir(socketfd); 
        else if(strncmp(comanda, "delete file", strlen("delete file")) == 0) delete_file(socketfd);
        else if(strncmp(comanda, "delete dir", strlen("delete dir")) == 0) delete_dir(socketfd);
        else if(strncmp(comanda, "create file", strlen("create file")) == 0) create_file(socketfd);
        else if(strncmp(comanda, "move file", strlen("move file")) == 0) move_file(socketfd);
        else if(strncmp(comanda, "read file", strlen("read file")) == 0) read_file(socketfd);  
        else if(strncmp(comanda, "create dir", strlen("create dir")) == 0) create_dir(socketfd);
        else if(strncmp(comanda, "comenzi posibile", strlen("comenzi posibile")) == 0) comenzi_posibile(socketfd); 
        else if(strncmp(comanda, "logout", strlen("logout")) == 0)
        {
            // se inchide socketul dupa ce se deconecteaza clientul
            close(socketfd); 
            break;
        }
        else // comanda invalida
        {
            // se primeste "COMANDA INVALIDA" -- ()
            bzero(msg, BUFFER_SIZE);
            if(recv(socketfd, msg, BUFFER_SIZE, 0) < 0) {perror("[client] eroare la read() de la server\n"); exit(1);}
            printf("%s", msg);
        }

    }

    close(socketfd); // inchidere conexiune
}