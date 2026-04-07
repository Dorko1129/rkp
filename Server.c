#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFSIZE 1024

int main() {
    int s;
    char on = 1;
    char buffer[BUFSIZE];
    unsigned int client_size;
    struct sockaddr_in server, client;

    // Véletlenszám generátor inicializálása és titkos szám (1-100)
    srand(time(NULL));
    int titok = rand() % 100 + 1;
    printf("Szerver elindult. A titkos szam: %d\n", titok);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(2222);
    client_size = sizeof client;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);

    if (bind(s, (struct sockaddr *)&server, sizeof server) < 0) {
        perror("Bind hiba");
        return 1;
    }

    while (1) {
        printf("\nVarakozas tippre...\n");
        recvfrom(s, buffer, BUFSIZE, 0, (struct sockaddr *)&client, &client_size);
        
        int tipp = atoi(buffer);
        printf("Kliens tippje: %d\n", tipp);

        if (tipp < titok) {
            strcpy(buffer, "Nagyobbat kene!");
        } else if (tipp > titok) {
            strcpy(buffer, "Kisebbet kene!");
        } else {
            strcpy(buffer, "Eltalaltad!");
            // Új játék indítása: új számot generálunk
            titok = rand() % 100 + 1;
            printf("\nGratulalok! Uj titkos szam: %d\n", titok);
        }

        sendto(s, buffer, strlen(buffer) + 1, 0, (struct sockaddr *)&client, client_size);
    }

    close(s);
    return 0;
}
