#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8000
#define BUFFER_SIZE 1000

int client_socket;

void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            break;
        }
        printf("%s", buffer);
    }
    return NULL;
}

void send_command(const char *command) {
    send(client_socket, command, strlen(command), 0);
}

void send_command1(int command) {
    send(client_socket, &command, sizeof(command), 0);
}

int main()
{
    struct sockaddr_in server_addr;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("Connected to the server.\n");

    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_messages, NULL);

    while (1)
    {
        printf("\nChoose an option:\n");
        printf("1. Concurrent File Reading\n");
        printf("2. Exclusive File Writing\n");
        printf("3. File Deletion\n");
        printf("4. File Renaming\n");
        printf("5. File Copying\n");
        printf("6. File Metadata Display\n");
        printf("7. Error Handling\n");
        printf("8. Logging Operations\n");
        printf("9. Compression and decompression of files\n");

        int choice;
        scanf("%d", &choice);

        char command[BUFFER_SIZE];
        memset(command, 0, BUFFER_SIZE);
        int command2;

        switch(choice)
        {
            case 1:
                command2 = 1;
                send_command1(command2);
                printf("File to read :");
                scanf("%s",command);
                send_command(command);
                break;
            case 2:
                snprintf(command,sizeof(command),"File_Writing");
                send_command(command);
                printf("File to write on :");
                scanf("%s",command);
                send_command(command);
                break;
            case 3:
                snprintf(command,sizeof(command),"File_Deletion");
                send_command(command);
                printf("File to delete:");
                scanf("%s",command);
                send_command(command);
            case 4:
                snprintf(command,sizeof(command),"File_Renaming");
                send_command(command);
                printf("File to rename:");
                scanf("%s",command);
                send_command(command);
            case 5:
                snprintf(command,sizeof(command),"File_copying");
                send_command(command);
                printf("File to copy:");
                scanf("%s",command);
                send_command(command);
            case 6:
                snprintf(command,sizeof(command),"File_meta_data");
                send_command(command);
                printf("File to get meta data:");
                scanf("%s",command);
                send_command(command);
            case 7:
                snprintf(command,sizeof(command),"Error_Handling");
                send_command(command);
            case 8:
                snprintf(command,sizeof(command),"Logging_Operation");
                send_command(command);
                printf("File to logging:");
                scanf("%s",command);
                send_command(command);
            case 9:
                snprintf(command,sizeof(command),"Compress_file");
                send_command(command);
                printf("File to compress:");
                scanf("%s",command);
                send_command(command);
            default:
                printf("Invalid option.\n");
                break;
        }
    }
    
}