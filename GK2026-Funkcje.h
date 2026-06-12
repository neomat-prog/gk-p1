// podstawowe funkcje
#ifndef GK2026_FUNKCJE_H_INCLUDED
#define GK2026_FUNKCJE_H_INCLUDED

#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>

// --- Temat 3: alfabet 5-bitowy, 32 barwy ---
// 4 tryby x (bez ditheringu / z ditheringiem Floyd-Steinberg)
void Funkcja1();  // kolor narzucony,    bez ditheringu
void Funkcja2();  // kolor narzucony,    Floyd-Steinberg
void Funkcja3();  // szarosci narzucone, bez ditheringu
void Funkcja4();  // szarosci narzucone, Floyd-Steinberg
void Funkcja5();  // kolor dedykowany,   bez ditheringu
void Funkcja6();  // kolor dedykowany,   Floyd-Steinberg
void Funkcja7();  // szarosci dedykowane,bez ditheringu
void Funkcja8();  // szarosci dedykowane,Floyd-Steinberg
void Funkcja9();  // odczyt zapisanego .gk26 -> z powrotem do BMP (roundtrip)

// dithering uporzadkowany Bayer 4x4 (PDF zad. 3 i 4) dla palet narzuconych
void Funkcja10(); // kolor narzucony    + Bayer 4x4
void Funkcja11(); // szarosci narzucone + Bayer 4x4

// kolejnosc blokowa zbierania danych (PDF2/3 zad. 2) - przyklad zapisu/odczytu
void Funkcja12(); // kolor narzucony, bez ditheringu, KOLEJNOSC_BLOKOWA

// podglad aktualnej palety (32 barwy) w postaci wiekszych blokow
void pokazPalete(int tryb);

void setPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B);
SDL_Color getPixel(int x, int y);
void czyscEkran(Uint8 R, Uint8 G, Uint8 B);
void ladujBMP(char const* nazwa, int x, int y);

// nazwa pliku BMP ostatnio zaladowanego / przekonwertowanego
extern char ostatniBMP[256];

#endif // GK2026_FUNKCJE_H_INCLUDED