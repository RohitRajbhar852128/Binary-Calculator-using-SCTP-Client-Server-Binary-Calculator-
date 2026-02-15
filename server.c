// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/sctp.h>

#define PORT 5000
#define BUFFER 1024

// Convert binary string to integer
int bin_to_int(char *bin) {
    return strtol(bin, NULL, 2);
}

// Convert integer to binary string
void int_to_bin(int num, char *bin) {
    int i;
    for (i = 31; i >= 0; i--) {
        bin[31 - i] = ((num >> i) & 1) + '0';
    }
    bin[32] = '\0';
}

// Unary operations
void unary_operation(char *op, char *bin, char *result) {
    int n = bin_to_int(bin);

    if (strcmp(op, "INC") == 0) n = n + 1;
    else if (strcmp(op, "DEC") == 0) n = n - 1;
    else if (strcmp(op, "ONES") == 0) {
        for (int i = 0; bin[i]; i++)
            result[i] = (bin[i] == '0') ? '1' : '0';
        result[strlen(bin)] = '\0';
        return;
    }
    else if (strcmp(op, "TWOS") == 0) {
        int i;
        int len = strlen(bin);
        strcpy(result, bin);

        // 1's complement
        for (i = 0; i < len; i++)
            result[i] = (bin[i] == '0') ? '1' : '0';

        // add 1
        for (i = len - 1; i >= 0; i--) {
            if (result[i] == '0') {
                result[i] = '1';
                break;
            } else result[i] = '0';
        }
        return;
    }

    int_to_bin(n, result);
}

// Arithmetic operations
void binary_arithmetic(char *op, char *b1, char *b2, char *result) {
    int n1 = bin_to_int(b1);
    int n2 = bin_to_int(b2);
    int ans = 0;

    if (strcmp(op, "ADD") == 0) ans = n1 + n2;
    else if (strcmp(op, "SUB") == 0) ans = n1 - n2;
    else if (strcmp(op, "MUL") == 0) ans = n1 * n2;
    else if (strcmp(op, "DIV") == 0) {
        if (n2 == 0) {
            strcpy(result, "ERROR: DIV BY ZERO");
            return;
        }
        ans = n1 / n2;
    }

    int_to_bin(ans, result);
}

int main() {
    int sock, clientSock, len;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[BUFFER], reply[BUFFER];

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (sock < 0) {
        perror("Socket error");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind error");
        exit(1);
    }

    listen(sock, 5);
    printf("📡 SCTP Binary Calculator Server Running on port %d...\n", PORT);

    len = sizeof(cliaddr);
    clientSock = accept(sock, (struct sockaddr *)&cliaddr, (socklen_t *)&len);
    printf("Client connected!\n");

    while (1) {
        memset(buffer, 0, BUFFER);
        sctp_recvmsg(clientSock, buffer, sizeof(buffer), NULL, 0, 0, 0);

        if (strcmp(buffer, "EXIT") == 0) break;

        printf("Received: %s\n", buffer);

        char op[20], b1[200], b2[200];
        int count = sscanf(buffer, "%s %s %s", op, b1, b2);

        memset(reply, 0, sizeof(reply));

        if (count == 2) {  
            unary_operation(op, b1, reply);
        }
        else if (count == 3) { 
            binary_arithmetic(op, b1, b2, reply);
        }
        else {
            strcpy(reply, "INVALID INPUT");
        }

        sctp_sendmsg(clientSock, reply, strlen(reply), NULL, 0, 0, 0, 0, 0, 0);
    }

    close(clientSock);
    close(sock);
    return 0;
}