#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Segédfüggvény egy fájl méretének lekérdezésére
long get_file_size(FILE *file) {
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

// ================= KÓDOLÁS (ENCODE) =================
int encode(const char *bmp_filename, const char *secret_filename) {
    FILE *f_bmp = fopen(bmp_filename, "rb");
    FILE *f_sec = fopen(secret_filename, "rb");
    
    if (!f_bmp || !f_sec) {
        fprintf(stderr, "Hiba: Nem sikerult megnyitni a bemeneti fajlokat!\n");
        if(f_bmp) fclose(f_bmp);
        if(f_sec) fclose(f_sec);
        return 1;
    }

    long bmp_size = get_file_size(f_bmp);
    long sec_size = get_file_size(f_sec);

    // Memóriafoglalás és betöltés
    unsigned char *bmp_data = (unsigned char *)malloc(bmp_size);
    unsigned char *sec_data = (unsigned char *)malloc(sec_size);
    
    fread(bmp_data, 1, bmp_size, f_bmp);
    fread(sec_data, 1, sec_size, f_sec);
    fclose(f_bmp);
    fclose(f_sec);

    // Ellenőrzések
    if (bmp_data[0] != 'B' || bmp_data[1] != 'M') {
        fprintf(stderr, "Hiba: A fajl nem BMP!\n");
        free(bmp_data); free(sec_data); return 1;
    }
    
    uint16_t bpp = bmp_data[28] | (bmp_data[29] << 8); // 28. offset = 29. bájt
    if (bpp != 24) {
        fprintf(stderr, "Hiba: A BMP nem 24 bites (TrueColor)!\n");
        free(bmp_data); free(sec_data); return 1;
    }

    // Kapacitás ellenőrzés (54. offsettől kezdődik a pixel array)
    long max_secret_size = (bmp_size - 54) / 3;
    if (sec_size > max_secret_size) {
        fprintf(stderr, "Hiba: A rejtendo fajl tul nagy ehhez a kephez!\n");
        free(bmp_data); free(sec_data); return 1;
    }

    // 1. Fájlméret elrejtése a 43-46. bájtokon (offset 42-45) little-endian formátumban
    bmp_data[42] = sec_size & 0xFF;
    bmp_data[43] = (sec_size >> 8) & 0xFF;
    bmp_data[44] = (sec_size >> 16) & 0xFF;
    bmp_data[45] = (sec_size >> 24) & 0xFF;

    // 2. Kódolás logikája (55. bájttól, azaz 54-es offsettől)
    for (long i = 0; i < sec_size; i++) {
        unsigned char sec_byte = sec_data[i];
        long px_idx = 54 + (i * 3);

        // Első bájt: alsó 2 bit törlése (0xFC), rejtett felső 2 bit beírása
        bmp_data[px_idx] = (bmp_data[px_idx] & 0xFC) | ((sec_byte >> 6) & 0x03);
        
        // Második bájt: alsó 3 bit törlése (0xF8), rejtett középső 3 bit beírása
        bmp_data[px_idx + 1] = (bmp_data[px_idx + 1] & 0xF8) | ((sec_byte >> 3) & 0x07);
        
        // Harmadik bájt: alsó 3 bit törlése (0xF8), rejtett alsó 3 bit beírása
        bmp_data[px_idx + 2] = (bmp_data[px_idx + 2] & 0xF8) | (sec_byte & 0x07);
    }

    // Kódolt fájl mentése
    FILE *f_out = fopen("encoded.bmp", "wb");
    fwrite(bmp_data, 1, bmp_size, f_out);
    fclose(f_out);

    free(bmp_data);
    free(sec_data);
    printf("Sikeres kodolas! A kimenet ide lett mentve: encoded.bmp\n");
    return 0;
}

// ================= DEKÓDOLÁS (DECODE) =================
int decode(const char *stego_filename) {
    FILE *f_bmp = fopen(stego_filename, "rb");
    if (!f_bmp) {
        fprintf(stderr, "Hiba: Nem sikerult megnyitni a rejtett informaciot tartalmazo BMP-t!\n");
        return 1;
    }

    long bmp_size = get_file_size(f_bmp);
    unsigned char *bmp_data = (unsigned char *)malloc(bmp_size);
    fread(bmp_data, 1, bmp_size, f_bmp);
    fclose(f_bmp);

    // Biztonsági ellenőrzések
    if (bmp_data[0] != 'B' || bmp_data[1] != 'M') {
        fprintf(stderr, "Hiba: A megadott fajl nem BMP!\n");
        free(bmp_data); return 1;
    }

    // 1. Rejtett fájlméret kiolvasása az offset 42-ről (43. bájt)
    uint32_t sec_size = bmp_data[42] | 
                       (bmp_data[43] << 8) | 
                       (bmp_data[44] << 16) | 
                       (bmp_data[45] << 24);

    // Biztonsági limit, nehogy szemét adat miatt túlcsorduljunk
    if (sec_size == 0 || sec_size > (bmp_size - 54) / 3) {
        fprintf(stderr, "Hiba: Eltolodott méret vagy nincs elrejtve semmi ebben a kepben!\n");
        free(bmp_data); return 1;
    }

    // 2. Memóriafoglalás a visszafejtett adatoknak
    unsigned char *sec_data = (unsigned char *)malloc(sec_size);

    // 3. Dekódolás logikája (55. bájttól / 54-es offsettől)
    for (uint32_t i = 0; i < sec_size; i++) {
        long px_idx = 54 + (i * 3);

        unsigned char p1 = bmp_data[px_idx] & 0x03;     // Alsó 2 bit
        unsigned char p2 = bmp_data[px_idx + 1] & 0x07; // Alsó 3 bit
        unsigned char p3 = bmp_data[px_idx + 2] & 0x07; // Alsó 3 bit

        // Bitek összeállítása egy bájttá
        sec_data[i] = (p1 << 6) | (p2 << 3) | p3;
    }

    // Visszafejtett fájl kimentése
    FILE *f_out = fopen("decoded.bin", "wb");
    fwrite(sec_data, 1, sec_size, f_out);
    fclose(f_out);

    free(bmp_data);
    free(sec_data);
    printf("Sikeres dekodolas! A kinyert fajl ide lett mentve: decoded.bin\n");
    return 0;
}

// ================= MAIN (PARANCSSORI ARGUMENTUMOK) =================
int main(int argc, char *argv[]) {
    // Ha 2 fájlnév van (összesen 3 argumentum a program nevével együtt): KÓDOLÁS
    if (argc == 3) {
        printf("Mod: KODOLAS\n");
        return encode(argv[1], argv[2]);
    } 
    // Ha 1 fájlnév van (összesen 2 argumentum): DEKÓDOLÁS
    else if (argc == 2) {
        printf("Mod: DEKODOLAS\n");
        return decode(argv[1]);
    } 
    // Hibás használat esetén help kiírása
    else {
        printf("Hasznalat:\n");
        printf("  Kodolashoz:   %s <eredeti_kep.bmp> <rejtendo_fajl.txt>\n", argv[0]);
        printf("  Dekodolashoz: %s <kodolt_kep.bmp>\n", argv[0]);
        return 1;
    }
}
    }
   
   
   
 close(in);
 close(out);


    return 0;
}
