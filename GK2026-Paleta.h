// funkcje do redukcji kolorow i tworzenia palet (zmienna liczba bitow 3..7)
#ifndef GK2026_PALETA_H_INCLUDED
#define GK2026_PALETA_H_INCLUDED

#include <SDL2/SDL.h>

// ---- podzial bitow na skladowe R-G-B w zaleznosci od liczby bitow ----
// 7-bit -> 3-2-2 (128 kolorow)
// 6-bit -> 2-2-2 ( 64 kolory)
// 5-bit -> 2-2-1 ( 32 kolory)
// 4-bit -> 2-1-1 ( 16 kolorow)
// 3-bit -> 1-1-1 (  8 kolorow)
// Funkcja zwraca liczbe bitow na kazda ze skladowych dla danego bpp.
void podzialBitow(int bpp, int* bitR, int* bitG, int* bitB);

// liczba kolorow palety dla danej liczby bitow = 1<<bpp
int liczbaKolorow(int bpp);

// ---- budowa palet narzuconych dla zadanej liczby bitow ----
// kolorowa: rownomierne poziomy na kazdej skladowej wg podzialu bitow
void zbudujPaleteKolorNarzucona(int bpp);
// szarosci: liniowo (1<<bpp) odcieni
void zbudujPaleteSzaryNarzucona(int bpp);

// ---- budowa palet dedykowanych (median-cut) dla zadanego bufora ----
void zbudujPaleteKolorDedykowana(const SDL_Color* piksele, int liczba, int bpp);
void zbudujPaleteSzaryDedykowana(const SDL_Color* piksele, int liczba, int bpp);

// ---- wyszukiwanie najblizszego koloru / szarosci -> indeks 0..(1<<bpp)-1 ----
int najblizszyKolorNarzucony(Uint8 r, Uint8 g, Uint8 b, int bpp);
int najblizszyKolorDedykowany(Uint8 r, Uint8 g, Uint8 b, int bpp);
int najblizszySzaryNarzucony(Uint8 y, int bpp);
int najblizszySzaryDedykowany(Uint8 y, int bpp);

// pomocnicze
Uint8 luminancja(Uint8 r, Uint8 g, Uint8 b);

// macierz Bayera 4x4 dla ditheringu uporzadkowanego (wartosci 0..15)
extern const int bayer4x4[4][4];

#endif // GK2026_PALETA_H_INCLUDED