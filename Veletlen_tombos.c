#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// A lineáris futásidejű, egyenletes eloszlású Fisher-Yates keverő eljárás
void shuffle(int *tomb, int elemszam) {
    // Hátulról indulunk, és minden elemet kicserélünk egy véletlenszerűen
    // választott, előtte lévő (vagy önmaga) elemmel.
    for (int i = elemszam - 1; i > 0; i--) {
        // Véletlen index generálása 0 és i között (i-t is beleértve)
        int j = rand() % (i + 1);
        
        // tomb[i] és tomb[j] felcserélése
        int temp = tomb[i];
        tomb[i] = tomb[j];
        tomb[j] = temp;
    }
}

int main() {
    const int MERET = 10;           // A tesztelt tömb elemszáma
    const int TESZTEK_SZAMA = 100000; // Hányszor végezzük el a keverést
    
    int tomb[MERET];
    
    // Tömb az összegzések tárolására (ebből számolunk majd átlagot pozíciónként)
    long long pozicio_osszegek[MERET];
    
    // Összegező tömb nullázása
    for (int i = 0; i < MERET; i++) {
        pozicio_osszegek[i] = 0;
    }

    // A véletlenszám-generátor inicializálása a főprogramban
    srand(time(NULL));

    // Tesztelés lefuttatása
    for (int t = 0; t < TESZTEK_SZAMA; t++) {
        
        // 1. A tömb inicializálása az eredeti, rendezett állapotba (0, 1, 2...)
        for (int i = 0; i < MERET; i++) {
            tomb[i] = i;
        }

        // 2. Keverés végrehajtása
        shuffle(tomb, MERET);

        // 3. Eredmények rögzítése (az adott indexen lévő érték hozzáadása)
        for (int i = 0; i < MERET; i++) {
            pozicio_osszegek[i] += tomb[i];
        }
    }

    // Kiértékelés: 
    // Ha a számok 0-tól (MERET-1)-ig tartanak, és a keverés egyenletes,
    // akkor bármelyik pozíción bármelyik szám azonos eséllyel állhat.
    // Így minden indexen a várható átlag a számok átlaga lesz.
    double elvart_atlag = (MERET - 1) / 2.0;

    printf("====================================================\n");
    printf("Shuffle Algoritmus Teszteles\n");
    printf("Tomb merete: %d\n", MERET);
    printf("Keveresek szama: %d\n", TESZTEK_SZAMA);
    printf("Elvart atlag minden pozicion: %.3f\n", elvart_atlag);
    printf("====================================================\n\n");

    for (int i = 0; i < MERET; i++) {
        // A tesztek alapján kiszámoljuk a valós átlagot az adott indexen
        double mert_atlag = (double)pozicio_osszegek[i] / TESZTEK_SZAMA;
        
        // Kiszámítjuk az eltérést az elvárttól
        double elteres = mert_atlag - elvart_atlag;
        
        printf("Index %2d: Mert atlag = %6.3f | Elteres az elvarttol: %7.3f\n", 
               i, mert_atlag, elteres);
    }

    return 0;
}
