# Makefile dla macOS (Apple Silicon / Intel) - GK2026 Projekt
# Wymaga: SDL2 zainstalowanego przez Homebrew (`brew install sdl2`)
# Uzycie:
#   make           - kompilacja
#   make run       - kompilacja i uruchomienie
#   make clean     - czyszczenie

CXX      := clang++
CXXFLAGS := -std=c++11 -O2 -Wall -Wno-deprecated-declarations

# Wykrywanie prefixu Homebrew (Apple Silicon vs Intel)
BREW_PREFIX := $(shell brew --prefix 2>/dev/null)
ifeq ($(BREW_PREFIX),)
  BREW_PREFIX := /opt/homebrew
endif

SDL_CFLAGS := -I$(BREW_PREFIX)/include -D_THREAD_SAFE
SDL_LIBS   := -L$(BREW_PREFIX)/lib -lSDL2

SRCS := GK2026-Projekt.cpp \
        GK2026-Zmienne.cpp \
        GK2026-Funkcje.cpp \
        GK2026-Paleta.cpp \
        GK2026-MedianCut.cpp \
        GK2026-Pliki.cpp

OBJS := $(SRCS:.cpp=.o)
BIN  := GK2026-Projekt

.PHONY: all run clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(SDL_LIBS) -o $(BIN)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(OBJS) $(BIN)
