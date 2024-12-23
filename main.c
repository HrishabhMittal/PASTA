#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define GRID_SIZE 20
#define NUM_COLORS 6

SDL_Color colors[NUM_COLORS] = {
    {255, 0, 0, 255},    
    {0, 255, 0, 255},    
    {0, 0, 255, 255},    
    {255, 255, 0, 255}, 
    {0, 0, 0, 255},
    {255, 255, 255, 255},
};

SDL_Color currentColor = {0, 0, 0, 255};
SDL_Renderer *renderer;
SDL_Texture *drawingTexture;
TTF_Font *font;
SDL_Color textColor = {0, 0, 0, 255};
char rgbInput[20] = " ";
void DrawCircle(SDL_Renderer * renderer, int32_t centreX, int32_t centreY, int32_t radius) {
   const int32_t diameter = (radius * 2);
   int32_t x = (radius - 1),y = 0,tx = 1,ty = 1;
   int32_t error = (tx - diameter);
   while (x >= y) {
      SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
      SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
      SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
      SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
      SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
      SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
      SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
      SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);
      if (error <= 0) {
         ++y;
         error += ty;
         ty += 2;
      }
      if (error > 0) {
         --x;
         tx += 2;
         error += (tx - diameter);
      }
   }
}
void DrawFilledCircle(SDL_Renderer *renderer, int32_t centreX, int32_t centreY, int32_t radius) {
    const int32_t diameter = (radius * 2);
    int32_t x = (radius - 1), y = 0, tx = 1, ty = 1;
    int32_t error = (tx - diameter);

    while (x >= y) {
        // Draw horizontal lines for the filled parts
        SDL_RenderDrawLine(renderer, centreX - x, centreY - y, centreX + x, centreY - y);  // Top
        SDL_RenderDrawLine(renderer, centreX - x, centreY + y, centreX + x, centreY + y);  // Bottom
        SDL_RenderDrawLine(renderer, centreX - y, centreY - x, centreX + y, centreY - x);  // Left
        SDL_RenderDrawLine(renderer, centreX - y, centreY + x, centreX + y, centreY + x);  // Right

        if (error <= 0) {
            ++y;
            error += ty;
            ty += 2;
        }
        if (error > 0) {
            --x;
            tx += 2;
            error += (tx - diameter);
        }
    }
}
void saveDrawingToFile(const char *filename) {
    SDL_SetRenderTarget(renderer, drawingTexture);
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, WINDOW_WIDTH, WINDOW_HEIGHT - 40, 32, SDL_PIXELFORMAT_RGBA32);
    if (!surface) {
        printf("Failed to create surface: %s\n", SDL_GetError());
        return;
    }
    if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA32, surface->pixels, surface->pitch) != 0) {
        printf("Failed to read pixels: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    if (IMG_SavePNG(surface, filename) != 0) {
        printf("Failed to save PNG: %s\n", IMG_GetError());
    } else {
        printf("Saved drawing to %s\n", filename);
    }
    SDL_FreeSurface(surface);
    SDL_SetRenderTarget(renderer, NULL);
}

void renderText(const char *text, int x, int y) {
    SDL_Surface *textSurface = TTF_RenderText_Blended(font, text, textColor);
    if (!textSurface) {
        printf("Text rendering failed! SDL_ttf Error: %s\n", TTF_GetError());
        return;
    }
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect dstRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &dstRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void drawCheckerboard() {
    for (int i = 0; i < WINDOW_WIDTH / GRID_SIZE; i++) {
        for (int j = 0; j < (WINDOW_HEIGHT - 40) / GRID_SIZE; j++) {
            if ((i + j) % 2 == 0) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            }
            SDL_Rect rect = {i * GRID_SIZE, j * GRID_SIZE + 40, GRID_SIZE, GRID_SIZE};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

void drawColorPalette() {
    for (int i = 0; i < NUM_COLORS; i++) {
        SDL_SetRenderDrawColor(renderer, colors[i].r, colors[i].g, colors[i].b, 255);
        SDL_Rect rect = {i * 60, 0, 60, 40}; 
        SDL_RenderFillRect(renderer, &rect);
    }
}

void handleDrawing(SDL_Event *e) {
    if (e->type == SDL_MOUSEBUTTONDOWN || (e->type == SDL_MOUSEMOTION && SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))) {
        if (e->button.y < 40) {
            int selectedColorIndex = e->button.x / 60;  
            if (selectedColorIndex < NUM_COLORS) {
                currentColor = colors[selectedColorIndex];
            }
        } else {
            int x = e->button.x / GRID_SIZE;
            int y = (e->button.y - 40) / GRID_SIZE;  
            if (x < WINDOW_WIDTH / GRID_SIZE && y < (WINDOW_HEIGHT - 40) / GRID_SIZE) {
                SDL_SetRenderTarget(renderer, drawingTexture);
                SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
                SDL_Rect rect = {x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE};
                SDL_RenderFillRect(renderer, &rect);
                SDL_SetRenderTarget(renderer, NULL);
            }
        }
    }
    if (e->type == SDL_TEXTINPUT) {
        if (strlen(rgbInput) < 7 && isxdigit(e->text.text[0])) {
            strncat(rgbInput, e->text.text, sizeof(rgbInput) - strlen(rgbInput) - 1);
        }
    }
    else if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_BACKSPACE) {
        if (strlen(rgbInput) > 1) {
            rgbInput[strlen(rgbInput) - 1] = '\0'; 
        }
    }
}
int convertHexChar(char c) {
    if (c>='0' && c<='9') return c-'0';
    if (c>='A' && c<='F') return c-'A'+10;
    if (c>='a' && c<='f') return c-'a'+10;
    return -1;
}
void updateDisplayColor() {
    int length=strlen(rgbInput);
    int valid=1;
    for (int i=1;i<length;i++) {
        if (convertHexChar(rgbInput[i])==-1) {
            valid=0;
            break;
        }
    }
    length--;
    if (valid && length) {
        colors[NUM_COLORS-1].r=convertHexChar(rgbInput[1])*16+convertHexChar(rgbInput[1%length+1]);
        colors[NUM_COLORS-1].g=convertHexChar(rgbInput[2%length+1])*16+convertHexChar(rgbInput[3%length+1]);
        colors[NUM_COLORS-1].b=convertHexChar(rgbInput[4%length+1])*16+convertHexChar(rgbInput[5%length+1]);
    } else {
        colors[NUM_COLORS-1]=(SDL_Color){255,255,255,255}; 
    }
}
int main() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);
    Uint32 lastTime = SDL_GetTicks();
    font = TTF_OpenFont("/usr/share/fonts/FiraCodeNerdFont-Regular.ttf", 24);
    SDL_Window *window = SDL_CreateWindow("Pixel Art Maker", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    drawingTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT - 40);
    SDL_SetTextureBlendMode(drawingTexture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, drawingTexture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, NULL);

    SDL_StartTextInput();
    int running = 1;
    SDL_Event e;
    int frameCount=0;

    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        frameCount++;
        if (currentTime - lastTime >= 1000) {
            float frameRate = frameCount / deltaTime;
            printf("Frame Rate: %.2f FPS\n", frameRate);
            frameCount = 0;
            lastTime = currentTime;
        }
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_s) {
                saveDrawingToFile("drawing.png");
            }
            handleDrawing(&e);
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        drawColorPalette();
        drawCheckerboard();
        updateDisplayColor();
        DrawFilledCircle(renderer,200,200,200);
        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderCopy(renderer, drawingTexture, NULL, &(SDL_Rect){0, 40, WINDOW_WIDTH, WINDOW_HEIGHT - 40});
        renderText("RGB: ", 450, 10);
        renderText(rgbInput, 500, 10);

        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }

    SDL_StopTextInput();
    SDL_DestroyTexture(drawingTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    IMG_Quit();
    SDL_Quit();
    TTF_Quit();
    return 0;
}
