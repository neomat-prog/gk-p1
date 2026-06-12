// zmienne globalne
#ifndef GK2026_ZMIENNE_H_INCLUDED
#define GK2026_ZMIENNE_H_INCLUDED

#include <SDL2/SDL.h>

#define szerokosc 640
#define wysokosc  400

#define tytul "GK2026 - Projekt - Zespol XX"

// ---- stale formatu .gk26 (wersja 2: zmienna liczba bitow 3..7) ----
#define GK26_MAGIC0 'G'
#define GK26_MAGIC1 'K'
#define GK26_MAGIC2 '2'
#define GK26_MAGIC3 '6'
#define GK26_WERSJA 0x02

// dopuszczalny zakres bitow na piksel
#define GK26_BPP_MIN 3
#define GK26_BPP_MAX 7
// maksymalna liczba kolorow w palecie = 1<<7 = 128
#define GK26_MAX_KOLOROW 128

// tryby pracy (rodzaj palety)
#define TRYB_KOLOR_NARZUCONY  0x01
#define TRYB_SZARY_NARZUCONY  0x02
#define TRYB_KOLOR_DEDYKOWANY 0x03
#define TRYB_SZARY_DEDYKOWANY 0x04

// rodzaj ditheringu
#define DITHER_BRAK   0
#define DITHER_FS     1   // Floyd-Steinberg
#define DITHER_BAYER  2   // uporzadkowany Bayer 4x4

// kolejnosc zbierania danych (PDF2 zad. 2)
#define KOLEJNOSC_STANDARD 0   // wierszami, lewo->prawo, gora->dol
#define KOLEJNOSC_BLOKOWA  1   // bloki 8px, kolumnami 8-pikselowymi

extern SDL_Window*  window;
extern SDL_Surface* screen;

// bufor obrazu logicznego (640x400) trzymany niezaleznie od ekranu
extern SDL_Color obrazRGB[szerokosc * wysokosc];

// palety wykorzystywane przez format .gk26 (pojemnosc 128, uzywany prefiks)
extern SDL_Color paletaKolorNarzucona[GK26_MAX_KOLOROW];
extern Uint8     paletaSzaryNarzucona[GK26_MAX_KOLOROW];
extern SDL_Color paletaKolorDedykowana[GK26_MAX_KOLOROW];
extern Uint8     paletaSzaryDedykowana[GK26_MAX_KOLOROW];

// stara paleta 256 (na potrzeby zgodnosci z dawnym kodem)
extern SDL_Color paleta8[szerokosc * wysokosc];
extern int       ileKolorow;
extern SDL_Color paleta8k[256];
extern SDL_Color paleta8s[256];

#endif // GK2026_ZMIENNE_H_INCLUDED