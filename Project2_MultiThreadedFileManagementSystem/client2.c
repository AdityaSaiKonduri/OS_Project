#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <time.h>

#define PORT 8010
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
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            break;
        }
        if(FILE *file = fopen(filename, "a") <0){
            perror("File not found\n");
            exit(1);
        }
        fprintf(file, "%s", buffer);

    }
    printf("\nEOF");
}

void file_copy_client(){
    char copy_filename[BUFFER_SIZE];
    printf("Enter the name of the file to copy into: ");
    scanf("%s", copy_filename);
    receive_messages(copy_filename);
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
    timeout.tv_sec = 1;
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
        printf("7. Error Handling\n");
        printf("8. Logging Operations\n");
        printf("9. Compression and decompression of files\n");

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
            snprintf(command, sizeof(command), "File_Writing");
            send_command(command);
            printf("File to write on : ");
            scanf("%s", command);
            send_command(command);
        } else if (choice == 3) {
            snprintf(command, sizeof(command), "File_Deletion");
            send_command(command);
            printf("File to delete: ");
            char command3[100];
            scanf("%s", command3);
            send_command(command3);
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

        } else if (choice == 6) {
            command2 = 6;
            send_command1(command2);
            printf("File to get meta data: ");
            scanf("%s", command);
            send_command(command);
            receive_messages();
        } else if (choice == 7) {
            snprintf(command, sizeof(command), "Error_Handling");
            send_command(command);
        } else if (choice == 8) {
            snprintf(command, sizeof(command), "Logging_Operation");
            send_command(command);
            printf("File to log: ");
            scanf("%s", command);
            send_command(command);
        } 
        else if (choice == 9) {
            snprintf(command, sizeof(command), "Compress_file");
            send_command(command);
            printf("File to compress: ");
            scanf("%s", command);
            send_command(command);
        }
        else if (choice == 10)
        {
            command2 = 10;
            send_command1(command2);
            break;
        }
         else {
            printf("Invalid option.\n");
            continue;
        }
        
    }
}
