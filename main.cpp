#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <string>

int main(int argc, char* argv[]) {
    // Initialize SDL video and TTF
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return 1;
    }

    if (TTF_Init() < 0) {
        SDL_Quit();
        return 2;
    }

    // Set video mode
    SDL_Surface* screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
    if (!screen) {
        TTF_Quit();
        SDL_Quit();
        return 3;
    }

    // Load font
    TTF_Font* font = TTF_OpenFont("assets/RussoOne-Regular.ttf", 72);
    if (!font) {
        SDL_Quit();
        TTF_Quit();
        return 4;
    }

    SDL_Event event;
    bool running = true;
    SDL_Color color = {0xb5, 0x7e, 0xdc}; // Lavender color
    bool escape_last_released = false;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE && escape_last_released) {
                    running = false;
                } else {
                    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

                    std::string keyName = SDL_GetKeyName(event.key.keysym.sym);
                    SDL_Surface* textSurface = TTF_RenderText_Blended(font, keyName.c_str(), color);

                    if (textSurface) {
                        SDL_Surface* rotatedSurface = NULL;

#ifdef FLIP
                        // Create a new surface for the rotated text
                        rotatedSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, textSurface->w, textSurface->h,
                                                              textSurface->format->BitsPerPixel,
                                                              textSurface->format->Rmask,
                                                              textSurface->format->Gmask,
                                                              textSurface->format->Bmask,
                                                              textSurface->format->Amask);

                        if (rotatedSurface) {
                            // Rotate the text 180 degrees
                            for (int y = 0; y < textSurface->h; ++y) {
                                Uint32 *src = ((Uint32*)textSurface->pixels) + (y * textSurface->pitch / 4);
                                Uint32 *dst = ((Uint32*)rotatedSurface->pixels) + ((textSurface->h - y - 1) * rotatedSurface->pitch / 4 + textSurface->w);
                                for (int x = 0; x < textSurface->w; ++x) {
                                    *--dst = *src++;
                                }
                            }
                        }
#endif

                        SDL_Rect textLocation = { (640 - textSurface->w) / 2, (480 - textSurface->h) / 2, 0, 0 };

                        if (rotatedSurface) {
                            SDL_BlitSurface(rotatedSurface, NULL, screen, &textLocation);
                            SDL_FreeSurface(rotatedSurface);
                        } else {
                            SDL_BlitSurface(textSurface, NULL, screen, &textLocation);
                        }

                        SDL_FreeSurface(textSurface);
                    }

                    SDL_Flip(screen);
                }
            } else if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    escape_last_released = true;
                } else {
                    escape_last_released = false;
                }
            }
        }

        SDL_Delay(10);
    }

    // Clean up
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
