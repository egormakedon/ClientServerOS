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
#define BUFSIZE 2048
#define MESSAGE_SIZE 100

char** serverBuf;
int serverBufIndex = 0;
pthread_mutex_t mutex;
int serverSocket;

struct client {
    int clientSocket;
    struct sockaddr_in clientAddr;
    pthread_t idleThread;
};

void* idleFunc(void* thread) {
    pthread_t pthread_self = *(pthread_t*) thread;
    while(TRUE) {
        pthread_mutex_lock(&mutex);
        sleep(1);
        serverBuf[serverBufIndex] = (char*) malloc(sizeof(char) * MESSAGE_SIZE);
        sprintf(serverBuf[serverBufIndex], "[%lu]: idle\n", pthread_self);
        serverBufIndex++;
        pthread_mutex_unlock(&mutex);
    }
}

void* clientFunc(void* ptr) {
    struct client* clientPtr = (struct client*) ptr;

    pthread_mutex_lock(&mutex);
    serverBuf[serverBufIndex] = (char*) malloc(sizeof(char) * MESSAGE_SIZE);
    sprintf(serverBuf[serverBufIndex], "[%lu]: accept new client %s\n", pthread_self(), inet_ntoa(clientPtr->clientAddr.sin_addr));
    serverBufIndex++;
    pthread_mutex_unlock(&mutex);

    char buf[BUFSIZE];
    int byteRead = (int) recv(clientPtr->clientSocket, buf, BUFSIZE, 0);
    buf[byteRead] = '\0';

    pthread_mutex_lock(&mutex);
    serverBuf[serverBufIndex] = (char*) malloc(sizeof(char) * strlen(buf));
    strcat(serverBuf[serverBufIndex], buf);
    serverBufIndex++;
    pthread_mutex_unlock(&mutex);

    send(clientPtr->clientSocket, buf, strlen(buf), 0);

    pthread_mutex_lock(&mutex);
    serverBuf[serverBufIndex] = (char*) malloc(sizeof(char) * MESSAGE_SIZE);
    sprintf(serverBuf[serverBufIndex], "[%lu]: client %s disconnected\n", pthread_self(), inet_ntoa(clientPtr->clientAddr.sin_addr));
    serverBufIndex++;
    pthread_mutex_unlock(&mutex);

    pthread_cancel(clientPtr->idleThread);
    close(clientPtr->clientSocket);
}

void initServerSocket() {
    struct sockaddr_in addr;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr));
    listen(serverSocket, 1);
}

void signal_handler() {
    FILE* file = fopen("tmp/buf.txt", "w");
    for (int index = 0; index < serverBufIndex; index++) {
        fprintf(file, "%s", serverBuf[index]);
        free(serverBuf[index]);
    }
    free(serverBuf);
    fclose(file);
    close(serverSocket);
    exit(0);
}

int main() {
    struct sockaddr_in clientAddr;
    int clientLen = sizeof(struct sockaddr_in);

    initServerSocket();
    serverBuf = (char**) malloc(sizeof(char*) * BUFSIZE);
    pthread_mutex_init(&mutex, NULL);
    signal(SIGINT, signal_handler);

    while (TRUE) {
        int clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, (socklen_t*)&clientLen);

        pthread_t clientPthread;
        pthread_t idleThread;

        struct client clientPtr;
        clientPtr.clientSocket = clientSocket;
        clientPtr.clientAddr = clientAddr;
        clientPtr.idleThread = idleThread;

        pthread_create(&clientPthread, NULL, clientFunc, &clientPtr);
        pthread_create(&idleThread, NULL, idleFunc, &clientPthread);
    }

    return 0;
}