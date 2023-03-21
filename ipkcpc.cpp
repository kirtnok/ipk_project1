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
//max number size of buffer
#define BUFSIZE 1024
//global flag for hadling SIGINT
bool sigint = false;

void print_help(){
    std::cout << "Usage: ipkcpc -h <host> -p <port> -m <mode> [--help]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "-h <host>   Hostname or IP address of the server" << std::endl;
    std::cout << "-p <port>   Port number of the server" << std::endl;
    std::cout << "-m <mode>   Mode, either tcp or udp" << std::endl;
    std::cout << "--help          Print this help" << std::endl;
}
//fucntion that is called after SIGINT
//sets global flag to true
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
    // if --help is the only argument
    if(argc == 2 && !strcmp(argv[1], "--help")){
        print_help();
        exit(0);
    }
    if (argc != 7){
        std::cerr << "Wrong number of arguments" << std::endl;
        std::cout << "Use --help for help" << std::endl;
        exit(1);
    }
    // parsing arguments
    while ((arg = getopt(argc, argv, "h:p:m:"))!= -1){
        switch (arg){
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = strtol(optarg,&p,10);
                //checking if port is in range and if it is a number
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
    //setting up SIGINT handler
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
            //checking if input is bigger than size of buffer
            if (strlen(buf) == BUFSIZE-1 && buf[BUFSIZE-2] != '\n'){
                // buffer is 1024 bytes long and last 2 characters are reserved for \n and \0
                std::cerr << "Error: input exceeds limit, max number of characters is " << BUFSIZE-2 << std::endl;
                bzero(buf, BUFSIZE);
                strcpy(buf,"BYE\n");
                //setting flag to true so that client exits with error after BYE is sent
                exiting = true;
            }
            //checking if EOF or SIGINT was received
            if(feof(stdin) || sigint){
                bzero(buf, BUFSIZE);
                strcpy(buf,"BYE\n");
            }
            bytestx = send(client_socket, buf, strlen(buf), 0);
            //checking if send was successful
            if (bytestx < 0){
                perror("ERROR in sendto");
                close(client_socket);
                exit(1);
            }
            bzero(buf, BUFSIZE);
            bytesrx = recv(client_socket, buf, BUFSIZE, 0);
            //checking if recv was successful
            if (bytesrx < 0){
                perror("ERROR in recvfrom");
                close(client_socket);
                exit(1);
            }
            std::cout << buf;
            //if BYE was received, connection is closing
            if(!strcmp(buf,"BYE\n")){
                break;
            }
        }
        close(client_socket);
        //if error occured but BYE was sent, exit with error
        if (exiting){
            exit(1);
        }
    }
    else if (udp_mode){
        serverlen = sizeof(server_address);
        if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) <= 0){
            perror("ERROR: socket");
            exit(1);
        }
        //setting timeout for udp connection
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
            std::cerr << "Unable to set UDP socket timeout" << std::endl;
            exit(1);
        }

        while(1){
            bzero(buf, BUFSIZE);
            // first 2 bytes are reserved, used pointer arithmetic
            fgets(buf + 2, BUFSIZE-2, stdin);
            //checking if input is bigger than size of buffer calculating with reserved bytes
            if (strlen(buf + 2) == BUFSIZE-3 && buf[BUFSIZE-2] != '\n'){
                // buffer is 1024 bytes long and last 4 characters are reserved for \n, \0 and 2 bytes because of protocol
                std::cerr << "Error: input exceeds limit, max number of characters is " << BUFSIZE-4 << std::endl;
                close(client_socket);
                exit(1);
            }
            // removing newline character from input
            buf[strcspn(buf + 2, "\n") + 2] = 0;
            // checking if payload is not bigger than 255 characters
            if(strlen(buf + 2) > 255){
                std::cerr << "Warning: payload is too long, max size of message is 255 characters" << std::endl;
                continue;
            }
            if(feof(stdin) || sigint){
                //its not needed to close connection in udp mode but closing file descriptor
                close(client_socket);
                exit(0);
            }
            // setting first byte to 0
            buf[0] = '\0';
            // setting second byte of sent message to length of payload
            buf[1] = (char)strlen(buf + 2);
            bytestx = sendto(client_socket, buf, strlen(buf + 2) + 2, 0, (struct sockaddr *) &server_address, serverlen);
            if (bytestx < 0){
                perror("ERROR: sendto");
                //its not needed to close connection in udp mode but closing file descriptor
                close(client_socket);
                exit(1);
            }
            bzero(buf, BUFSIZE);
            bytesrx = recvfrom(client_socket, buf, BUFSIZE, 0, (struct sockaddr *) &server_address, &serverlen);
            if (bytesrx < 0){
                perror("ERROR: recvfrom");
                //its not needed to close connection in udp mode but closing file descriptor
                close(client_socket);
                exit(1);
            }
            // checking if second byte of recieved message is 0 if so, message is OK, if not, message is ERR
            if ((int)buf[1] == 0){
                std::cout << "OK:" << buf+3 << std::endl;
            }
            else{
                std::cout << "ERR:" << buf+3 << std::endl;
            }
        }
        //its not needed to close connection in udp mode but closing file descriptor
        close(client_socket);
    }
    return 0;
}
