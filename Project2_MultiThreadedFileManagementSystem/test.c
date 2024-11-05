#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

void display_file_metadata(const char *filename) {
    struct stat file_stat;

    // Get file metadata
    if (stat(filename, &file_stat) == -1) {
        perror("Error getting file metadata");
        return;
    }

    // Display file size
    printf("File Size: %lld bytes\n", (long long)file_stat.st_size);

    // Display file permissions
    printf("Permissions: ");
    printf((file_stat.st_mode & S_IRUSR) ? "r" : "-");
    printf((file_stat.st_mode & S_IWUSR) ? "w" : "-");
    printf((file_stat.st_mode & S_IXUSR) ? "x" : "-");
    printf((file_stat.st_mode & S_IRGRP) ? "r" : "-");
    printf((file_stat.st_mode & S_IWGRP) ? "w" : "-");
    printf((file_stat.st_mode & S_IXGRP) ? "x" : "-");
    printf((file_stat.st_mode & S_IROTH) ? "r" : "-");
    printf((file_stat.st_mode & S_IWOTH) ? "w" : "-");
    printf((file_stat.st_mode & S_IXOTH) ? "x" : "-");
    printf("\n");

    // Display creation/modification times
    printf("Last Access Time: %s", ctime(&file_stat.st_atime));
    printf("Last Modification Time: %s", ctime(&file_stat.st_mtime));
    printf("Last Status Change Time: %s", ctime(&file_stat.st_ctime));
}

int main() {
    const char *filename = "file5.txt";
    display_file_metadata(filename);
    return 0;
}
