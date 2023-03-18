/*auhor:Jakub Kontrik*/
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <csignal>
#define BUFSIZE 1024
bool sigint = false;

void sigint_handle(int signum){
    (void)signum;
    sigint = true;
    std::cout << "\n";
}

int  main(int argc, char *argv[]){
    int arg;
    const char *host;
    char *p;
    int port;
    int client_socket;
    char buf[BUFSIZE];
    int bytestx;
    int bytesrx;
    bool tcp_mode = false;
    bool udp_mode = false;
    struct sockaddr_in server_address;
    struct hostent *server;
    socklen_t serverlen;
    struct sigaction sigint_handler;
    if (argc != 7){
        std::cout << "Wrong number of arguments" << "\n";
        exit(1);
    }
    while ((arg = getopt(argc, argv, "h:p:m:"))!= -1){
        switch (arg){
            case 'h':
                //debug print
                std::cout << "Host: " << optarg << "\n";
                host = optarg;
                break;
            case 'p':
                std::cout << "Port: " << optarg << "\n";
                port = strtol(optarg,&p,10);
                if (*p != '\0'){
                    std::cout << "Error wrong port format\n";
                    exit(1);
                }
                break;
            case 'm':
                std::cout << "Mode: " << optarg << "\n";
                if (!strcmp(optarg,"udp")){
                    udp_mode = true;
                }
                else if(!strcmp(optarg,"tcp")){
                    tcp_mode = true;
                }
                else{
                    std::cout << "Error wrong mode\n";
                    exit(1);
                }
                break;
            case '?':
                std::cout << "Unknown option: " << (char)optopt << "\n";
                exit(1);
        }
    }
    if ((server = gethostbyname(host)) == NULL) {
        std::cerr << "ERROR: no such host as " << host << "\n";
        exit(1);
    }
    sigint_handler.sa_handler = sigint_handle;
    sigemptyset(&sigint_handler.sa_mask);
    sigint_handler.sa_flags = 0;
    sigaction(SIGINT, &sigint_handler, NULL);
    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
    server_address.sin_port = htons(port);
    if(tcp_mode){
        if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
        {
            perror("ERROR: socket");
            exit(1);
        }
        if (connect(client_socket, (const struct sockaddr *) &server_address, sizeof(server_address)) != 0)
        {
            perror("ERROR: connect");
            exit(1);        
        }
        while (1){
            bzero(buf, BUFSIZE);
            fgets(buf, BUFSIZE, stdin);
            if(feof(stdin) || sigint){
                bzero(buf, BUFSIZE);
                strcpy(buf,"BYE\n");
            }
            bytestx = send(client_socket, buf, strlen(buf), 0);
            if (bytestx < 0) perror("ERROR in sendto");
            bzero(buf, BUFSIZE);
            bytesrx = recv(client_socket, buf, BUFSIZE, 0);
            if (bytesrx < 0) perror("ERROR in recvfrom");
            std::cout << buf;
            if(!strcmp(buf,"BYE\n")){
                break;
            }
        }
        close(client_socket);
    }
    else if (udp_mode){
        serverlen = sizeof(server_address);
        if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) <= 0)
        {
            perror("ERROR: socket");
            exit(1);
        }
        while(1){
            bzero(buf, BUFSIZE);
            fgets(buf + 2, BUFSIZE - 2, stdin);
            if(feof(stdin) || sigint){
                close(client_socket);
                exit(0);
            }
            buf[0] = '\0';
            buf[1] = (char)strlen(buf + 2);
            std::cout << "Sending: " << (int)buf[1] << "\n";
            bytestx = sendto(client_socket, buf, (int)buf[1] + 2, 0, (struct sockaddr *) &server_address, serverlen);
            if (bytestx < 0) perror("ERROR: sendto");
            bzero(buf, BUFSIZE);
            bytesrx = recvfrom(client_socket, buf, BUFSIZE, 0, (struct sockaddr *) &server_address, &serverlen);
            if (bytesrx < 0) perror("ERROR: recvfrom");
            if ((int)buf[1] == 0){
                std::cout << "OK:" << buf+3 << "\n";
            }
            else{
                std::cout << "ERR:" << buf+3 << "\n";
            }
        }
        close(client_socket);
    }
    return 0;
}
