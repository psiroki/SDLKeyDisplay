#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include <sstream>
#include <iostream>

SDL_Color color = {0xb5, 0x7e, 0xdc}; // Lavender color

SDL_Surface* screen;
TTF_Font* font;

const int TYPE_KEY = 0 << 16;
const int TYPE_BUTTON = 1 << 16;
const int TYPE_HAT = 2 << 16;

void displayString(const char *text, float progress) {
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

  Uint16 width = progress * screen->w;
  SDL_Rect progressBar = {
    .x = static_cast<Sint16>(screen->w - width >> 1),
#ifdef FLIP
    .y = 0,
#else
    .y = static_cast<Sint16>(screen->h - 32),
#endif
    .w = width,
    .h = 32,
  };
  SDL_FillRect(screen, &progressBar, SDL_MapRGB(screen->format, color.r, color.g, color.b));

  SDL_Surface* textSurface = TTF_RenderText_Blended(font, text, color);

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

    SDL_Rect textLocation = {
      .x = static_cast<Sint16>((640 - textSurface->w) / 2),
      .y = static_cast<Sint16>((480 - textSurface->h) / 2),
      .w = 0,
      .h = 0,
    };

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

int main(int argc, char* argv[]) {
  // Initialize SDL video and TTF
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
    perror("Cannot initialize SDL");
    return 1;
  }
  SDL_JoystickEventState(SDL_ENABLE);
  SDL_Joystick *joy = SDL_JoystickOpen(0);

  if (TTF_Init() < 0) {
    perror("Can't initialize SDL_TTF");
    SDL_Quit();
    return 2;
  }

  // Set video mode
#ifdef PORTRAIT
  screen = SDL_SetVideoMode(480, 640, 32, SDL_SWSURFACE);
#else
  screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
#endif
  if (!screen) {
    perror("Can't set video mode");
    TTF_Quit();
    SDL_Quit();
    return 3;
  }

  // Load font
  font = TTF_OpenFont("assets/RussoOne-Regular.ttf", 72);
  if (!font) {
    perror("Can't load font");
    SDL_Quit();
    TTF_Quit();
    return 4;
  }

  SDL_Event event;
  bool running = true;
  bool joyButtons[256];
  bool keys[static_cast<int>(SDLK_LAST)];
  int lastHat = 0;
  memset(joyButtons, 0, sizeof(joyButtons));
  memset(keys, 0, sizeof(keys));

  int lastDown = 0, downStride = 0;

  while (running && SDL_WaitEvent(&event)) {
    std::string textToDisplay;
    int downCode = 0;
    if (event.type == SDL_QUIT) {
      running = false;
    } else if (event.type == SDL_JOYHATMOTION) {
      SDL_JoyHatEvent &hat(event.jhat);
      if (hat.value) {
        const char *upDown = hat.value & SDL_HAT_UP ? "up" : hat.value & SDL_HAT_DOWN ? "down" : nullptr;
        const char *leftRight = hat.value & SDL_HAT_LEFT ? "left" : hat.value & SDL_HAT_RIGHT ? "right" : nullptr;
        std::stringstream hatName;
        hatName << "Hat ";
        if (!upDown && !leftRight) {
          hatName << "centered";
        } else {
          if (upDown) {
            hatName << upDown;
            if (leftRight) hatName << " ";
          }
          if (leftRight) hatName << leftRight;
        }
        textToDisplay = hatName.str();
      }
      if (hat.value != lastHat) {
        lastHat = hat.value;
        if (hat.value) downCode = TYPE_HAT | hat.value;
      }
    } else if (event.type == SDL_JOYBUTTONDOWN) {
      char buttonName[256] = { 0 };
      int button = event.jbutton.button;
      snprintf(buttonName, sizeof(buttonName), "Button #%d", button);
      textToDisplay = buttonName;
      if (!joyButtons[button]) {
        joyButtons[button] = true;
        downCode = TYPE_BUTTON | button;
      }
    } else if (event.type == SDL_JOYBUTTONUP) {
      int button = event.jbutton.button;
      if (joyButtons[button]) {
        joyButtons[button] = false;
      }
    } else if (event.type == SDL_KEYDOWN) {
      int key = static_cast<int>(event.key.keysym.sym);
      if (!keys[key]) {
        keys[key] = true;
        downCode = TYPE_KEY | key;
      }
      const char *keyName = SDL_GetKeyName(event.key.keysym.sym);
      textToDisplay = keyName;
    } else if (event.type == SDL_KEYUP) {
      int key = static_cast<int>(event.key.keysym.sym);
      if (keys[key]) {
        keys[key] = false;
      }
    }
    if (downCode) {
      if (lastDown == downCode) {
        ++downStride;
      } else {
        downStride = 1;
      }
      lastDown = downCode;
    }
    if (downStride == 3) running = false;
    if (textToDisplay.length()) displayString(textToDisplay.c_str(), (downStride - 1) / 2.0f);
  }

  // Clean up
  TTF_CloseFont(font);
  TTF_Quit();
  SDL_Quit();

  return 0;
}
