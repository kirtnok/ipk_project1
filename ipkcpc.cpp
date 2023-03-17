/*auhor:Jakub Kontrik*/
#include <iostream>
#include <unistd.h>
#include <string.h>

int  main(int argc, char *argv[]){
    int arg;
    std::string host;
    char *p;
    int port;
    bool tcp_mode;
    bool udp_mode;
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
    //debug print
    std::cout << port << " " << host << "\n";
    return 0;
}
