/****************************************************************************************
Program Name: ftserver.c
Author Name: Wisam Thalij
Date Created: 11-14-2015
Last Day Modified: 11-19-2015
Description: This program works as a Server. The progran will require the user to enter
      the server port number from the command line to wait on, then the program run
      and wait for a connection.
      once a connection made the server and the client will start a TCP Control Connection
      to start sending and receiving message with the client.
      if client sent an invalid command the server send an error to the client to display.
      otherwise, The server will initiate a TCP Data connection with the client on the
      data port.
      if client sent -l command , server will send it's directory list to the client and
      the client will display it.
      if the client set -g commad, server will validate the filemane and either send the
      file content to the client as a message and the client will save it to it's dirctory
      as a file. or, the server will send a message to the client "file not found"
Extra:
  The server is a multi-thread server, it will accept up to 5 connection at the
  same time.
*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>

// This function is called when a system call fails
void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main (int argc, char *argv[]) {
    // Create the socket
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    pid_t pid;
    char message[500];
    char Buffer[50];
    int n;
    int nread;
    char dataPort[50];
    char hostName[50];
    char command[50];
    char fileName[50];
    char fileNameLength[3];
    size_t size;
    char ClientHost[1024];
    char ClientService[20];
    char Data[5000];
    char Comm[5000];
    char CommBuffer[5000];
    int FileContentSize;
    char *textBuffer;
    char TextFileContentSize[10];
    struct stat st;
    char ConfirmMsg[3];
    size_t result;

    // Check for Usage validation
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    // Initialize the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // Bind the socket to the port
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    // Listen to the clients up to 5 at a time
    listen(sockfd,5);
    clilen = sizeof(cli_addr);

    printf("Server open on %d\n", portno );

    // Go to infinite while loop and wait for client requests
    while(1) {
        // Accept a connection and make new socket
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0)
            error("ERROR on accept");

        // Fork a new process to process this client request and go back to be availabe again
        pid = fork();

        if(pid < 0) {
            error("ERROR on fork");
        }
        // free the variable for the next connection
        bzero(ClientHost,1024);
        // Get the client host name and servoice
        getnameinfo((struct sockaddr *) &cli_addr, sizeof cli_addr, ClientHost, sizeof ClientHost, ClientService, sizeof ClientService, 0);

        if(pid == 0) {
            printf("\nConnection from %s\n", ClientHost);
            // When in child process close the original socke
            close(sockfd);
            n = write(newsockfd,"TCP Contorl",11);

            // Get data from client
            // wipe out the content of dataPort
            bzero(dataPort,50);
            n = read(newsockfd,dataPort,6);
            n = write(newsockfd,"ok",3);
            // wipe out the content of hostName
            bzero(hostName,50);
            n = read(newsockfd,hostName,10);
            n = write(newsockfd,"ok",3);
            // wipe out the content of dataPort
            bzero(command,50);
            n = read(newsockfd,command,5);
            if (strcmp(command,"-g") == 0) {
                // wipe out the content of fileName
                bzero(fileName,50);
                n = write(newsockfd,"ok",3);
                bzero(fileNameLength,3);
                n = read(newsockfd,fileNameLength,2);
                n = write(newsockfd,"ok",3);
                size = atoi(fileNameLength);
                n = read(newsockfd,fileName,size + 1);
            }

            // Initialize a data TCP connection
            int dataSockfd, portno1, m;
            struct sockaddr_in serv_addr1;
            struct hostent *server1;
            portno1 = atoi(dataPort);
            // Creatig the socket
            dataSockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (dataSockfd < 0)
                error("ERROR opening socket");

            // Get the address of the host
            server1= gethostbyname(ClientHost);

            if (server1 == NULL) {
                fprintf(stderr,"ERROR, no such host\n");
                exit(0);
            }

            // Initialize the serv_adder
            bzero((char *) &serv_addr1, sizeof(serv_addr1));
            serv_addr1.sin_family = AF_INET;
            bcopy((char *)server1->h_addr, (char *)&serv_addr1.sin_addr.s_addr, server1->h_length);
            serv_addr1.sin_port = htons(portno1);

            // test if datasocket ready on the client side
            n = read(newsockfd, Buffer, 50);
            size_t ln = strlen(Buffer) - 1;
            if (Buffer[ln] == '\n')
                Buffer[ln] = '\0';
            if (strcmp(Buffer,"READY") == 0) {
                // If client sent a valid command
                if (strcmp(command,"-g") == 0 || strcmp(command,"-l") == 0) {
                    // Initiate a Data Connection
                    if (connect(dataSockfd,(struct sockaddr *) &serv_addr1,sizeof(serv_addr1)) < 0)
                          perror("ERROR connecting %d");

                    if (strcmp(command,"-l") == 0) {
                        printf("\nList directory requested on port %d.\n", portno1);
                        // look up the files listed in the directory
                        FILE *pf;
                        bzero(Data,5000);
                        sprintf(Comm, "ls");

                        pf = popen(Comm, "r");
                        while (fgets(CommBuffer, 5000, pf)) {
                            strcat(Data, CommBuffer);
                        }

                        if (pclose(pf) !=0 )
                            fprintf(stderr, "Error: Failed to close command stream \n");

                        printf("\nSending directory content to %s:%d\n", ClientHost ,portno1);
                        // Send the list to the client
                        n = write(dataSockfd,Data,5000);
                    }

                    if (strcmp(command,"-g") == 0) {
                        printf("\nFile \"%s\" requested on port %d.\n",fileName ,portno1);
                        // Check if the file exist in the server repository
                        if( access( fileName, F_OK ) != -1 ) {
                            n = write(dataSockfd,"start",6);
                            // If the file exits in the repository
                            // Open the file and get it's size
                            FILE *plainFile;

                            plainFile = fopen(fileName, "r");

                            if (plainFile == NULL) {
                                perror("Can not open that file! try again!");
                            }

                            // Get the size of the file and go back to the start position
                            stat(fileName, &st);
                            FileContentSize = st.st_size;

                            // Alocate memory for the textBuffer to store the text
                            textBuffer = malloc(FileContentSize);

                            // copy the file into the buffer
                            result = fread (textBuffer, 1 , FileContentSize, plainFile);
                            if (result != FileContentSize) {
                                fputs ("Reading error 2",stderr);
                                exit (3);
                            }
                            // Close the Plain text file
                            fclose(plainFile);

                            // send file size to client
                            bzero(TextFileContentSize,10);
                            snprintf(TextFileContentSize, 10,  "%d", FileContentSize);
                            n = write(dataSockfd,TextFileContentSize,strlen(TextFileContentSize));
                            bzero(ConfirmMsg,3);
                            n = read(dataSockfd,ConfirmMsg,3);
                            if (n < 0)
                                error("ERROR reading from socket");

                            printf("\nSending \"%s\" to %s:%d\n",fileName , ClientHost, portno1);

                            // Send the file contect to the client
                            n = write(dataSockfd,textBuffer,FileContentSize);

                            // Free the saved text buffer
                            free(textBuffer);
                        } else {
                            // If file does not exist in the repository
                            printf("\nFile not found. Sending error message to %s:%d\n", ClientHost, portno1);
                            n = write(dataSockfd,"FILE FILE NOT FOUND!.",22);
                        }
                    }
                    close(dataSockfd);
                } else {
                    // Client sent an invalid message
                    printf("Client sent invalid Comand\n");
                }
            }
            exit(0);
        } else {
            close(newsockfd);
        }
    }
    close(sockfd);
    return 0;
}
