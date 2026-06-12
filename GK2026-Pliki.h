// funkcje do operacji na plikach (format .gk26 v2: zmienne N-bit 3..7)
#ifndef GK2026_PLIKI_H_INCLUDED
#define GK2026_PLIKI_H_INCLUDED

#include <SDL2/SDL.h>

// Konwersja BMP -> .gk26 z pelna parametryzacja.
//   nazwaWej : sciezka do pliku BMP
//   nazwaWyj : sciezka do pliku .gk26
//   tryb     : TRYB_KOLOR_NARZUCONY / TRYB_SZARY_NARZUCONY /
//              TRYB_KOLOR_DEDYKOWANY / TRYB_SZARY_DEDYKOWANY
//   bpp      : liczba bitow na piksel (3..7) -> 8..128 kolorow
//   dithering: DITHER_BRAK / DITHER_FS / DITHER_BAYER
//   kolejnosc: KOLEJNOSC_STANDARD / KOLEJNOSC_BLOKOWA (PDF2 zad.2)
// zwraca 0 przy sukcesie.
int konwersjaBMPdoGK26(const char* nazwaWej, const char* nazwaWyj,
                       int tryb, int bpp, int dithering, int kolejnosc);

// konwersja .gk26 -> BMP (z odtworzeniem palety i kolejnosci)
int konwersjaGK26doBMP(const char* nazwaWej, const char* nazwaWyj);

// wczytanie .gk26 do bufora obrazRGB i odswiezenie ekranu
int ladujGK26(const char* nazwa);

#endif // GK2026_PLIKI_H_INCLUDED