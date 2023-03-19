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

void print_help(){
    std::cout << "Usage: ipkcpc -h <host> -p <port> -m <mode> [--help]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "-h <host>   Hostname or IP address of the server" << std::endl;
    std::cout << "-p <port>   Port number of the server" << std::endl;
    std::cout << "-m <mode>   Mode, either tcp or udp" << std::endl;
    std::cout << "--help          Print this help" << std::endl;
}

void sigint_handle(int signum){
    (void)signum;
    sigint = true;
    std::cout << std::endl;
}

int  main(int argc, char *argv[]){
    int arg, port, client_socket, bytestx, bytesrx;
    const char *host;
    char *p;
    char buf[BUFSIZE];
    bool tcp_mode = false;
    bool udp_mode = false;
    bool exiting = false;
    struct sockaddr_in server_address;
    struct hostent *server;
    socklen_t serverlen;
    struct sigaction sigint_handler;
    
    if(argc == 2 && !strcmp(argv[1], "--help")){
        print_help();
        exit(0);
    }
    if (argc != 7){
        std::cerr << "Wrong number of arguments" << std::endl;
        std::cout << "Use --help for help" << std::endl;
        exit(1);
    }
    while ((arg = getopt(argc, argv, "h:p:m:"))!= -1){
        switch (arg){
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = strtol(optarg,&p,10);
                if (*p != '\0' || port < 0 || port > 65535){
                    std::cerr << "Error: wrong port format" << std::endl;
                    exit(1);
                }
                break;
            case 'm':
                if (!strcmp(optarg,"udp")){
                    udp_mode = true;
                }
                else if(!strcmp(optarg,"tcp")){
                    tcp_mode = true;
                }
                else{
                    std::cerr << "Error: wrong mode" << std::endl;
                    std::cout << "Use --help for help" << std::endl;
                    exit(1);
                }
                break;
            case '?':
                std::cerr << "Error: unknown option: " << (char)optopt << std::endl;
                std::cout << "Use --help for help" << std::endl;
                exit(1);
        }
    }

    if ((server = gethostbyname(host)) == NULL){
        std::cerr << "ERROR: no such host as " << host << std::endl;
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
        if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0){
            perror("ERROR: socket");
            exit(1);
        }
        if (connect(client_socket, (const struct sockaddr *) &server_address, sizeof(server_address)) != 0){
            perror("ERROR: connect");
            exit(1);        
        }
        while (1){
            bzero(buf, BUFSIZE);
            fgets(buf, BUFSIZE, stdin);
            if (strlen(buf) == BUFSIZE-1 && buf[BUFSIZE-2] != '\n'){
                std::cerr << "Error: input exceeds limit, max number of characters is " << BUFSIZE-2 << std::endl;
                bzero(buf, BUFSIZE);
                strcpy(buf,"BYE\n");
                exiting = true;
            }
            if(feof(stdin) || sigint){
                bzero(buf, BUFSIZE);
                strcpy(buf,"BYE\n");
            }
            bytestx = send(client_socket, buf, strlen(buf), 0);
            if (bytestx < 0){
                perror("ERROR in sendto");
                close(client_socket);
                exit(1);
            }
            bzero(buf, BUFSIZE);
            bytesrx = recv(client_socket, buf, BUFSIZE, 0);
            if (bytesrx < 0){
                perror("ERROR in recvfrom");
                close(client_socket);
                exit(1);
            }
            std::cout << buf;
            if(!strcmp(buf,"BYE\n")){
                break;
            }
        }
        close(client_socket);
        if (exiting){
            exit(1);
        }
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
            fgets(buf + 2, BUFSIZE-2, stdin);
            if (strlen(buf + 2) == BUFSIZE-3 && buf[BUFSIZE-2] != '\n'){
                std::cerr << "Error: input exceeds limit, max number of characters is " << BUFSIZE-4 << std::endl;
                close(client_socket);
                exit(1);
            }
            buf[strcspn(buf+2, "\n")+2] = 0;
            if(strlen(buf + 2) > 255){
                std::cerr << "Warning: payload is too long, max size of message is 255 characters" << std::endl;
                continue;
            }
            if(feof(stdin) || sigint){
                close(client_socket);
                exit(0);
            }
            buf[0] = '\0';
            buf[1] = (char)strlen(buf + 2);
            bytestx = sendto(client_socket, buf, strlen(buf + 2) + 2, 0, (struct sockaddr *) &server_address, serverlen);
            if (bytestx < 0){
                perror("ERROR: sendto");
                close(client_socket);
                exit(1);
            }
            bzero(buf, BUFSIZE);
            bytesrx = recvfrom(client_socket, buf, BUFSIZE, 0, (struct sockaddr *) &server_address, &serverlen);
            if (bytesrx < 0){
                perror("ERROR: recvfrom");
                close(client_socket);
                exit(1);
            }
            if ((int)buf[1] == 0){
                std::cout << "OK:" << buf+3 << std::endl;
            }
            else{
                std::cout << "ERR:" << buf+3 << std::endl;
            }
        }
        close(client_socket);
    }
    return 0;
}
