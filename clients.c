#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>


void displayMenu() {
    printf("----- Menu -----\n");
    printf("1. Manage Account (Change Password)\n");
    printf("2.New account registration\n");
    printf("3.Upload file\n");
    printf("4. display all files\n");
    printf("5.Download file\n");
    printf("6.exit\n");
    printf("Enter your choice: ");
}

void registerNewAccount(int client_socket) {
    char username[256];
    char hpassword[256];
    

    printf("Enter your new username: ");
    scanf("%s", username);
    getchar();
    
   
    printf("Enter your new password: ");
    scanf("%s", hpassword);
    getchar();

    
    // Send username and hashed password to the server for registration
    send(client_socket, username, strlen(username), 0);
    //send(client_socket, salt, strlen(salt), 0);
    send(client_socket, hpassword, strlen(hpassword), 0);

    char registrationResult[256];
    recv(client_socket, registrationResult, sizeof(registrationResult), 0);
    printf("%s\n", registrationResult);
}


void receiveFileList(int client_socket) {
    char fileInfo[512];  // Adjust the buffer size as needed

    // Receive and print file names and sizes
    while (recv(client_socket, fileInfo, sizeof(fileInfo), 0) > 0) {
        char fileName[256];
        long fileSize;
        sscanf(fileInfo, "%s %ld", fileName, &fileSize);
        printf("File Name: %s, Size: %ld bytes\n", fileName, fileSize);
    }
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345); // Server port number
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP address

    // Connect to the server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    char username[256];
    char password[256];
    char hPassword[256];
    char newPassword[256];
    char command[256];
    char msg[256];
    int buffer[4096];
    int file_fd;
    int bytes_received;
    char filename[256];
    int choice;
     char fileListRequest[] = "list";

    printf("Enter your username: ");
    scanf("%s", username);
    
    send(client_socket, username, strlen(username), 0);

    //recv(client_socket,msg,sizeof(msg),0);
    //printf("%s",msg);

    
    
    printf("Enter your password: ");
    scanf("%s", password);

    
    send(client_socket, password, strlen(password), 0);
    
    recv(client_socket,msg,sizeof(msg),0);
    printf("%s\n",msg);
    
    while (1) {
        displayMenu();
        scanf("%d", &choice);

        switch(choice){
            case 1:
               strcpy(command, "changepassword");
               send(client_socket, command, strlen(command), 0);

               
                // Get the new password from the user
                printf("Enter your new password: ");
                scanf("%s", newPassword);
                getchar(); // Clear input buffer

                // Send the new password to the server for modification
                send(client_socket, newPassword, strlen(newPassword), 0);

                // Receive and print the result from the server
                recv(client_socket, msg, sizeof(msg), 0);
                printf("%s\n", msg);
                break;
            
            
            case 2:
                // Register new account

                printf("Enter your new username: ");
                scanf("%s", username);
                getchar();
                printf("Enter your new password: ");
                scanf("%s", hPassword);
                getchar();
                strcpy(command, "newregister");
                send(client_socket, command, strlen(command), 0);
                // Send username and hashed password to the server for registration
                send(client_socket, username, strlen(username), 0);
                send(client_socket, hPassword, strlen(hPassword), 0);

                char registrationResult[256];
                recv(client_socket, registrationResult, sizeof(registrationResult), 0);
                printf("%s\n", registrationResult);
    
    
             break;

case 3:
    strcpy(command, "upload");
    char filename[]="log.txt";
    FILE* file = fopen(filename, "rb"); // Open the file in binary mode
    if (file == NULL) {
        perror("Error opening file for reading");
        // Handle the error
        break;
    }

    // Send the upload command to the server
    send(client_socket, command, sizeof(command), 0);
    // Send filename to the server
    send(client_socket, filename, sizeof(filename), 0);

    // Read and send file data in chunks
    int bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        ssize_t bytes_sent = send(client_socket, buffer, bytes_read, 0);
        if (bytes_sent < 0) {
            perror("Error sending file data");
            // Handle the error
            fclose(file);
            break;
        }
    }
    fclose(file);
    printf("File '%s' uploaded successfully.\n", filename);
    break;

case 4:
// Request file list from the server
               
send(client_socket, fileListRequest, strlen(fileListRequest), 0);

                // Receive and display file list from the server
     receiveFileList(client_socket);
                break;


case 5:
 // Logic for downloading file
                
                
                strcpy(command,"download");
                printf("Enter the filename to download: ");
                scanf("%s", filename);

                // Send download command and filename to the server
                send(client_socket, command, sizeof(command), 0);
                send(client_socket, filename, sizeof(filename),0);

                
                
               

                // Receive and save file data
                int file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (file_fd == -1) {
                    perror("Error opening file for writing");
                } else {
                    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
                        write(file_fd, buffer, bytes_received);
                    }
                    close(file_fd);
                    printf("File '%s' downloaded successfully.\n",filename);
}                
case 6:
         // Exit the program
        close(client_socket);
        exit(EXIT_SUCCESS);
            
        default:
         printf("Invalid choice. Please try again.\n");
        }
        }
return 0;
}
