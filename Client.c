#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFSIZE 1024

int main(int argc, char *argv[]) {
    int s;
    unsigned int server_size;
    char on = 1;
    char buffer[BUFSIZE];
    struct sockaddr_in server;

    srand(time(NULL));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argc == 1 ? "127.0.0.1" : argv[1]);
    server.sin_port = htons(2222);
    server_size = sizeof server;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);

    int eltalalva = 0;
    while (!eltalalva) {
        // Véletlenszerű tipp generálása (1-100)
        int tipp = rand() % 100 + 1;
        sprintf(buffer, "%d", tipp);
        
        printf("Tippeles: %d\n", tipp);
        sendto(s, buffer, strlen(buffer) + 1, 0, (struct sockaddr *)&server, server_size);

        // Válasz fogadása a szervertől
        recvfrom(s, buffer, BUFSIZE, 0, (struct sockaddr *)&server, &server_size);
        printf("Szerver valasza: %s\n", buffer);

        if (strcmp(buffer, "Eltalaltad!") == 0) {
            printf("Hurra! Megtalaltam a szamot!\n");
            eltalalva = 1;
        }

        // Egy kis szünet, hogy ne fusson le túl gyorsan a terminálban
        usleep(500000); // 0.5 másodperc
    }

    close(s);
    return 0;
}
