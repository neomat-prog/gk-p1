// glowny plik projektu - format graficzny .gk26 (Temat 3: 5-bit, 32 barwy)
// Klawiatura:
//   a..l  -> wybor jednego z 9 plikow BMP jako biezace zrodlo
//   1     -> kolor narzucony,     bez ditheringu
//   2     -> kolor narzucony,     dithering Floyd-Steinberg
//   3     -> szarosci narzucone,  bez ditheringu
//   4     -> szarosci narzucone,  dithering Floyd-Steinberg
//   5     -> kolor dedykowany,    bez ditheringu
//   6     -> kolor dedykowany,    dithering Floyd-Steinberg
//   7     -> szarosci dedykowane, bez ditheringu
//   8     -> szarosci dedykowane, dithering Floyd-Steinberg
//   9     -> ostatni .gk26 -> obrazek_out.bmp i podglad (roundtrip do BMP)
//   q     -> kolor narzucony    + Bayer 4x4 (dithering uporzadkowany)
//   w     -> szarosci narzucone + Bayer 4x4 (dithering uporzadkowany)
//   e     -> kolor narzucony, bez ditheringu, kolejnosc blokowa (PDF zad.2)
//   p     -> podglad palety kolorowej narzuconej (bloki 60x60)
//   o     -> podglad palety szarosci narzuconej
//   i     -> podglad palety kolorowej dedykowanej (po ostatniej konwersji)
//   u     -> podglad palety szarosci dedykowanej (po ostatniej konwersji)
//   b     -> czyszczenie ekranu
//   ESC   -> wyjscie
#include "GK2026-Zmienne.h"
#include "GK2026-Funkcje.h"
#include "GK2026-Paleta.h"
#include "GK2026-MedianCut.h"
#include "GK2026-Pliki.h"

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    window = SDL_CreateWindow(tytul,
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              szerokosc * 2, wysokosc * 2,
                              SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    screen = SDL_GetWindowSurface(window);
    if (screen == NULL) {
        fprintf(stderr, "SDL_GetWindowSurface Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // budujemy palety narzucone 5-bit od razu - sa stale niezaleznie od obrazu
    zbudujPaleteKolorNarzucona(5);
    zbudujPaleteSzaryNarzucona(5);

    SDL_UpdateWindowSurface(window);

    bool done = false;
    SDL_Event event;
    while (SDL_WaitEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                done = true;
                break;
            case SDL_KEYDOWN: {
                if (event.key.keysym.sym == SDLK_ESCAPE) done = true;

                if (event.key.keysym.sym == SDLK_1) Funkcja1();
                if (event.key.keysym.sym == SDLK_2) Funkcja2();
                if (event.key.keysym.sym == SDLK_3) Funkcja3();
                if (event.key.keysym.sym == SDLK_4) Funkcja4();
                if (event.key.keysym.sym == SDLK_5) Funkcja5();
                if (event.key.keysym.sym == SDLK_6) Funkcja6();
                if (event.key.keysym.sym == SDLK_7) Funkcja7();
                if (event.key.keysym.sym == SDLK_8) Funkcja8();
                if (event.key.keysym.sym == SDLK_9) Funkcja9();

                if (event.key.keysym.sym == SDLK_q) Funkcja10();
                if (event.key.keysym.sym == SDLK_w) Funkcja11();
                if (event.key.keysym.sym == SDLK_e) Funkcja12();

                if (event.key.keysym.sym == SDLK_p) pokazPalete(TRYB_KOLOR_NARZUCONY);
                if (event.key.keysym.sym == SDLK_o) pokazPalete(TRYB_SZARY_NARZUCONY);
                if (event.key.keysym.sym == SDLK_i) pokazPalete(TRYB_KOLOR_DEDYKOWANY);
                if (event.key.keysym.sym == SDLK_u) pokazPalete(TRYB_SZARY_DEDYKOWANY);

                if (event.key.keysym.sym == SDLK_a) ladujBMP("obrazek1.bmp", 0, 0);
                if (event.key.keysym.sym == SDLK_s) ladujBMP("obrazek2.bmp", 0, 0);
                if (event.key.keysym.sym == SDLK_d) ladujBMP("obrazek3.bmp", 0, 0);
                if (event.key.keysym.sym == SDLK_f) ladujBMP("obrazek4.bmp", 0, 0);
                if (event.key.keysym.sym == SDLK_g) ladujBMP("obrazek5.bmp", 0, 0);
                if (event.key.keysym.sym == SDLK_h) ladujBMP("obrazek6.bmp", 0, 0);
                if (event.key.keysym.sym == SDLK_j) ladujBMP("obrazek7.bmp", 0, 0);
                if (event.key.keysym.sym == SDLK_k) ladujBMP("obrazek8.bmp", 0, 0);
                if (event.key.keysym.sym == SDLK_l) ladujBMP("obrazek9.bmp", 0, 0);

                if (event.key.keysym.sym == SDLK_b) czyscEkran(0, 0, 0);
                break;
            }
        }
        if (done) break;
    }

    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}