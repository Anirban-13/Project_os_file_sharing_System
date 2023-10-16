#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include<dirent.h>
#include<semaphore.h>
#include<ctype.h>
#define FILE_STORAGE_DIRECTORY "./home/anubhav/coding2/files/"

struct ClientInfo {
    int socket;
};


struct Credentials {
    char username[256];
    char hashedPassword[256];
};

struct Credentials userCredentials[] = {
    {"username1", "hashed_password1"},
    {"username2", "hashed_password2"},
    {"username3", "hashed_password3"},
    {"username4", "hashed_password4"},
    {"username5", "hashed_password5"},
    {"username6", "hashed_password6"},
    {"username7", "hashed_password7"},
    {"username8", "hashed_password8"},
    {"username9", "hashed_password9"},
    {"username10", "hashed_password10"},
    {"username11", "hashed_password11"},

};

int numUsers = sizeof(userCredentials) / sizeof(userCredentials[0]);

int isValidUsername(const char* username) {
    // Check if the username contains only alphanumeric characters
    for (int i = 0; username[i] != '\0'; i++) {
        if (!isalnum(username[i])) {
            return 0; // Invalid username
        }
    }
    return 1; // Valid username
}

int authenticateClient(const char* username, const char* receivedHashedPassword) {
    int i;
    for (i = 0; i < sizeof(userCredentials) / sizeof(userCredentials[0]); ++i) {
        if (strcmp(userCredentials[i].username, username) == 0 &&
            strcmp(userCredentials[i].hashedPassword, receivedHashedPassword) == 0) {
            return 1; // Authentication successful
        }
    }
    return 0; // Authentication failed
}


void handleFileUpload(int client_socket, const char* filename) {

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s%s", FILE_STORAGE_DIRECTORY, filename);

    // Open the file for writing
    int file_fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC|O_BINARY, 0666);
    if (file_fd == -1) {
        perror("Error opening file for writing");
        return;
    }

    // Receive and save the file data
    char buffer[4096];
    int bytes_received;
    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        write(file_fd, buffer, bytes_received);
    }

    close(file_fd);
    printf("File '%s' uploaded successfully.\n", filename);
    
}

void handleFileDownload(int client_socket, const char* filename) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s%s", FILE_STORAGE_DIRECTORY, filename);

    // Open the file for reading
    int file_fd = open(filepath, O_RDONLY);
    if (file_fd == -1) {
        perror("Error opening file for reading");
        // Send an error message to the client if the file could not be opened
        char errorMessage[] = "File not found or could not be opened.";
        send(client_socket, errorMessage, sizeof(errorMessage), 0);
        return;
    }

    // Send the file data to the client
    char buffer[1024];
    int bytes_read;
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }

    close(file_fd);
    printf("File '%s' sent to client.\n", filename);
}


void sendFileList(int client_socket) {
    struct dirent *entry;
    DIR *dp = opendir("./files"); // Open the directory

    if (dp == NULL) {
        perror("Error opening directory");
        return;
    }

    // Buffer to store file information (name and size)
    char fileInfo[4096];  // Adjust the buffer size as needed

    // Send each filename and size to the client
    while ((entry = readdir(dp))) {
        if (entry->d_type == DT_REG) {  // If it's a regular file
            snprintf(fileInfo, sizeof(fileInfo), "%s %ld\n", entry->d_name, (long)entry->d_reclen);
            send(client_socket, fileInfo, strlen(fileInfo), 0);
        }
    }

    closedir(dp);  // Close the directory
}

// Modify the function to handle password change requests
void handleChangePassword(int client_socket,char *username,char *new_password) {
    // Code to store the new password for the client
    // ...
    const size_t numUsers = sizeof(userCredentials) / sizeof(userCredentials[0]);
    int i;

    for (i = 0; i < numUsers; ++i) {
        if (strncmp(userCredentials[i].username, username, sizeof(userCredentials[i].username) - 1) == 0) {
            strncpy(userCredentials[i].hashedPassword, new_password, sizeof(userCredentials[i].hashedPassword) - 1);
            userCredentials[i].hashedPassword[sizeof(userCredentials[i].hashedPassword) - 1] = '\0'; // Null-terminate the string
            const char* changeSuccessMessage = "Password changed successfully.";
            send(client_socket, changeSuccessMessage, strlen(changeSuccessMessage), 0);
            return;
        }
    }

    // If username not found, send an error message to the client
    const char* changeFailureMessage = "Username not found. Password change failed.";
    send(client_socket, changeFailureMessage, strlen(changeFailureMessage), 0);
}


void* handleClient(void* arg) {

    struct ClientInfo* clientInfo = (struct ClientInfo*)arg;
    int client_socket = clientInfo->socket;

    // Receive hashed password from the client
    char username[256];
    char receivedHashedPassword[256];
   

   
    // Receive username and hashed password from the client
    recv(client_socket, username, sizeof(username), 0);

    recv(client_socket, receivedHashedPassword, sizeof(receivedHashedPassword), 0);

    // Check the validity of the username
    if (!isValidUsername(username)) {
        const char* invalidUsernameMessage = "Invalid username format. Please use alphanumeric characters only.";
        send(client_socket, invalidUsernameMessage, strlen(invalidUsernameMessage), 0);
        close(client_socket);
        free(clientInfo);
        pthread_exit(NULL);
    }
    // Check client authentication
    if (authenticateClient(username, receivedHashedPassword)) {
        // Authentication successful
        const char* authSuccessMessage = "Authentication successful.";
        send(client_socket, authSuccessMessage, strlen(authSuccessMessage), 0);
        
        // Receive client's command
        char command[256];
        recv(client_socket, command, sizeof(command), 0);
        
    if (strcmp(command, "changepassword") == 0) {
        printf("changepassword command recieved from the client!!\n");
        char new_password[256];
        recv(client_socket, new_password, sizeof(new_password), 0);
        handleChangePassword(client_socket,username,new_password);
    } 


    else if(strcmp(command, "upload") == 0) {
        printf("upload command recieved from the client!!\n");
       
        char filename[256];
        recv(client_socket, filename, sizeof(filename), 0);
        handleFileUpload(client_socket, filename);
    }

else if(strcmp(command, "download") == 0) {
        printf("download command recieved from the client!!\n");
       
        char filename[256];
        recv(client_socket, filename, sizeof(filename), 0);
        handleFileDownload(client_socket, filename);
    }


else if (strcmp(command, "list") == 0) {
            printf("Command received from the client: Get File List\n");
            sendFileList(client_socket);
}



    else {

        printf("Invalid command received from client.\n");
    }
}

else{
    const char* authFailureMessage = "Authentication failed. Invalid username or password.";
    send(client_socket, authFailureMessage, strlen(authFailureMessage), 0);
}
 // Client disconnected, clean up and exit the thread
    close(client_socket);
    free(clientInfo);
    pthread_exit(NULL);
}

int main() {

  
    int primary_server_socket;
    struct sockaddr_in primary_server_addr;
    pthread_t primary_thread_id;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);


    

    // Create socket
    primary_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (primary_server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address struct
    primary_server_addr.sin_family = AF_INET;
    primary_server_addr.sin_port = htons(12345); // Port number
    primary_server_addr.sin_addr.s_addr = INADDR_ANY; // Accept connections on any network interface

    // Bind the socket
    if (bind(primary_server_socket, (struct sockaddr*)&primary_server_addr, sizeof(primary_server_addr)) == -1) {
        perror("Binding failed");
        close(primary_server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(primary_server_socket, 5) == -1) {
        perror("Listening failed");
        close(primary_server_socket);
        exit(EXIT_FAILURE);
    }
    
    

    printf("Primary Server listening on port 12345...\n");

while (1) {
        int primary_client_socket = accept(primary_server_socket, (struct sockaddr*)&primary_server_addr, &client_addr_len);
        if (primary_client_socket == -1) {
            perror("Acceptance failed");
            continue;
        }

        struct ClientInfo* clientInfo1 = malloc(sizeof(struct ClientInfo));
        clientInfo1->socket = primary_client_socket;

        if (pthread_create(&primary_thread_id, NULL, handleClient, (void*)clientInfo1) != 0) {
            perror("Thread creation failed");
            close(primary_client_socket);
            free(clientInfo1);
        } else {
            printf("New client connected to primary server. Thread created.\n");
        }
    }
       

    // Close server sockets
    close(primary_server_socket);
   // close(redundant_server_socket);

    return 0;
}
