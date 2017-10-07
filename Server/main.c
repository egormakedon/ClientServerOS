#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <zconf.h>

#define PORT 5223
#define BUFSIZE 500
#define TRUE 1
#define FALSE !TRUE

int boolean = TRUE;
char *serverBuf;

struct threadDataStruct {
    int clientSocket;
    char *clientIp;
};

void *threadFunc(void *voidPtr) {
    struct threadDataStruct* threadData = (struct threadDataStruct*) voidPtr;

    int socket = threadData->clientSocket;
    char *ip = threadData->clientIp;
    char *string = (char*) malloc(sizeof(char) * BUFSIZE);
    sprintf(string, "[%d]: accept new client %s\n", socket, ip);

    for (int i = 5; i < 5; i++) {
        char *newString = (char*) malloc(sizeof(char) * BUFSIZE);
        sprintf(newString, "[%d]: idle\n", socket);
        strcat(string, newString);
        free(newString);
    }

    strcat(serverBuf, string);

    free(string);
    free(threadData);
    free(ip);
}

int initServerSocket(char *ip) {
    struct sockaddr_in addr;

    int serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(ip);
    bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr));
    listen(serverSocket, 5);

    return serverSocket;
}

int initClientSocket(struct sockaddr_in clientAddr, int serverSocket) {
    int clientLen = sizeof(clientAddr);
    return accept(serverSocket, (struct sockaddr *) &clientAddr, &clientLen);
}

struct threadDataStruct *initStruct(struct sockaddr_in clientAddr, int clientSocket) {
    struct threadDataStruct *threadData = (struct threadDataStruct*) malloc(sizeof(struct threadDataStruct));
    threadData->clientSocket = clientSocket;
    threadData->clientIp = inet_ntoa(clientAddr.sin_addr);
    return threadData;
}

int main(void) {
    struct sockaddr_in clientAddr;
    serverBuf = (char*) malloc(sizeof(char) * BUFSIZE);

    char *ip = (char*) malloc(sizeof(char) * BUFSIZE);
    fgets(ip, BUFSIZE, stdin);
    int serverSocket = initServerSocket(ip);
    free(ip);

    while (TRUE) {
        int clientSocket = initClientSocket(clientAddr, serverSocket);
        struct threadDataStruct *threadData = initStruct(clientAddr, clientSocket);

//        pthread_t pthread;
//        pthread_create(&pthread, NULL, threadFunc, (void*)threadData);
//        pthread_join(pthread, NULL);
//        boolean = FALSE;

        char buf[sizeof(BUFSIZE)];
        int byteRead = recv(clientSocket, buf, BUFSIZE - 1, 0);
        buf[byteRead] = '\0';

        if (strcmp(buf, "INT") == 0) {
            FILE *file = fopen("/tmp/buf.txt","r");
            fwrite(serverBuf, sizeof(serverBuf), 1, file);
            printf("buf.txt");
            free(serverBuf);
            free(threadData);
            close(clientSocket);
            serverBuf = (char*) malloc(sizeof(char) * BUFSIZE);
        } else {
            strcat(serverBuf, buf);
            send(clientSocket, buf, strlen(buf), 0);

            char *string = (char *) malloc(sizeof(char) * BUFSIZE);
            sprintf(string, "[%d]: client %s disconnected\n", clientSocket, inet_ntoa(clientAddr.sin_addr));
            strcat(serverBuf, string);

            free(string);
            free(threadData);
            close(clientSocket);
        }
    }
}