#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <time.h>
#include <string.h>

#define BUFSIZE 1024
#define PORT 5223
#define MESSAGE_SIZE 50

struct sockaddr_in addr;
int sock;

void sleepFun(int num) {
    for (int i = 0; i < num; i++) {
        printf(".\n");
        sleep(1);
    }
}
void setSocket(char *ip) {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(PORT);
}
void setConnection(int sock) {
    int num = 2 + rand() % 10;
    sleepFun(num);
    connect(sock, (struct sockaddr *)&addr, sizeof(addr));
}
void sendMessage(int sock, char* message) {
    int num = 2 + rand() % 10;
    sleepFun(num);
    send(sock, message, strlen(message), 0);
}
void getMessage(int sock) {
    char buf[BUFSIZE];
    int byteRead = (int) recv(sock, buf, BUFSIZE, 0);
    buf[byteRead] = '\0';
    printf("get message: %s\n", buf);
}

int main () {
    srand(time(NULL));

    char *ip = (char*) malloc(sizeof(char) * MESSAGE_SIZE);
    fgets(ip, MESSAGE_SIZE, stdin);
    setSocket(ip);
    free(ip);

    setConnection(sock);

    char* message = (char*) malloc(sizeof(char) * BUFSIZE);
    fgets(message, BUFSIZE, stdin);
    free(message);

    sendMessage(sock, message);
    getMessage(sock);

    close(sock);
    return 0;
}