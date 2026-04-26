// podstawowe funkcje
#include "GK2026-Funkcje.h"
#include "GK2026-Zmienne.h"
#include "GK2026-Paleta.h"
#include "GK2026-MedianCut.h"
#include "GK2026-Pliki.h"

#include <stdio.h>
#include <string.h>

// nazwa BMP wybranego ostatnio przez uzytkownika - sluzy jako zrodlo konwersji
char ostatniBMP[256] = "obrazek1.bmp";

static void wyswietlObrazRGB(int w, int h) {
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
}

// konwertuje aktualnie wybrany BMP -> .gk26 -> wczytuje z powrotem do podgladu
static void konwertujIPokaz(int tryb, int dithering) {
    const char* nazwaWyj = "obrazek.gk26";
    if (konwersjaBMPdoGK26(ostatniBMP, nazwaWyj, tryb, dithering) != 0) return;
    if (ladujGK26(nazwaWyj) != 0) return;
}

void Funkcja1() { konwertujIPokaz(TRYB_KOLOR_NARZUCONY,  0); }
void Funkcja2() { konwertujIPokaz(TRYB_KOLOR_NARZUCONY,  1); }
void Funkcja3() { konwertujIPokaz(TRYB_SZARY_NARZUCONY,  0); }
void Funkcja4() { konwertujIPokaz(TRYB_SZARY_NARZUCONY,  1); }
void Funkcja5() { konwertujIPokaz(TRYB_KOLOR_DEDYKOWANY, 0); }
void Funkcja6() { konwertujIPokaz(TRYB_KOLOR_DEDYKOWANY, 1); }
void Funkcja7() { konwertujIPokaz(TRYB_SZARY_DEDYKOWANY, 0); }
void Funkcja8() { konwertujIPokaz(TRYB_SZARY_DEDYKOWANY, 1); }

void Funkcja9() {
    // .gk26 -> BMP roundtrip
    if (konwersjaGK26doBMP("obrazek.gk26", "obrazek_out.bmp") == 0) {
        ladujBMP("obrazek_out.bmp", 0, 0);
    }
}


void setPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B)
{
  if ((x>=0) && (x<szerokosc) && (y>=0) && (y<wysokosc))
  {
    /* Zamieniamy poszczegolne skladowe koloru na format koloru piksela */
    Uint32 pixel = SDL_MapRGB(screen->format, R, G, B);

    /* Pobieramy informacje ile bajtow zajmuje jeden piksel */
    int bpp = screen->format->BytesPerPixel;

    /* Obliczamy adres piksela */
    Uint8 *p1 = (Uint8 *)screen->pixels + (y*2) * screen->pitch + (x*2) * bpp;
    Uint8 *p2 = (Uint8 *)screen->pixels + (y*2+1) * screen->pitch + (x*2) * bpp;
    Uint8 *p3 = (Uint8 *)screen->pixels + (y*2) * screen->pitch + (x*2+1) * bpp;
    Uint8 *p4 = (Uint8 *)screen->pixels + (y*2+1) * screen->pitch + (x*2+1) * bpp;

    /* Ustawiamy wartosc piksela, w zaleznosci od formatu powierzchni*/
    switch(bpp)
    {
        case 1: //8-bit
            *p1 = pixel;
            *p2 = pixel;
            *p3 = pixel;
            *p4 = pixel;
            break;

        case 2: //16-bit
            *(Uint16 *)p1 = pixel;
            *(Uint16 *)p2 = pixel;
            *(Uint16 *)p3 = pixel;
            *(Uint16 *)p4 = pixel;
            break;

        case 3: //24-bit
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                p1[0] = (pixel >> 16) & 0xff;
                p1[1] = (pixel >> 8) & 0xff;
                p1[2] = pixel & 0xff;
                p2[0] = (pixel >> 16) & 0xff;
                p2[1] = (pixel >> 8) & 0xff;
                p2[2] = pixel & 0xff;
                p3[0] = (pixel >> 16) & 0xff;
                p3[1] = (pixel >> 8) & 0xff;
                p3[2] = pixel & 0xff;
                p4[0] = (pixel >> 16) & 0xff;
                p4[1] = (pixel >> 8) & 0xff;
                p4[2] = pixel & 0xff;
            } else {
                p1[0] = pixel & 0xff;
                p1[1] = (pixel >> 8) & 0xff;
                p1[2] = (pixel >> 16) & 0xff;
                p2[0] = pixel & 0xff;
                p2[1] = (pixel >> 8) & 0xff;
                p2[2] = (pixel >> 16) & 0xff;
                p3[0] = pixel & 0xff;
                p3[1] = (pixel >> 8) & 0xff;
                p3[2] = (pixel >> 16) & 0xff;
                p4[0] = pixel & 0xff;
                p4[1] = (pixel >> 8) & 0xff;
                p4[2] = (pixel >> 16) & 0xff;
            }
            break;

        case 4: //32-bit
            *(Uint32 *)p1 = pixel;
            *(Uint32 *)p2 = pixel;
            *(Uint32 *)p3 = pixel;
            *(Uint32 *)p4 = pixel;
            break;

        }
    }
}

void setPixelSurface(int x, int y, Uint8 R, Uint8 G, Uint8 B)
{
  if ((x>=0) && (x<szerokosc*2) && (y>=0) && (y<wysokosc*2))
  {
    Uint32 pixel = SDL_MapRGB(screen->format, R, G, B);
    int bpp = screen->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)screen->pixels + y * screen->pitch + x * bpp;

    switch(bpp)
    {
        case 1: *p = pixel; break;
        case 2: *(Uint16 *)p = pixel; break;
        case 3:
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            } else {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;
        case 4: *(Uint32 *)p = pixel; break;
        }
    }
}

SDL_Color getPixel(int x, int y) {
    SDL_Color color ;
    Uint32 col = 0 ;
    if ((x>=0) && (x<szerokosc) && (y>=0) && (y<wysokosc)) {
        char* pPosition=(char*)screen->pixels ;
        pPosition+=(screen->pitch*y*2) ;
        pPosition+=(screen->format->BytesPerPixel*x*2);
        memcpy(&col, pPosition, screen->format->BytesPerPixel);
        SDL_GetRGB(col, screen->format, &color.r, &color.g, &color.b);
    }
    return ( color ) ;
}

SDL_Color getPixelSurface(int x, int y, SDL_Surface *surface) {
    SDL_Color color ;
    Uint32 col = 0 ;
    if ((x>=0) && (x<surface->w) && (y>=0) && (y<surface->h)) {
        char* pPosition=(char*)surface->pixels ;
        pPosition+=(surface->pitch*y) ;
        pPosition+=(surface->format->BytesPerPixel*x);
        memcpy(&col, pPosition, surface->format->BytesPerPixel);
        SDL_GetRGB(col, surface->format, &color.r, &color.g, &color.b);
    }
    return ( color ) ;
}


void ladujBMP(char const* nazwa, int x, int y) {
    SDL_Surface* bmp = SDL_LoadBMP(nazwa);
    if (!bmp)
    {
        printf("Unable to load bitmap: %s\n", SDL_GetError());
        return;
    }

    // zapamietujemy nazwe pliku jako zrodlo do dalszych konwersji
    strncpy(ostatniBMP, nazwa, sizeof(ostatniBMP) - 1);
    ostatniBMP[sizeof(ostatniBMP) - 1] = 0;

    SDL_Color kolor;
    int W = (bmp->w < szerokosc) ? bmp->w : szerokosc;
    int H = (bmp->h < wysokosc)  ? bmp->h : wysokosc;
    for (int yy=0; yy<H; yy++) {
        for (int xx=0; xx<W; xx++) {
            kolor = getPixelSurface(xx, yy, bmp);
            obrazRGB[yy * szerokosc + xx] = kolor;
            setPixel(xx, yy, kolor.r, kolor.g, kolor.b);
        }
    }
    SDL_FreeSurface(bmp);
    SDL_UpdateWindowSurface(window);
}


void czyscEkran(Uint8 R, Uint8 G, Uint8 B)
{
    SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, R, G, B));
    SDL_UpdateWindowSurface(window);
}
