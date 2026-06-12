// zmienne globalne
#include "GK2026-Zmienne.h"

SDL_Window*  window = NULL;
SDL_Surface* screen = NULL;

SDL_Color obrazRGB[szerokosc * wysokosc];

SDL_Color paletaKolorNarzucona[GK26_MAX_KOLOROW];
Uint8     paletaSzaryNarzucona[GK26_MAX_KOLOROW];
SDL_Color paletaKolorDedykowana[GK26_MAX_KOLOROW];
Uint8     paletaSzaryDedykowana[GK26_MAX_KOLOROW];

SDL_Color paleta8[szerokosc * wysokosc];
int       ileKolorow = 0;
SDL_Color paleta8k[256];
SDL_Color paleta8s[256];