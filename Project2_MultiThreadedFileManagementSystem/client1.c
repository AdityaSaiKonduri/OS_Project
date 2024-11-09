#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <time.h>

#define PORT 8001
#define BUFFER_SIZE 4096

int client_socket;
sem_t mutex;

void receive_messages() {
    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            break;
        }
        printf("%s", buffer);
        fflush(stdout);
    }
    printf("\nEOF");
}

void receive_messages_for_copy(char *filename) {
    char buffer[BUFFER_SIZE];
    FILE *file =fopen(filename, "a");
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            break;
        }
        if(file<0){
            perror("File not found\n");
            exit(1);
        }
        printf("%s",buffer);
        fprintf(file, "%s", buffer);

    }
    fclose(file);
    printf("\nEOF");
}

void file_copy_client(){
    char copy_filename[BUFFER_SIZE];
    printf("Enter the name of the file to copy into: ");
    scanf("%s", copy_filename);
    receive_messages_for_copy(copy_filename);
}

void send_command(const char *command) {
    send(client_socket, command, strlen(command), 0);
}

void send_command1(int command) {
    send(client_socket, &command, sizeof(command), 0);
}

int main() {
    //sem_init(&mutex, 0, 1);
    struct sockaddr_in server_addr;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }
    printf("Connected to the server.\n");

    struct timeval timeout;
    timeout.tv_sec = 7;
    timeout.tv_usec = 0;
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    srand(time(0));

    while (1) {
        printf("\nChoose an option:\n");
        printf("1. Concurrent File Reading\n");
        printf("2. Exclusive File Writing\n");
        printf("3. File Deletion\n");
        printf("4. File Renaming\n");
        printf("5. File Copying\n");
        printf("6. File Metadata Display\n");
        printf("7. Compression\n");
        printf("8. Decompression\n");
        printf("9. Exit\n");

        int choice;
        scanf("%d", &choice);
        getchar(); 

        char command[BUFFER_SIZE];
        memset(command, 0, BUFFER_SIZE);
        int command2;

        if (choice == 1) {
            command2 = 1;
            send_command1(command2);
            printf("File to read : ");
            scanf("%s", command);
            send_command(command);
            receive_messages();
        } else if (choice == 2) {
            command2 = 2;
            send_command1(command2);
            printf("File to write : ");
            scanf("%s", command);
            send_command(command);
            printf("Text to write");
            // fgets(command,BUFFER_SIZE,stdin);
            // command[strcspn(command, "\n")] = 0;
            // scanf("%s",command);
            getchar();
            // scanf("%[^\n]%*c",command);
            // Read the text to write with fgets
            // printf("Text to write: ");
            fgets(command, BUFFER_SIZE, stdin);
            command[strcspn(command, "\n")] = 0;
            send_command(command);
            receive_messages();

        } else if (choice == 3) {
            command2 = 3;
            send_command1(command2);
            printf("File to delete : ");
            scanf("%s", command);
            send_command(command);
            ;
            receive_messages();
        } else if (choice == 4) {
            command2 = 4;
            send_command1(command2);
            printf("File to rename: ");
            scanf("%s", command);
            send_command(command);
            char command3[100];
            printf("New file name :");
            scanf("%s",command3);
            send_command(command3);
            receive_messages();
        } else if (choice == 5) {
            command2 = 5;
            send_command1(command2);
            printf("File to copy: ");
            scanf("%s", command);
            send_command(command);
            file_copy_client();

        } else if (choice == 6) {
            command2 = 6;
            send_command1(command2);
            printf("File to get meta data: ");
            scanf("%s", command);
            printf("Requesting meta data for %s",command);
            send_command(command);
            receive_messages();
        } 
        else if (choice == 7) {
            command2 = 7;
            send_command1(command2);
            printf("File to compress: ");
            scanf("%s", command);
            send_command(command);
        }
        else if (choice == 8) {
            command2 = 8;
            send_command1(command2);
            printf("File to decompress: ");
            scanf("%s", command);
            send_command(command);
        }
        else if (choice == 9)
        {
            command2 = 9;
            send_command1(command2);
            break;
        }
         else {
            printf("Invalid option.\n");
            continue;
        }
        
    }
}
