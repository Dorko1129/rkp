#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void shuffle(int *tomb, int elemszam) {
    for (int i = elemszam - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = tomb[i];
        tomb[i] = tomb[j];
        tomb[j] = temp;
    }
}

int main() {
    int elemszam = 10;
    int tomb[10];

    srand(time(NULL));

    for (int t = 0; t < 3; t++) {
        for (int i = 0; i < elemszam; i++) {
            tomb[i] = i;
        }

        shuffle(tomb, elemszam);

        for (int i = 0; i < elemszam; i++) {
            printf("%d ", tomb[i]);
        }
        printf("\n");
    }

    return 0;
}
