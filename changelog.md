# Changelog

## Limitations
Because I am using static buffer, client input message is limited by default for 1022 chars for TCP.  
For UDP it is only 1020 chars because of protocol.
Another limitation is when program is in TCP mode and send or recieve  
fails program catches error and terminates, that shutdown host server.(atleast given test serever)

## Changes

### Commit: Add changelog  
Date:   Mar 21 2023
    
    -added changelog, also with history of commits

### Commit: Remove newline from output after SIGINT signal  
Date:   Mar 21 2023
    
    -removed newline after C-c as stated in forum
    
### Commit: Add timeout for UDP connection  
Date:   Mar 21 2023
    
    -added socket modification with setsockopt, now program timeouts after 10 seconds of not recieving response

### Commit: Add commets  
Date:   Mar 20 2023

    - added comments

### Commit: Refactor  
Date:   Mar 19 2023
    
    - changed newlines to cpp standard
    - moved same type initialization to one line
    - removed debug prints

    
### Commit: Add --help option, edit error messages  
Date:   Mar 19 2023

    - added print_help function
    - added --help argument option that prints help and exits
    - added information for user when he use program wrongly
    - changed erros so they are printed to stderr from stdout
    - changed send and recieve error checks, now they terminate program

### Commit: Fix size of sent UDP message  
Date:   Mar 19 2023

    - fixed overflowing size of payload message

### Commit: Add buffer overflow protection, enforcing maximum UDP payload length  
Date:   Mar 19 2023

    - added check after fgets so program work with input bigger than buffer size
    - added user limit for UDP payload message
    - added flag and check if program had error state but not terminating one

### Commit: Add SIGINT hadling  
Date:   Mar 19 2023

    - added fuction and flag for SIGINT response
    - changed check for EOF so now it controls sigint flag in both protocols

### Commit: Add UDP implementation  
Date:   Mar 18 2023

    - added basic structure for tcp iplementation

### Commit: Add TCP implementation  
Date:   Mar 18 2023

    - added basic structure for tcp iplementation

### Commit: Add command line arguments loading  
Date:   Mar 17 2023

    - added simple parsing of arguments  
    - added debug prints and error messages for argument parsing  

