#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <time.h>
#include <string.h>

#define BUFSIZE 32
#define PORT 5223

struct sockaddr_in addr;

void sleepFun(int num) {
    for (int i = 0; i < num; i++) {
        printf(".\n");
        sleep(1);
    }
}

int setSocket(char *ip) {
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(PORT);

    return sock;
}

void setConnection(int sock) {
    connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    printf("connection\n");
    int num = 1 + rand() % 8;
    sleepFun(num);
}

void sendMessage(int sock, char* message) {
    send(sock, message, strlen(message), 0);
    printf("send message\n");
    int num = 1 + rand() % 8;
    sleepFun(num);
}

void getMessage(int sock) {
    char buf[sizeof(BUFSIZE)];
    int byteRead = recv(sock, buf, BUFSIZE - 1, 0);
    buf[byteRead] = '\0';
    printf("get message: %s\n", buf);
}

int main () {
    srand(time(NULL));

    char *ip = (char*) malloc(sizeof(char) * BUFSIZE);
    fgets(ip, BUFSIZE, stdin);
    int sock = setSocket(ip);
    setConnection(sock);

    char *message = (char*) malloc(sizeof(char) * BUFSIZE);
    fgets(message, BUFSIZE, stdin);
    sendMessage(sock, message);

    getMessage(sock);

    close(sock);
    return 0;
}