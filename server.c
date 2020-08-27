/* This is the server file of the File Transfer Application*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

void serviceClient(int, int);

int main(int argc, char *argv[])
{
    //connection variables
    int sd, client, portNumber;
    struct sockaddr_in servAdd;

    //if there are not enough arguments
    if (argc != 2)
    {
        printf("Call model: %s <Port Number>\n", argv[0]);
        exit(0);
    }
    // creating socket descriptor
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "Cannot create socket\n");
        exit(1);
    }
    // creating socket address
    servAdd.sin_family = AF_INET;
    servAdd.sin_addr.s_addr = htonl(INADDR_ANY);

    //on port number
    sscanf(argv[1], "%d", &portNumber);
    servAdd.sin_port = htons((uint16_t)portNumber);

    //binding socket to socket addr
    bind(sd, (struct sockaddr *)&servAdd, sizeof(servAdd));

    // listening on that socket with max backlog of 5 connections
    listen(sd, 5);

    printf("---------------------Server Started--------------------\n");

    while (1)
    {
        // block process until a client appears on this socket
        client = accept(sd, NULL, NULL);

        //call the serviceClient to process the request
        if (!fork())
        {
            printf("-------Connection established with a client %d-------\n", getpid());
            serviceClient(client, getpid());
        }

        close(client);
    }
}
//handles the processing of the request
void serviceClient(int sd, int id)
{
    //variable declaration
    char command[255], fileName[100], fileExists[10], serverText[255];
    char r, s;
    char *message, *isFound;
    int fd, end;

    while (1)
    {
        isFound = "true";
        //Sending message to the client
        message = "Please select your option.\n 1. Enter 'get <fileName>' to receive a file from server.\n 2. Enter 'put <fileName>' to upload a file on server. \n 3. Enter 'Quit' to exit.";
        write(sd, message, strlen(message) + 1);

        //Reading the command and fileName from the client
        read(sd, command, 255);
        read(sd, fileName, 100);

        //if command is get then sends the file to client
        if (!strcasecmp(command, "get"))
        {
            //if file doesnot exists then notifying the client about it.
            if ((fd = open(fileName, O_RDONLY)) == -1)
            {
                isFound = "false";
                write(sd, isFound, strlen(isFound) + 1);
                fprintf(stderr, ">Client %d: File '%s' does not exists!!\n", id, fileName);
            }
            //if the file exists then sending the contents to client
            else
            {

                write(sd, isFound, strlen(isFound) + 1);
                while ((end = read(fd, &s, 1)) == 1)
                {
                    // fprintf(stderr, "%c", s);
                    write(sd, &s, 1);
                }
                if (end == 0)
                {
                    s = 4;
                    //fprintf(stderr, "%d", end);
                    write(sd, &s, 1);
                }
                close(fd);
                fprintf(stderr, ">Client %d: File Contents of '%s' sent to the client!!\n", id, fileName);
            }
        }
        //if the command is put then receives the file from client
        if (!strcasecmp(command, "put"))
        {
            sleep(1);
            //check whether the file exists or not
            read(sd, fileExists, 10);
            if (!strcasecmp(fileExists, "true"))
            {
                if ((fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0700)) == 0)
                {
                    fprintf(stderr, ">Client %d:Could not open the file\n", id);
                }
                else
                {
                    while (read(sd, &r, 1) == 1 && r != 4)
                    {
                        // fprintf(stderr, "%c", r);
                        write(fd, &r, 1);
                    }
                    fprintf(stderr, ">Client %d: File Contents of '%s' Received\n", id, fileName);
                    close(fd);
                }
            }
        }
        //if the command is quit then closes the client
        if (!strcasecmp(command, "Quit"))
        {
            fprintf(stderr, "--------------Bye, the client %d has left------------\n", id);
            close(sd);
            exit(0);
        }
    }
}