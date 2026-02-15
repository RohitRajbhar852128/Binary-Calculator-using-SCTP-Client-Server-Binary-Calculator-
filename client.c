#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/sctp.h>

#define SERVER_IP "127.0.0.1"
#define PORT 5000
#define BUFFER 1024

int main() {
    int sock;
    struct sockaddr_in servaddr;
    char sendbuf[BUFFER], recvbuf[BUFFER];

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (sock < 0) {
        perror("Socket error");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Connection error");
        exit(1);
    }

    printf("Connected to SCTP Calculator Server!\n");
    printf("Commands:\n");
    printf("Unary:  INC 1010\n");
    printf("        DEC 1010\n");
    printf("        ONES 1010\n");
    printf("        TWOS 1010\n");
    printf("Arithmetic: ADD 1010 0101\n");
    printf("           SUB 1010 0011\n");
    printf("           MUL 101 11\n");
    printf("           DIV 1010 10\n");
    printf("Type EXIT to quit.\n\n");

    while (1) {
        printf("\nEnter command: ");
        fgets(sendbuf, sizeof(sendbuf), stdin);
        sendbuf[strcspn(sendbuf, "\n")] = 0;

        sctp_sendmsg(sock, sendbuf, strlen(sendbuf), NULL, 0, 0, 0, 0, 0, 0);

        if (strcmp(sendbuf, "EXIT") == 0) break;

        memset(recvbuf, 0, sizeof(recvbuf));
        sctp_recvmsg(sock, recvbuf, sizeof(recvbuf), NULL, 0, 0, 0);

        printf("Result: %s\n", recvbuf);
    }

    close(sock);
    return 0;
}