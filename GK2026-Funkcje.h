// podstawowe funkcje
#ifndef GK2026_FUNKCJE_H_INCLUDED
#define GK2026_FUNKCJE_H_INCLUDED

#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>

// dziewiec demonstracyjnych slotow - prezentacja efektow konwersji
void Funkcja1(); // narzucona kolorowa, bez ditheringu
void Funkcja2(); // narzucona kolorowa, z ditheringiem
void Funkcja3(); // narzucone szarosci, bez ditheringu
void Funkcja4(); // narzucone szarosci, z ditheringiem
void Funkcja5(); // dedykowana kolorowa, bez ditheringu
void Funkcja6(); // dedykowana kolorowa, z ditheringiem
void Funkcja7(); // dedykowane szarosci, bez ditheringu
void Funkcja8(); // dedykowane szarosci, z ditheringiem
void Funkcja9(); // odczyt zapisanego .gk26 i konwersja z powrotem do BMP

void setPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B);
SDL_Color getPixel(int x, int y);
void czyscEkran(Uint8 R, Uint8 G, Uint8 B);
void ladujBMP(char const* nazwa, int x, int y);

// nazwa pliku BMP, ktory zostanie ostatnio zaladowany / przekonwertowany
extern char ostatniBMP[256];

#endif // GK2026_FUNKCJE_H_INCLUDED
