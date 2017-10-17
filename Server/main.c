#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <zconf.h>
#include <signal.h>

#define PORT 5223
#define TRUE 1
#define BUFSIZE 1024

char **serverBuf;
int serverBufIndex = 0;
pthread_mutex_t mutex;

struct client {
    int clientSocket;
    struct sockaddr_in clientAddr;
};
void *idleFunc(void* thread) {
    pthread_t pthread_self = *(pthread_t*)thread;
    while(TRUE) {
        pthread_mutex_lock(&mutex);
        serverBuf[serverBufIndex] = (char*) malloc(sizeof(char) * 50);
        sprintf(serverBuf[serverBufIndex], "[%lu]: idle\n", pthread_self);
        serverBufIndex++;
        pthread_mutex_unlock(&mutex);
    }
}
void *clientFunc(void* ptr) {
    struct client* clientPtr = (struct client*) ptr;

    pthread_mutex_lock(&mutex);
    serverBuf[serverBufIndex] = (char*) malloc(sizeof(char) * 50);
    sprintf(serverBuf[serverBufIndex], "[%d]: accept new client %s\n", clientPtr->clientSocket, inet_ntoa(clientPtr->clientAddr.sin_addr));
    serverBufIndex++;
    pthread_mutex_unlock(&mutex);

    char buf[BUFSIZE];
    int byteRead = recv(clientPtr->clientSocket, buf, BUFSIZE - 1, 0);
    buf[byteRead] = '\0';

    pthread_mutex_lock(&mutex);
    serverBuf[serverBufIndex] = (char*) malloc(sizeof(char) * strlen(buf));
    strcat(serverBuf[serverBufIndex], buf);
    serverBufIndex++;
    pthread_mutex_unlock(&mutex);

    send(clientPtr->clientSocket, buf, strlen(buf), 0);

    pthread_mutex_lock(&mutex);
    serverBuf[serverBufIndex] = (char*) malloc(sizeof(char) * 50);
    sprintf(serverBuf[serverBufIndex], "[%d]: client %s disconnected\n", clientPtr->clientSocket, inet_ntoa(clientPtr->clientAddr.sin_addr));
    serverBufIndex++;
    pthread_mutex_unlock(&mutex);
}
int initServerSocket() {
    struct sockaddr_in addr;
    int serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1\0");
    bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr));
    listen(serverSocket, 5);
    return serverSocket;
}
int initClientSocket(struct sockaddr_in clientAddr, int serverSocket) {
    int clientLen = sizeof(clientAddr);
    return accept(serverSocket, (struct sockaddr *) &clientAddr, &clientLen);
}
void signal_handler() {
    FILE *file = fopen("tmp/buf.txt", "w");
    for (int index = 0; index < serverBufIndex; index++) {
        fprintf(file, "%s", serverBuf[index]);
        free(serverBuf[index]);
    }
    free(serverBuf);
    fclose(file);
    exit(0);
}

int main() {
    serverBuf = (char**) malloc(sizeof(char*) * BUFSIZE);

    int serverSocket = initServerSocket();
    pthread_mutex_init(&mutex, NULL);

    signal(SIGINT, signal_handler);

    while (TRUE) {
        struct sockaddr_in clientAddr;
        int clientSocket = initClientSocket(clientAddr, serverSocket);

        struct client clientPtr;
        clientPtr.clientSocket = clientSocket;
        clientPtr.clientAddr = clientAddr;

        pthread_t clientPthread;
        pthread_create(&clientPthread, NULL, clientFunc, &clientPtr);

        //pthread_t idleThread;
        //pthread_create(&idleThread, NULL, idleFunc, (void*)pthread_self());


//        if (strcmp(buf, "INT\n") == 0) {
//            FILE *file = fopen("tmp/buf.txt","w");
//
//            pthread_mutex_lock(&mutex);
//            for (int index = 0; index < serverBufIndex; index++) {
//                fprintf(file, "%s", serverBuf[index]);
//                free(serverBuf[index]);
//            }
//            serverBufIndex = 0;
//            free(serverBuf);
//            pthread_mutex_unlock(&mutex);
//
//            fclose(file);
//
//            char *filename = (char *) malloc(sizeof(char) * 20);
//            sprintf(filename, "tmp/buf.txt\n");
//            printf("%s", filename);
//            free(filename);
//        }
    }
}