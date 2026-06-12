// funkcje do operacji na plikach (format .gk26 v2: zmienne N-bit 3..7)
#include "GK2026-Pliki.h"
#include "GK2026-Zmienne.h"
#include "GK2026-Funkcje.h"
#include "GK2026-Paleta.h"
#include "GK2026-MedianCut.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

using namespace std;

// ============================================================
//  PAKOWANIE BITOW wg PDF2 zadanie 1 (transpozycja plaszczyzn bitowych)
// ------------------------------------------------------------
//  Dla grupy 8 kolejnych pikseli A..H o wartosciach n-bitowych:
//    bajt 0 = A0 B0 C0 D0 E0 F0 G0 H0   (bit nr 0 kazdego z 8 pikseli)
//    bajt 1 = A1 B1 C1 D1 E1 F1 G1 H1   (bit nr 1 kazdego z 8 pikseli)
//    ...
//    bajt n-1 = A(n-1) ... H(n-1)
//  Czyli 8 pikseli n-bitowych zajmuje dokladnie n bajtow.
//  Piksel A trafia na najbardziej znaczacy bit (pozycja 7) kazdego bajtu.
// ============================================================

// Pakuje tablice indeksow (idx[0..n-1], wartosci 0..(2^bpp)-1) do bajtow.
static void spakujIndeksy(const Uint8* idx, int n, int bpp,
                          vector<Uint8>& wynik) {
    wynik.clear();
    for (int grupa = 0; grupa < n; grupa += 8) {
        int ile = (grupa + 8 <= n) ? 8 : (n - grupa); // pikseli w tej grupie
        // dla kazdej plaszczyzny bitowej (bit b) tworzymy jeden bajt
        for (int b = 0; b < bpp; b++) {
            Uint8 bajt = 0;
            for (int p = 0; p < 8; p++) {
                int bit = 0;
                if (p < ile) {
                    bit = (idx[grupa + p] >> b) & 1;
                }
                // piksel p o pozycji p liczonej od MSB (A -> bit 7)
                bajt |= (Uint8)(bit << (7 - p));
            }
            wynik.push_back(bajt);
        }
    }
}

// Rozpakowuje bajty z powrotem do tablicy indeksow (n pikseli, bpp bitow).
static void rozpakujIndeksy(const Uint8* bajty, int rozmiarBajtow,
                            int n, int bpp, Uint8* idx) {
    int poz = 0; // licznik bajtow
    for (int grupa = 0; grupa < n; grupa += 8) {
        int ile = (grupa + 8 <= n) ? 8 : (n - grupa);
        // zerujemy wartosci grupy
        for (int p = 0; p < ile; p++) idx[grupa + p] = 0;
        for (int b = 0; b < bpp; b++) {
            Uint8 bajt = (poz < rozmiarBajtow) ? bajty[poz] : 0;
            poz++;
            for (int p = 0; p < ile; p++) {
                int bit = (bajt >> (7 - p)) & 1;
                idx[grupa + p] |= (Uint8)(bit << b);
            }
        }
    }
}

// ============================================================
//  KOLEJNOSC ZBIERANIA DANYCH (PDF2 zadanie 2)
// ------------------------------------------------------------
//  STANDARD : wierszami, lewo->prawo, gora->dol  (p = y*w + x)
//  BLOKOWA  : bloki 8 pikseli w wierszu; przechodzimy kolejne wiersze
//             w obrebie 8-pikselowej kolumny, potem nastepna kolumna.
//  Zwraca liniowy indeks piksela (y*w+x) dla k-tej zbieranej wartosci.
// ============================================================
static int indeksWgKolejnosci(int k, int w, int h, int kolejnosc) {
    if (kolejnosc == KOLEJNOSC_STANDARD) {
        return k; // identycznosc: kolejnosc == liniowy uklad
    }
    // BLOKOWA:
    // - kazdy blok to 8 kolejnych pikseli w wierszu (chyba ze obciety na brzegu)
    // - bloki w obrebie jednej "kolumny 8-pikselowej" ida w dol (po wierszach)
    // - po wyczerpaniu wysokosci przechodzimy do nastepnej kolumny 8-pikselowej
    int kolumnBlokow = (w + 7) / 8;        // ile 8-pikselowych kolumn
    // liczba pikseli w pelnej kolumnie 8-pikselowej zalezy od szerokosci ostatniej
    // Liczymy pozycje przez akumulacje, bo ostatnia kolumna moze byc wezsza.
    int poz = 0;
    for (int kb = 0; kb < kolumnBlokow; kb++) {
        int x0 = kb * 8;
        int szerBloku = (x0 + 8 <= w) ? 8 : (w - x0); // szerokosc tej kolumny
        int pikseliWKolumnie = szerBloku * h;
        if (k < poz + pikseliWKolumnie) {
            int lokalny = k - poz;          // indeks w obrebie kolumny 8-pikselowej
            int y = lokalny / szerBloku;    // numer wiersza
            int dx = lokalny % szerBloku;   // przesuniecie w bloku
            int x = x0 + dx;
            return y * w + x;
        }
        poz += pikseliWKolumnie;
    }
    return 0; // nie powinno wystapic
}

// ============================================================
//  FILTRACJA + DITHERING -> indeksy palety
// ============================================================
static void filtrujKolor(const SDL_Color* zrodlo, int w, int h,
                         Uint8* idx, int bpp, int dithering, int dedykowana) {
    vector<int> rR(w * h), rG(w * h), rB(w * h);
    for (int i = 0; i < w * h; i++) {
        rR[i] = zrodlo[i].r;
        rG[i] = zrodlo[i].g;
        rB[i] = zrodlo[i].b;
    }

    // liczba poziomow na kazdej skladowej -> krok kwantyzacji dla Bayera
    int bR, bG, bB;
    podzialBitow(bpp, &bR, &bG, &bB);
    const int stepR = 256 / (1 << bR);
    const int stepG = 256 / (1 << bG);
    const int stepB = 256 / (1 << bB);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int p = y * w + x;
            int r = rR[p];
            int g = rG[p];
            int b = rB[p];

            if (dithering == DITHER_BAYER) {
                int M = bayer4x4[y & 3][x & 3];
                // bias z zakresu (-step/2, +step/2): (2M-15)/32 * step
                r += ((2 * M - 15) * stepR) / 32;
                g += ((2 * M - 15) * stepG) / 32;
                b += ((2 * M - 15) * stepB) / 32;
            }

            if (r < 0) r = 0;
            if (r > 255) r = 255;
            if (g < 0) g = 0;
            if (g > 255) g = 255;
            if (b < 0) b = 0;
            if (b > 255) b = 255;

            int k = dedykowana
                ? najblizszyKolorDedykowany((Uint8)r, (Uint8)g, (Uint8)b, bpp)
                : najblizszyKolorNarzucony((Uint8)r, (Uint8)g, (Uint8)b, bpp);
            idx[p] = (Uint8)k;

            if (dithering == DITHER_FS) {
                SDL_Color pal = dedykowana ? paletaKolorDedykowana[k]
                                           : paletaKolorNarzucona[k];
                int eR = r - pal.r;
                int eG = g - pal.g;
                int eB = b - pal.b;
                if (x + 1 < w) {
                    rR[p + 1] += eR * 7 / 16;
                    rG[p + 1] += eG * 7 / 16;
                    rB[p + 1] += eB * 7 / 16;
                }
                if (y + 1 < h) {
                    if (x > 0) {
                        rR[p + w - 1] += eR * 3 / 16;
                        rG[p + w - 1] += eG * 3 / 16;
                        rB[p + w - 1] += eB * 3 / 16;
                    }
                    rR[p + w] += eR * 5 / 16;
                    rG[p + w] += eG * 5 / 16;
                    rB[p + w] += eB * 5 / 16;
                    if (x + 1 < w) {
                        rR[p + w + 1] += eR * 1 / 16;
                        rG[p + w + 1] += eG * 1 / 16;
                        rB[p + w + 1] += eB * 1 / 16;
                    }
                }
            }
        }
    }
}

static void filtrujSzary(const SDL_Color* zrodlo, int w, int h,
                         Uint8* idx, int bpp, int dithering, int dedykowana) {
    vector<int> rY(w * h);
    for (int i = 0; i < w * h; i++) {
        rY[i] = luminancja(zrodlo[i].r, zrodlo[i].g, zrodlo[i].b);
    }

    int n = liczbaKolorow(bpp);
    const int stepY = 256 / n;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int p = y * w + x;
            int yy = rY[p];

            if (dithering == DITHER_BAYER) {
                int M = bayer4x4[y & 3][x & 3];
                yy += ((2 * M - 15) * stepY) / 32;
            }

            if (yy < 0) yy = 0;
            if (yy > 255) yy = 255;

            int k = dedykowana
                ? najblizszySzaryDedykowany((Uint8)yy, bpp)
                : najblizszySzaryNarzucony((Uint8)yy, bpp);
            idx[p] = (Uint8)k;

            if (dithering == DITHER_FS) {
                Uint8 pal = dedykowana ? paletaSzaryDedykowana[k]
                                       : paletaSzaryNarzucona[k];
                int e = yy - pal;
                if (x + 1 < w) rY[p + 1] += e * 7 / 16;
                if (y + 1 < h) {
                    if (x > 0)     rY[p + w - 1] += e * 3 / 16;
                    rY[p + w] += e * 5 / 16;
                    if (x + 1 < w) rY[p + w + 1] += e * 1 / 16;
                }
            }
        }
    }
}

// ============================================================
//  KONWERSJA BMP -> GK26
// ============================================================
int konwersjaBMPdoGK26(const char* nazwaWej, const char* nazwaWyj,
                       int tryb, int bpp, int dithering, int kolejnosc) {
    if (bpp < GK26_BPP_MIN) bpp = GK26_BPP_MIN;
    if (bpp > GK26_BPP_MAX) bpp = GK26_BPP_MAX;

    SDL_Surface* bmp = SDL_LoadBMP(nazwaWej);
    if (!bmp) {
        printf("Nie mozna wczytac BMP: %s\n", SDL_GetError());
        return -1;
    }

    int w = bmp->w;
    int h = bmp->h;

    vector<SDL_Color> zrodlo(w * h);
    SDL_LockSurface(bmp);
    for (int yy = 0; yy < h; yy++) {
        for (int xx = 0; xx < w; xx++) {
            Uint32 col = 0;
            char* pPos = (char*)bmp->pixels + bmp->pitch * yy
                       + bmp->format->BytesPerPixel * xx;
            memcpy(&col, pPos, bmp->format->BytesPerPixel);
            SDL_Color c;
            SDL_GetRGB(col, bmp->format, &c.r, &c.g, &c.b);
            c.a = 255;
            zrodlo[yy * w + xx] = c;
        }
    }
    SDL_UnlockSurface(bmp);
    SDL_FreeSurface(bmp);

    // kopia do globalu obrazRGB (z przycieciem do pojemnosci)
    int W = (w < szerokosc) ? w : szerokosc;
    int H = (h < wysokosc)  ? h : wysokosc;
    for (int yy = 0; yy < H; yy++)
        for (int xx = 0; xx < W; xx++)
            obrazRGB[yy * szerokosc + xx] = zrodlo[yy * w + xx];

    int kolor      = (tryb == TRYB_KOLOR_NARZUCONY || tryb == TRYB_KOLOR_DEDYKOWANY);
    int dedykowana = (tryb == TRYB_KOLOR_DEDYKOWANY || tryb == TRYB_SZARY_DEDYKOWANY);
    if (tryb == TRYB_KOLOR_NARZUCONY)  zbudujPaleteKolorNarzucona(bpp);
    if (tryb == TRYB_SZARY_NARZUCONY)  zbudujPaleteSzaryNarzucona(bpp);
    if (tryb == TRYB_KOLOR_DEDYKOWANY) zbudujPaleteKolorDedykowana(&zrodlo[0], w * h, bpp);
    if (tryb == TRYB_SZARY_DEDYKOWANY) zbudujPaleteSzaryDedykowana(&zrodlo[0], w * h, bpp);

    // filtracja -> indeksy w ukladzie liniowym (y*w+x)
    vector<Uint8> idxLiniowy(w * h);
    if (kolor) filtrujKolor(&zrodlo[0], w, h, &idxLiniowy[0], bpp, dithering, dedykowana);
    else       filtrujSzary(&zrodlo[0], w, h, &idxLiniowy[0], bpp, dithering, dedykowana);

    // przeporzadkowanie wg wybranej kolejnosci zbierania danych
    int n = w * h;
    vector<Uint8> idxWg(n);
    for (int k = 0; k < n; k++) {
        idxWg[k] = idxLiniowy[ indeksWgKolejnosci(k, w, h, kolejnosc) ];
    }

    // pakowanie N-bit
    vector<Uint8> dane;
    spakujIndeksy(&idxWg[0], n, bpp, dane);

    // zapis pliku
    FILE* f = fopen(nazwaWyj, "wb");
    if (!f) { printf("Nie mozna otworzyc pliku do zapisu: %s\n", nazwaWyj); return -1; }

    Uint8 hdr[16];
    hdr[0] = GK26_MAGIC0;
    hdr[1] = GK26_MAGIC1;
    hdr[2] = GK26_MAGIC2;
    hdr[3] = GK26_MAGIC3;
    hdr[4] = GK26_WERSJA;
    hdr[5] = (Uint8)tryb;
    hdr[6] = (Uint8)(dithering & 0xFF);
    hdr[7] = (Uint8)bpp;                 // liczba bitow na piksel (3..7)
    hdr[8]  = (Uint8)(w & 0xFF);
    hdr[9]  = (Uint8)((w >> 8) & 0xFF);
    hdr[10] = (Uint8)(h & 0xFF);
    hdr[11] = (Uint8)((h >> 8) & 0xFF);
    hdr[12] = (Uint8)(kolejnosc & 0xFF); // kolejnosc zbierania danych
    Uint32 dataSize = (Uint32)dane.size();
    hdr[13] = (Uint8)( dataSize        & 0xFF);
    hdr[14] = (Uint8)((dataSize >>  8) & 0xFF);
    hdr[15] = (Uint8)((dataSize >> 16) & 0xFF);
    fwrite(hdr, 1, 16, f);

    // paleta tylko dla trybow dedykowanych (zapisujemy faktyczna liczbe kolorow)
    int nKol = liczbaKolorow(bpp);
    if (tryb == TRYB_KOLOR_DEDYKOWANY) {
        for (int i = 0; i < nKol; i++) {
            Uint8 rgb[3] = { paletaKolorDedykowana[i].r,
                             paletaKolorDedykowana[i].g,
                             paletaKolorDedykowana[i].b };
            fwrite(rgb, 1, 3, f);
        }
    } else if (tryb == TRYB_SZARY_DEDYKOWANY) {
        fwrite(paletaSzaryDedykowana, 1, nKol, f);
    }

    if (!dane.empty())
        fwrite(&dane[0], 1, dane.size(), f);
    fclose(f);

    printf("Zapisano %s (tryb=%d bpp=%d dith=%d kolejnosc=%d) -- %d bajtow danych\n",
           nazwaWyj, tryb, bpp, dithering, kolejnosc, (int)dataSize);
    return 0;
}

// ============================================================
//  ODCZYT .gk26
// ============================================================
static int wczytajGK26(const char* nazwa, int* zwrocW, int* zwrocH,
                       int* zwrocTryb, int* zwrocBpp,
                       int* zwrocDith, int* zwrocKolejnosc) {
    FILE* f = fopen(nazwa, "rb");
    if (!f) { printf("Nie mozna otworzyc %s\n", nazwa); return -1; }

    Uint8 hdr[16];
    if (fread(hdr, 1, 16, f) != 16) { fclose(f); return -1; }
    if (hdr[0] != GK26_MAGIC0 || hdr[1] != GK26_MAGIC1 ||
        hdr[2] != GK26_MAGIC2 || hdr[3] != GK26_MAGIC3) {
        printf("To nie jest plik .gk26\n"); fclose(f); return -1;
    }

    int wer      = hdr[4];
    int tryb     = hdr[5];
    int dith     = hdr[6];
    int bpp      = hdr[7];
    int w        = hdr[8]  | (hdr[9]  << 8);
    int h        = hdr[10] | (hdr[11] << 8);
    int kolejnosc = hdr[12];
    Uint32 dataSize = (Uint32)hdr[13]
                    | ((Uint32)hdr[14] << 8)
                    | ((Uint32)hdr[15] << 16);

    if (wer != GK26_WERSJA || bpp < GK26_BPP_MIN || bpp > GK26_BPP_MAX) {
        printf("Niewspierana wersja/bpp pliku (wer=%d bpp=%d)\n", wer, bpp);
        fclose(f); return -1;
    }

    int nKol = liczbaKolorow(bpp);

    // odtworzenie palet
    if (tryb == TRYB_KOLOR_NARZUCONY) zbudujPaleteKolorNarzucona(bpp);
    if (tryb == TRYB_SZARY_NARZUCONY) zbudujPaleteSzaryNarzucona(bpp);
    if (tryb == TRYB_KOLOR_DEDYKOWANY) {
        for (int i = 0; i < nKol; i++) {
            Uint8 rgb[3];
            if (fread(rgb, 1, 3, f) != 3) { fclose(f); return -1; }
            paletaKolorDedykowana[i].r = rgb[0];
            paletaKolorDedykowana[i].g = rgb[1];
            paletaKolorDedykowana[i].b = rgb[2];
            paletaKolorDedykowana[i].a = 255;
        }
    } else if (tryb == TRYB_SZARY_DEDYKOWANY) {
        if ((int)fread(paletaSzaryDedykowana, 1, nKol, f) != nKol) { fclose(f); return -1; }
    }

    vector<Uint8> dane(dataSize);
    if (dataSize > 0 && fread(&dane[0], 1, dataSize, f) != dataSize) {
        fclose(f); return -1;
    }
    fclose(f);

    int n = w * h;

    // rozpakowanie -> indeksy w kolejnosci zbierania danych
    vector<Uint8> idxWg(n);
    rozpakujIndeksy(dane.empty() ? NULL : &dane[0], (int)dataSize, n, bpp, &idxWg[0]);

    // odwzorowanie kolejnosci na uklad liniowy (y*w+x)
    int maxN = szerokosc * wysokosc;
    for (int k = 0; k < n; k++) {
        int liniowy = indeksWgKolejnosci(k, w, h, kolejnosc);
        if (liniowy >= maxN) continue;
        Uint8 idx = idxWg[k];
        if (idx >= nKol) idx = nKol - 1;
        if (tryb == TRYB_KOLOR_NARZUCONY) {
            obrazRGB[liniowy] = paletaKolorNarzucona[idx];
        } else if (tryb == TRYB_KOLOR_DEDYKOWANY) {
            obrazRGB[liniowy] = paletaKolorDedykowana[idx];
        } else if (tryb == TRYB_SZARY_NARZUCONY) {
            Uint8 v = paletaSzaryNarzucona[idx];
            obrazRGB[liniowy].r = v; obrazRGB[liniowy].g = v;
            obrazRGB[liniowy].b = v; obrazRGB[liniowy].a = 255;
        } else if (tryb == TRYB_SZARY_DEDYKOWANY) {
            Uint8 v = paletaSzaryDedykowana[idx];
            obrazRGB[liniowy].r = v; obrazRGB[liniowy].g = v;
            obrazRGB[liniowy].b = v; obrazRGB[liniowy].a = 255;
        }
    }

    *zwrocW = w; *zwrocH = h;
    *zwrocTryb = tryb; *zwrocBpp = bpp;
    *zwrocDith = dith; *zwrocKolejnosc = kolejnosc;
    return 0;
}

int ladujGK26(const char* nazwa) {
    int w, h, tryb, bpp, dith, kol;
    if (wczytajGK26(nazwa, &w, &h, &tryb, &bpp, &dith, &kol) != 0) return -1;

    czyscEkran(0, 0, 0);
    int W = (w < szerokosc) ? w : szerokosc;
    int H = (h < wysokosc)  ? h : wysokosc;
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            SDL_Color c = obrazRGB[y * w + x];
            setPixel(x, y, c.r, c.g, c.b);
        }
    }
    SDL_UpdateWindowSurface(window);
    printf("Wczytano %s: %dx%d tryb=%d bpp=%d dith=%d kolejnosc=%d\n",
           nazwa, w, h, tryb, bpp, dith, kol);
    return 0;
}

int konwersjaGK26doBMP(const char* nazwaWej, const char* nazwaWyj) {
    int w, h, tryb, bpp, dith, kol;
    if (wczytajGK26(nazwaWej, &w, &h, &tryb, &bpp, &dith, &kol) != 0) return -1;

    SDL_Surface* out = SDL_CreateRGBSurfaceWithFormat(
        0, w, h, 24, SDL_PIXELFORMAT_RGB24);
    if (!out) {
        printf("SDL_CreateRGBSurface: %s\n", SDL_GetError());
        return -1;
    }
    SDL_LockSurface(out);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            SDL_Color c = obrazRGB[y * w + x];
            Uint8* p = (Uint8*)out->pixels + y * out->pitch + x * 3;
            p[0] = c.r; p[1] = c.g; p[2] = c.b;
        }
    }
    SDL_UnlockSurface(out);
    int rc = SDL_SaveBMP(out, nazwaWyj);
    SDL_FreeSurface(out);
    if (rc != 0) { printf("SDL_SaveBMP: %s\n", SDL_GetError()); return -1; }
    printf("Zapisano BMP: %s (%dx%d)\n", nazwaWyj, w, h);
    return 0;
}