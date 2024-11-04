#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<semaphore.h>
#include<pthread.h>

void filereader(int client_socket){
    char filename[100];
    recv(client_socket, filename, 100, 0);
    FILE *file = fopen(filename, "r");
    if(file == NULL){
        perror("File not found\n");
        exit(1);
    }
    char buffer[1024];
    while(fgets(buffer, 1024, file)){
        send(client_socket, buffer, 1024, 0);
    }
}

void *client_handler(void *arg){
    int client_socket = *(int*)arg;
    char buffer[1024];
    
    recv(client_socket, buffer, 1024, 0);
    if(strcmp(buffer, "1") == 0){
        filereader(client_socket);
    }
}

int main(){
    int server_socket;
    struct sockaddr_in server_address;
    socklen_t server_address_size = sizeof(server_address);
    int client_socket[10];
    int i=0;
    pthread_t thread_id[10];

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket <= 0){
        perror("Socket creation failed");
        exit(1);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_socket, (struct sockaddr*)&server_address, server_address_size) < 0){
        perror("Binding failed");
        exit(1);
    }

    if(listen(server_socket, 10) < 0){
        perror("Listening failed");
        exit(1);
    }

    printf("Waiting for connections....\n");

    
    while(i<2){
        client_socket[i] = accept(server_socket, (struct sockaddr*)&server_address, &server_address_size);
        if(client_socket[i] <0){
            perror("Accept failed\n");
            exit(1);
        }
        printf("Connection established with client %d\n", i+1);
        pthread_create(&thread_id[i], NULL, client_handler, (void*)&client_socket[i]);
        i++;
    }

    for(int j=0; j<2; j++){
        pthread_join(thread_id[j], NULL);
    }

    close(server_socket);

    return 0;
}