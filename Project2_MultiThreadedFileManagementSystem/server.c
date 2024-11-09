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
#include<time.h>
#include<sys/stat.h>
#include<zlib.h>
#include <glib.h>
#include<assert.h>
#include <signal.h>

#define PORT 8002
#define MAX_CLIENTS 10
#define CHUNK 16384
#define MAX_FILENAME_LEN 1004
#define MAX_COMPRESSED_FILENAME_LEN (1024 - 13)

typedef struct {
    sem_t read_semaphore;
    sem_t write_semaphore;
    int read_count;
} FileSemaphore;

GHashTable *semaphore_map;
pthread_mutex_t map_lock = PTHREAD_MUTEX_INITIALIZER;

void free_semaphore(gpointer data) {
    FileSemaphore *file_sem = (FileSemaphore *)data;
    sem_destroy(&file_sem->read_semaphore);
    sem_destroy(&file_sem->write_semaphore);
    free(file_sem);
}

void initialize_semaphore_map() {
    semaphore_map = g_hash_table_new_full(g_str_hash, g_str_equal, free, free_semaphore);
}

FileSemaphore *get_file_semaphore(const char *file_name, unsigned int initial_value) {
    pthread_mutex_lock(&map_lock);

    FileSemaphore *file_semaphore = g_hash_table_lookup(semaphore_map, file_name);
    if (!file_semaphore) {
        file_semaphore = malloc(sizeof(FileSemaphore));
        if (sem_init(&file_semaphore->read_semaphore, 0, initial_value) != 0 ||
            sem_init(&file_semaphore->write_semaphore, 0, initial_value) != 0) {
            perror("Semaphore initialization failed");
            free(file_semaphore);
            pthread_mutex_unlock(&map_lock);
            return NULL;
        }
        g_hash_table_insert(semaphore_map, g_strdup(file_name), file_semaphore);
    }

    pthread_mutex_unlock(&map_lock);
    return file_semaphore;
}

void destroy_all_semaphores() {
    pthread_mutex_lock(&map_lock);
    g_hash_table_destroy(semaphore_map);
    pthread_mutex_unlock(&map_lock);
}

void signal_handler(int signum) {
    switch(signum) {
        case SIGINT:
            printf("Server shutdown initiated by SIGINT\n");
            destroy_all_semaphores();
            exit(0);
            break;
        case SIGSEGV:
            fprintf(stderr, "Segmentation fault occurred\n");
            destroy_all_semaphores();
            exit(1);
            break;
        case SIGUSR1:
            fprintf(stderr, "Custom signal SIGUSR1 received - Permission Error\n");
            break;
        case SIGUSR2:
            fprintf(stderr, "Custom signal SIGUSR2 received - File operation error\n");
            break;
    }
}

void setup_signal_handling() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
}

int check_file_access(const char* filename) {
    if (strcmp(filename, "log.txt") == 0) {
        raise(SIGUSR1);  // Raise permission error signal
        return 0;
    }
    return 1;
}

char *null_string = "";

int read_count;
sem_t read_mutex, write_mutex;

void operation_logging(int client_socket, int operation, char *filename_accessed, char *newname, char *log_message){
    FILE *log_file = fopen("log.txt", "a");
    if(log_file==NULL){
        perror("Log file not found\n");
        exit(1);
    }
    time_t current_time;
    struct tm * time_info;
    time(&current_time);
    time_info = localtime(&current_time);

    if(operation == 1){
        fprintf(log_file, "File %s read at %s by %d STATUS : %s\n", filename_accessed, asctime(time_info), client_socket,log_message);
    }
    if(operation == 2){
        fprintf(log_file, "File %s written at %s by %d STATUS : %s\n", filename_accessed, asctime(time_info), client_socket,log_message);
    }
    if(operation == 3)
    {
        fprintf(log_file, "File %s deleted at %s by %d STATUS : %s\n", filename_accessed, asctime(time_info), client_socket,log_message);
    }
    if(operation == 4){
        fprintf(log_file, "File %s renamed to %s at %s by %d STATUS : %s\n", filename_accessed, newname, asctime(time_info), client_socket, log_message);
    }
    if(operation == 5){
        fprintf(log_file, "File %s copied at %s by %d STATUS : %s\n", filename_accessed, asctime(time_info), client_socket,log_message);
    }
    if(operation == 6){
        fprintf(log_file, "Metadata of file %s accessed at %s by %d STATUS %s\n", filename_accessed, asctime(time_info), client_socket,log_message);
    }
    if(operation == 7):
    {
        fprintf(log_file, "File %s compression operation at %s by %d STATUS : %s\n", 
        filename_accessed, asctime(time_info), client_socket, log_message);
    }
    if(operation == 8):
    {
        fprintf(log_file, "File %s decompression operation at %s by %d STATUS : %s\n", 
        filename_accessed, asctime(time_info), client_socket, log_message);
    }
    fclose(log_file);
}

void file_decompression(int client_socket) {
    char compressed_filename[1024];
    recv(client_socket, compressed_filename, sizeof(compressed_filename), 0);
    printf("Compressed file name received: %s\n", compressed_filename);

    // Open the compressed file for reading
    FILE *compressed_file = fopen(compressed_filename, "rb");
    if (!compressed_file) {
        perror("Could not open compressed file");
        operation_logging(client,8,filename,null_string,"Could not open compressed file");
        return;
    }

    // Get the size of the compressed file
    fseek(compressed_file, 0, SEEK_END);
    long compressed_size = ftell(compressed_file);
    rewind(compressed_file);

    // Allocate memory to read compressed data
    char *compressed_data = malloc(compressed_size);
    if (!compressed_data) {
        perror("Memory allocation failed for compressed data");
        operation_logging(client,8,filename,null_string,"Memory allocation failed for compressed data");
        fclose(compressed_file);
        return;
    }

    // Read the compressed data into memory
    fread(compressed_data, 1, compressed_size, compressed_file);
    fclose(compressed_file);

    // Guess the decompressed size
    uLongf decompressed_size = compressed_size * 4;  // This is an initial guess, may need adjustment
    char *decompressed_data = malloc(decompressed_size);
    if (!decompressed_data) {
        perror("Memory allocation failed for decompressed data");
        operation_logging(client,8,filename,null_string,"Memory allocation failed for decompressed data");
        free(compressed_data);
        return;
    }

    // Decompress the data
    int result = uncompress((Bytef *)decompressed_data, &decompressed_size, (const Bytef *)compressed_data, compressed_size);
    if (result != Z_OK) {
        fprintf(stderr, "Decompression failed with code %d\n", result);
        operation_logging(client,8,filename,null_string,"Decompression failed with code");
        free(compressed_data);
        free(decompressed_data);
        return;
    }

    // Construct the decompressed file name
    char decompressed_filename[1024];
    snprintf(decompressed_filename, sizeof(decompressed_filename), "decompressed_%.1009s", compressed_filename);

    // Write the decompressed data to the new file
    FILE *decompressed_file = fopen(decompressed_filename, "wb");
    if (!decompressed_file) {
        perror("Could not create decompressed file");
        operation_logging(client,8,filename,null_string,"Could not create decompressed file");
        free(compressed_data);
        free(decompressed_data);
        return;
    }

    fwrite(decompressed_data, 1, decompressed_size, decompressed_file);
    printf("File decompressed and saved as: %s\n", decompressed_filename);
    operation_logging(client,8,filename,null_string,"Decompression success");

    // Clean up
    fclose(decompressed_file);
    free(compressed_data);
    free(decompressed_data);
}


char *readcontent(const char *filename, int *input_size) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("File not found");
        raise(SIGUSR2);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    *input_size = ftell(file);
    rewind(file);

    char *content = malloc(*input_size);
    fread(content, 1, *input_size, file);
    fclose(file);

    return content;
}

void file_compression(int client_socket) {
    char filename[1024] = {0};
    recv(client_socket, filename, 1024, 0);
    printf("File name received: %s\n", filename);

    int input_size;
    char *content_of_file = readcontent(filename, &input_size);

    uLongf compressed_data_size = compressBound(input_size);
    char *compressed_data = malloc(compressed_data_size);
    int result = compress((Bytef *)compressed_data, &compressed_data_size, (const Bytef *)content_of_file, input_size);

    if (result != Z_OK) {
        fprintf(stderr, "Compression failed with code %d\n", result);
        operation_logging(client,7,filename,null_string,"Compression failed with code");
        free(content_of_file);
        free(compressed_data);
        return;
    }

    // Creating a compressed filename with a .gz extension
    char compress_filename[1024];
    snprintf(compress_filename, sizeof(compress_filename), "compressed_%.1004s", filename);

    // Writing the compressed data to a .gz file
    FILE *compressed_file = fopen(compress_filename, "wb");
    if (!compressed_file) {
        perror("Could not create compressed file");
        operation_logging(client,7,filename,null_string,"Could not create compressed file");
        free(content_of_file);
        free(compressed_data);
        return;
    }

    fwrite(compressed_data, 1, compressed_data_size, compressed_file);
    printf("File compressed and saved as: %s\n", compress_filename);
    operation_logging(client,7,filename,null_string,"Compression successful");

    fclose(compressed_file);
    free(content_of_file);
    free(compressed_data);
}


void filereader(int client_socket){
    char filename[1024];
    recv(client_socket, filename, 1024, 0);
    printf("File name received %s\n",filename);

    if (!check_file_access(filename)) {
        raise(SIGUSR1);
        return;
    }

    FileSemaphore *file_semaphore = get_file_semaphore(filename, 1);
    if(file_semaphore)
    {
        sem_wait(&file_semaphore->read_semaphore);
        file_semaphore->read_count++;
        if(file_semaphore->read_count == 1){
            sem_wait(&file_semaphore->write_semaphore);
        }

        sem_post(&file_semaphore->read_semaphore);

        FILE *file = fopen(filename, "r");
        if(file == NULL){
            perror("File not found\n");
            raise(SIGUSR2);
            operation_logging(client_socket, 1, filename, null_string, "File read failed");
            exit(1);
        }
        char buffer[1024];
        while(fgets(buffer, 1024, file)){
            send(client_socket, buffer, 1024, 0);
        }
        operation_logging(client_socket, 1, filename, null_string, "File read successfully");
        
        sem_wait(&file_semaphore->read_semaphore);
        file_semaphore->read_count--;
        if(file_semaphore->read_count == 0){
            sem_post(&file_semaphore->write_semaphore);
        }
        sem_post(&file_semaphore->read_semaphore);
    }
}

void filewriter(int client_socket) {
    char filename[1024];
    memset(filename, 0, sizeof(filename));
    recv(client_socket, filename, 1024, 0);
    printf("File name received %s\n", filename);
    fflush(stdout);
    if (!check_file_access(filename)) {
        raise(SIGUSR1);
        return;
    }
    FileSemaphore *file_semaphore = get_file_semaphore(filename, 1);
    if(file_semaphore)
    {
        sem_wait(&file_semaphore->write_semaphore);

        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));


        FILE *file = fopen(filename, "a");
        if (file == NULL) {
            perror("File not found\n");
            raise(SIGUSR2);
            exit(1);
        }

        char buffer[1024];
        while (1) {
            memset(buffer, 0, sizeof(buffer));
            int bytes_received = recv(client_socket, buffer, 1024, 0);

            if (bytes_received <= 0) {
                break;
            }

            printf("%s", buffer);
            fflush(stdout);
            
            fprintf(file, "%s", buffer);
        }

        send(client_socket, "File written successfully", 1024, 0);

        operation_logging(client_socket, 2, filename, null_string, "File write successful");

        fclose(file);
        sem_post(&file_semaphore->read_semaphore);
    }
    else
    {
        send(client_socket, "File semaphore not found", 1024, 0);
        operation_logging(client_socket,2,filename,null_string,"File write not successful");
    }

}


void file_deletion(int client_socket){
    char filename[1024];
    recv(client_socket,filename,1024,0);
    printf("File name received %s\n",filename);
    if (!check_file_access(filename)) {
        raise(SIGUSR1);
        return;
    }
    FileSemaphore *file_semaphore = get_file_semaphore(filename, 1);
    if(file_semaphore)
    {
        sem_wait(&file_semaphore->write_semaphore);
        fflush(stdout);

        if(remove(filename)<0){
            perror("File deletion failed\n");
             raise(SIGUSR2);
            operation_logging(client_socket,3,filename,null_string,"File Deletion Failed");
            exit(1);
        }
        send(client_socket, "File deleted successfully\n",1024,0);
        operation_logging(client_socket, 4, filename, null_string, "File deleted successfully");

        sem_post(&file_semaphore->write_semaphore);
    }
}

void file_renamer(int client_socket){
    char filename[1024];
    recv(client_socket, filename, 1024, 0);
    printf("File name received %s\n",filename);
    if (!check_file_access(filename)) {
        raise(SIGUSR1);
        return;
    }
    FileSemaphore *file_semaphore = get_file_semaphore(filename, 1);
    if(file_semaphore)
    {
        sem_wait(&file_semaphore->write_semaphore);
        fflush(stdout);
        char newname[1024];
        recv(client_socket, newname, 1024, 0);  

        printf("New name received %s\n",newname);
        fflush(stdout);
        if(rename(filename, newname) < 0){
            perror("File renaming failed\n");
            raise(SIGUSR2);
            operation_logging(client_socket, 4, filename, newname, "File renaming failed");
            exit(1);
        }
        send(client_socket, "File renamed successfully", 1024, 0);
        operation_logging(client_socket, 4, filename, newname, "File renamed successfully");
        sem_post(&file_semaphore->write_semaphore);
    }
}

void metadata_display(int client_socket) {
    char filename[1024];
    struct stat file_stat;
    memset(filename, 0, sizeof(filename));

    recv(client_socket, filename, 1024, 0);
    printf("File name received for metadata display: %s\n", filename);
    fflush(stdout);
    if (!check_file_access(filename)) {
        raise(SIGUSR1);
        return;
    }

    FileSemaphore *file_semaphore = get_file_semaphore(filename, 1);
    if(file_semaphore)
    {
        sem_wait(&file_semaphore->read_semaphore);

        file_semaphore->read_count++;
        if(file_semaphore->read_count == 1){
            sem_wait(&file_semaphore->write_semaphore);
        }

        sem_post(&file_semaphore->read_semaphore);


        if (stat(filename, &file_stat) == -1) {
            perror("Error getting file metadata");
            raise(SIGUSR2);
            send(client_socket, "Error: File not found or unable to access metadata.\n", 1024, 0);
            operation_logging(client_socket, 6, filename, null_string, "File metadata access failed");
            return;
        }

        char metadata[4096]; 
        snprintf(metadata, sizeof(metadata), 
                "File Size: %lld bytes\n"
                "Permissions: %c%c%c%c%c%c%c%c%c\n"
                "Last Access Time: %s"
                "Last Modification Time: %s"
                "Last Status Change Time: %s",
                (long long)file_stat.st_size,
                (file_stat.st_mode & S_IRUSR) ? 'r' : '-',
                (file_stat.st_mode & S_IWUSR) ? 'w' : '-',
                (file_stat.st_mode & S_IXUSR) ? 'x' : '-',
                (file_stat.st_mode & S_IRGRP) ? 'r' : '-',
                (file_stat.st_mode & S_IWGRP) ? 'w' : '-',
                (file_stat.st_mode & S_IXGRP) ? 'x' : '-',
                (file_stat.st_mode & S_IROTH) ? 'r' : '-',
                (file_stat.st_mode & S_IWOTH) ? 'w' : '-',
                (file_stat.st_mode & S_IXOTH) ? 'x' : '-',
                ctime(&file_stat.st_atime),
                ctime(&file_stat.st_mtime),
                ctime(&file_stat.st_ctime));

        send(client_socket, metadata, sizeof(metadata), 0);
        operation_logging(client_socket, 6, filename, null_string, "File metadata accessed successfully");
        
        sem_wait(&file_semaphore->read_semaphore);
        file_semaphore->read_count--;
        if(file_semaphore->read_count == 0){
            sem_post(&file_semaphore->write_semaphore);
        }
        sem_post(&file_semaphore->read_semaphore);
    }
}

void file_copy(int client_socket){

    char filename[1024];
    
    recv(client_socket, filename, 1024, 0);
    printf("File name received %s\n",filename);
    if (!check_file_access(filename)) {
        raise(SIGUSR1);
        return;
    }
    FileSemaphore *file_semaphore = get_file_semaphore(filename, 1);
    if(file_semaphore)
    {
        sem_wait(&file_semaphore->read_semaphore);
        file_semaphore->read_count++;
        if(file_semaphore->read_count == 1){
            sem_wait(&file_semaphore->write_semaphore);
        }

        sem_post(&file_semaphore->read_semaphore);

        FILE *file = fopen(filename, "r");
        if(file == NULL){
            perror("File not found\n");
            operation_logging(client_socket, 5, filename, null_string, "File copy failed");
            exit(1);
        }
        char buffer[1024];
        while(fgets(buffer, 1024, file)){
            send(client_socket, buffer, 1024, 0);
        }
        operation_logging(client_socket, 5, filename, null_string, "File copied successfully");
        sem_wait(&file_semaphore->read_semaphore);
        file_semaphore->read_count--;
        if(file_semaphore->read_count == 0){
            sem_post(&file_semaphore->write_semaphore);
        }
        sem_post(&file_semaphore->read_semaphore);
    }
    else
    {
        operation_logging(client_socket, 5, filename, null_string, "File copy failed");
        return;
    }

}


void *client_handler(void *arg){
    int client_socket = (int)arg;
    char buffer[1024];
    int choice;
    while (1)
    {
        printf("Waiting for command from client\n");
        int received = recv(client_socket, &choice, sizeof(choice), 0);
        if(received < 0)
        {
            break;
        }
        printf("%d",choice);
        if(choice == 1){
            filereader(client_socket);

        }
        if(choice == 2)
        {
            filewriter(client_socket);
        }
        if(choice == 3)
        {
            file_deletion(client_socket);
        }
        if(choice == 4)
        {
            file_renamer(client_socket);
        }
        if(choice == 5)
        {
            file_copy(client_socket);
        }
        if(choice == 6)
        {
            metadata_display(client_socket);
        }
        if(choice == 7){
            file_compression(client_socket);
        }
        if(choice == 8){
            file_decompression(client_socket);
        }
        if(choice == 9)
        {
            break;
        }
    }
    
    close(client_socket);
    return NULL;
}


int main(){
    setup_signal_handling();
    initialize_semaphore_map();
    int server_socket;
    struct sockaddr_in server_address;
    socklen_t server_address_size = sizeof(server_address);
    int client_socket[MAX_CLIENTS];
    int client_count = 0;
    pthread_t thread_id[MAX_CLIENTS];
    srand(time(0));

    sem_init(&read_mutex, 0, 1);
    sem_init(&write_mutex, 0, 1);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket <= 0){
        perror("Socket creation failed");
        exit(1);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_socket, (struct sockaddr*)&server_address, server_address_size) < 0){
        perror("Binding failed");
        exit(1);
    }

    if(listen(server_socket, MAX_CLIENTS) < 0){
        perror("Listening failed");
        exit(1);
    }

    printf("Waiting for connections....\n");

    while(1){
        client_socket[client_count] = accept(server_socket, (struct sockaddr*)&server_address, &server_address_size);
        if(client_socket[client_count] <0){
            perror("Accept failed\n");
            exit(1);
        }
        printf("Connection established with client %d\n", client_count+1);
        pthread_create(&thread_id[client_count], NULL, client_handler, (void*)&client_socket[client_count]);
        // printf("HELLO\n");
        client_count++;
        // printf("%d\n", client_count);

        if(client_count == MAX_CLIENTS){
            for(int j=0; j<MAX_CLIENTS; j++){
                pthread_join(thread_id[j], NULL);
            }
            client_count = 0;
        }
    }
    destroy_all_semaphores();
    for(int i=0;i<client_count;i++)
    {
        close(client_socket[i]);
    }

    close(server_socket);

    return 0;
}