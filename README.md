# ipk_projekt1

Implementation of the TCP and UDP client for the IPK Calculator
in the C++. Program is limited only for linux operating systems because of some used libraries aviable only for linux.

## Usage
ipkcpc -h \<host\> -p \<port> -m \<mode> [--help]

-h \<host>   Hostname or IP address of the server  
-p \<port>   Port number of the server  
-m \<mode>   Mode, either tcp or udp  


## Code structure of the ipkcpc.cpp

### Parsing of the arguments
Arguments are parsed in `main()` after variable initializations, `getopt()` function is used in loop where are arguments also checked if they are in valid format.
### Body 
After argument parsing is condition which handles whole logic of the program. Dividing program into two main sections. 
One for the TCP communication and one for the UDP communication.
### INPUT
Input is loaded with `fgets()`. Any input bigger than `BUFSIZE-2` for TCP and `BUFSIZE-4` for UDP, raises error and terminates program (default value of BUFSIZE is 1024 chars). Also for UDP message limit is set for 255 chars . If it is more user is informed with warning and message is not sent.
### TCP communication 
Handling of the TCP communication starts with the creation of the socket with the function `socket()`, and attempting to `connect()`.
After successful connection, program starts reading messages from the standard input in loop and tries to send it to the server. After sending buffer is cleared with `bzero()`. And program then awaits response.
If everything is ok, received message is stored into buffer and displayed on the standard output. If server message was "BYE", loop breaks and socket is closed. `close()` also disconnects TCP connection and closes file descriptor.
### UDP communication 
UDP does not need to be connected to the server. Buffer is divided and managed as stated in protocol.
Maximum size of payload is 255 chars.
When user exceeds this limit, warning is raised and message is not sent.
Response is saved into buffer and its status code is checked. If it is zero, the message is displayed with standard output with prefix "OK:", if one, with prefix "ERR:". After recieving eof or C-c UDP loop breaks and socket is closed, although it is UDP because socket is still file descriptor. For UDP is also implemented timeout 10 seconds if sent or recieved message is lost.
### Handling SIGINT
After signal SIGINT flag is raised and both protocols will be closed gracefully. In TCP "BYE" will be sent, In UDP is closed socket and program ends.
## Testing
Testing has been done during the whole process of the implementation.  
At first the basic inputs from the assignment were tested in the command line.
Testing has been done manualy mostly with edgecases.
For example: input bigger than payload or buffer size, not existing server, SIGINT at random state, eof in input, wrong arguments... 

## Bibliography
[C++ Documentation](https://learn.microsoft.com/en-us/cpp/cpp/?view=msvc-170)
[STUBS](https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Stubs)

