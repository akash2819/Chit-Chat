#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080

void *connection_handler(void *);

int main(int argc, char const *argv[])
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attach socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the specified port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for incoming connections...\n");

    while(1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        } 

        printf("New client connected\n");

        // Create a new thread to handle the connection
        pthread_t thread_id;
        if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &new_socket) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        // Detach the thread to allow it to run independently
        pthread_detach(thread_id);
    }

    return 0;
}

void *connection_handler(void *socket_desc)
{
    int sock = *(int*)socket_desc;
    int valread;
    char buffer[1024] = {0};
    char message[1024] = {0};

    while(1) {
        valread = read( sock , buffer, 1024);
        printf("Client message: %s\n", buffer);
        printf("Enter response: ");
        fgets(message, 1024, stdin);
        send(sock , message , strlen(message) , 0 );
        printf("Response sent\n");
    }

    return NULL;
}
