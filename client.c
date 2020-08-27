/* This is the client file of the File Transfer Application*/

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    //variable declaration
    char message[255], isFound[10], tempMessage[255];
    char r, s;
    char *getMessage, *command, *fileName;
    int server, portNumber, pid, n, fd, end;
    struct sockaddr_in servAdd; // server socket address
    char *fileExists;
    getMessage = "Please enter the file name you want to receive from server:\n";

    //if the number of arguments is not equal to 3
    if (argc != 3)
    {
        printf("Call model: %s <IP Address> <Port Number>\n", argv[0]);
        exit(0);
    }

    // creating socket descriptor
    if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "Cannot create socket\n");
        exit(1);
    }
    // creating socket address
    servAdd.sin_family = AF_INET;

    //on port number
    sscanf(argv[2], "%d", &portNumber);
    servAdd.sin_port = htons((uint16_t)portNumber);

    // converting addr from presentation to network format
    if (inet_pton(AF_INET, argv[1], &servAdd.sin_addr) < 0)
    {
        fprintf(stderr, " inet_pton() has failed\n");
        exit(2);
    }
    // connect to the server socket
    if (connect(server, (struct sockaddr *)&servAdd, sizeof(servAdd)) < 0)
    {
        fprintf(stderr, "connect() has failed, exiting\n");
        exit(3);
    }
    fprintf(stderr, "------------------------Welcome------------------------\n");

    while (1)
    {
        fileExists = "true";
        //reads the message from the server
        if (read(server, message, 255) < 0)
        {
            fprintf(stderr, "Unable to read from server");
        }

        fprintf(stderr, "%s\n", message);
        //Reading the tempMessage from the user
        read(0, tempMessage, 255);
        //fprintf(stderr, "Command:: %s\n", tempMessage);
        //Dividing the tempMessage into command and fileName
        int i = 0;
        char *tokens = strtok(tempMessage, " \n");
        while (tokens != NULL && i < 2)
        {
            if (i == 0)
                command = tokens;
            else
                fileName = tokens;

            i++;
            tokens = strtok(NULL, " \n");
        }
        //sending the command and fileName to the server
        write(server, command, strlen(command) + 1);
        write(server, fileName, strlen(fileName) + 1);
        sleep(1);
        //if the command is get then downloads the file from server
        if (!strcasecmp(command, "get"))
        {
            fprintf(stderr, "Verifying whether the file exists or not.\n");
            //reads from the server if the file exists or not
            read(server, isFound, 10);

            //if the file exists  is true then receives the file contents from the server
            if (!strcasecmp(isFound, "true"))
            {
                if ((fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0700)) == 0)
                {
                    fprintf(stderr, "Unable to read from server");
                }
                else
                {
                    while (read(server, &r, 1) == 1 && r != 4)
                    {
                        // fprintf(stderr, "%c", r);
                        write(fd, &r, 1);
                    }
                    fprintf(stderr, "\nFile found and downloaded successfully!!\n");
                    close(fd);
                }
            }
            //if file does not exists then notifies the client that file does not exist.
            else
            {
                fprintf(stderr, "File '%s' doesnot exist!!\n", fileName);
            }
        }
        //if the command is put then uploads a file to server
        else if (!strcasecmp(command, "put"))
        {
            sleep(1);
            //if the file does not exists then notifies the client about the same
            if ((fd = open(fileName, O_RDONLY)) == -1)
            {
                fileExists = "false";
                write(server, fileExists, strlen(fileExists) + 1);
                fprintf(stderr, "File does not exists. Please try again!!\n");
            }
            //if the file exists then send the contents to the server
            else
            {
                write(server, fileExists, strlen(fileExists) + 1);
                while ((end = read(fd, &s, 1)) == 1)
                {
                    // fprintf(stderr, "%c", s);
                    write(server, &s, 1);
                }
                if (end == 0)
                {
                    s = 4;
                    //fprintf(stderr, "%d", end);
                    write(server, &s, 1);
                }
                close(fd);
                fprintf(stderr, "File Uploaded Successfully!!\n");
            }
        }
        //if the command is quit then closes the server and exits
        else if (!strcasecmp(command, "Quit") || !strcasecmp(command, "quit"))
        {
            fprintf(stderr, "------------------------Bye Bye------------------------\n");
            close(server);
            exit(0);
        }
        //if proper option is not selected
        else
        {
            fprintf(stderr, "Please enter a valid option.\n");
        }
        fprintf(stderr, "-------------------------------------------------------\n");
    }
}