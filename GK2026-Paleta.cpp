// funkcje do redukcji kolorow i tworzenia palet (zmienna liczba bitow 3..7)
#include "GK2026-Paleta.h"
#include "GK2026-Zmienne.h"
#include "GK2026-MedianCut.h"

#include <stdlib.h>
#include <math.h>

// klasyczny wzorzec Bayera 4x4 - wartosci 0..15
const int bayer4x4[4][4] = {
    {  0,  8,  2, 10 },
    { 12,  4, 14,  6 },
    {  3, 11,  1,  9 },
    { 15,  7, 13,  5 }
};

Uint8 luminancja(Uint8 r, Uint8 g, Uint8 b) {
    // klasyczne wagi BT.601
    int y = (299 * r + 587 * g + 114 * b) / 1000;
    if (y < 0)   y = 0;
    if (y > 255) y = 255;
    return (Uint8)y;
}

int liczbaKolorow(int bpp) {
    if (bpp < GK26_BPP_MIN) bpp = GK26_BPP_MIN;
    if (bpp > GK26_BPP_MAX) bpp = GK26_BPP_MAX;
    return 1 << bpp;
}

// podzial bitow R-G-B w zaleznosci od liczby bitow (zgodnie z PDF1 zad.1)
void podzialBitow(int bpp, int* bitR, int* bitG, int* bitB) {
    switch (bpp) {
        case 7: *bitR = 3; *bitG = 2; *bitB = 2; break; // 3-2-2 ->128
        case 6: *bitR = 2; *bitG = 2; *bitB = 2; break; // 2-2-2 -> 64
        case 5: *bitR = 2; *bitG = 2; *bitB = 1; break; // 2-2-1 -> 32
        case 4: *bitR = 2; *bitG = 1; *bitB = 1; break; // 2-1-1 -> 16
        case 3: *bitR = 1; *bitG = 1; *bitB = 1; break; // 1-1-1 ->  8
        default:
            // bezpieczny fallback: rozdzielamy mozliwie rowno, B najmniej
            if (bpp < 3) bpp = 3;
            if (bpp > 7) bpp = 7;
            podzialBitow(bpp, bitR, bitG, bitB);
            break;
    }
}

// rozkłada [0..(2^bity)-1] poziom na pelny zakres 0..255
static Uint8 poziomNaWartosc(int poziom, int bity) {
    int maxPoz = (1 << bity) - 1;
    if (maxPoz <= 0) return 0;             // 1 poziom -> tylko 0? (nie wystepuje, min 1 bit)
    return (Uint8)((poziom * 255 + maxPoz / 2) / maxPoz);
}

// ---- paleta kolorowa narzucona: rownomierne poziomy wg podzialu bitow ----
void zbudujPaleteKolorNarzucona(int bpp) {
    int bR, bG, bB;
    podzialBitow(bpp, &bR, &bG, &bB);
    int nR = 1 << bR, nG = 1 << bG, nB = 1 << bB;
    int i = 0;
    for (int r = 0; r < nR; r++) {
        for (int g = 0; g < nG; g++) {
            for (int b = 0; b < nB; b++) {
                paletaKolorNarzucona[i].r = poziomNaWartosc(r, bR);
                paletaKolorNarzucona[i].g = poziomNaWartosc(g, bG);
                paletaKolorNarzucona[i].b = poziomNaWartosc(b, bB);
                paletaKolorNarzucona[i].a = 255;
                i++;
            }
        }
    }
}

// ---- paleta szarosci narzucona: liniowo (1<<bpp) odcieni ----
void zbudujPaleteSzaryNarzucona(int bpp) {
    int n = liczbaKolorow(bpp);
    for (int i = 0; i < n; i++) {
        paletaSzaryNarzucona[i] = (Uint8)((i * 255 + (n - 1) / 2) / (n - 1));
    }
}

void zbudujPaleteKolorDedykowana(const SDL_Color* piksele, int liczba, int bpp) {
    medianCutKolor(piksele, liczba, paletaKolorDedykowana, liczbaKolorow(bpp));
}

void zbudujPaleteSzaryDedykowana(const SDL_Color* piksele, int liczba, int bpp) {
    medianCutSzary(piksele, liczba, paletaSzaryDedykowana, liczbaKolorow(bpp));
}

static int kwadrat(int x) { return x * x; }

int najblizszyKolorNarzucony(Uint8 r, Uint8 g, Uint8 b, int bpp) {
    int n = liczbaKolorow(bpp);
    int najIdx = 0;
    int najD   = 1 << 30;
    for (int i = 0; i < n; i++) {
        int d = kwadrat((int)paletaKolorNarzucona[i].r - r)
              + kwadrat((int)paletaKolorNarzucona[i].g - g)
              + kwadrat((int)paletaKolorNarzucona[i].b - b);
        if (d < najD) { najD = d; najIdx = i; }
    }
    return najIdx;
}

int najblizszyKolorDedykowany(Uint8 r, Uint8 g, Uint8 b, int bpp) {
    int n = liczbaKolorow(bpp);
    int najIdx = 0;
    int najD   = 1 << 30;
    for (int i = 0; i < n; i++) {
        int d = kwadrat((int)paletaKolorDedykowana[i].r - r)
              + kwadrat((int)paletaKolorDedykowana[i].g - g)
              + kwadrat((int)paletaKolorDedykowana[i].b - b);
        if (d < najD) { najD = d; najIdx = i; }
    }
    return najIdx;
}

int najblizszySzaryNarzucony(Uint8 y, int bpp) {
    int n = liczbaKolorow(bpp);
    // odwzorowanie liniowe na indeks 0..n-1 (palety zbudowanej liniowo)
    int idx = ((int)y * (n - 1) + 127) / 255;
    if (idx < 0)     idx = 0;
    if (idx > n - 1) idx = n - 1;
    return idx;
}

int najblizszySzaryDedykowany(Uint8 y, int bpp) {
    int n = liczbaKolorow(bpp);
    int najIdx = 0;
    int najD   = 1 << 30;
    for (int i = 0; i < n; i++) {
        int d = abs((int)paletaSzaryDedykowana[i] - (int)y);
        if (d < najD) { najD = d; najIdx = i; }
    }
    return najIdx;
}