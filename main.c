#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <SDL2/SDL_image.h>
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
    {255,255,255,255},
};
SDL_Color currentColor = {0, 0, 0, 255}; 
SDL_Color grid[WINDOW_WIDTH / GRID_SIZE][WINDOW_HEIGHT / GRID_SIZE];
int isDrawing = 0;
SDL_Renderer *renderer;
TTF_Font *font;
SDL_Texture *textTexture;
SDL_Rect inputBox = {200, 5, 200, 40};  
SDL_Color textColor = {0, 0, 0, 255};   
char rgbInput[20] = " ";
void saveDrawingToFile(const char *filename) {
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_PIXELFORMAT_RGBA32);
    if (!surface) {
        printf("Failed to create surface: %s\n", SDL_GetError());
        return;
    }

    // Read pixels from the renderer into the surface
    if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA32, surface->pixels, surface->pitch) != 0) {
        printf("Failed to read pixels: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    // Save the surface to a PNG file
    if (IMG_SavePNG(surface, filename) != 0) {
        printf("Failed to save PNG: %s\n", IMG_GetError());
    } else {
        printf("Saved drawing to %s\n", filename);
    }

    SDL_FreeSurface(surface);
}
void renderText(const char *text, int x, int y) {
    SDL_Surface *textSurface = TTF_RenderText_Blended(font, text, textColor);
    if (!textSurface) {
        printf("Text rendering failed! SDL_ttf Error: %s\n", TTF_GetError());
        return;
    }
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect dstRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &dstRect);
    SDL_FreeSurface(textSurface);
}
void drawGrid() {
    for (int i = 0; i < WINDOW_WIDTH / GRID_SIZE; i++) {
        for (int j = 0; j < WINDOW_HEIGHT / GRID_SIZE; j++) {
            SDL_SetRenderDrawColor(renderer, grid[i][j].r, grid[i][j].g, grid[i][j].b, 255);
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
    if (e->type == SDL_MOUSEBUTTONDOWN) {
        if (e->button.y < 40) {
            int selectedColorIndex = e->button.x / 60;  
            if (selectedColorIndex < NUM_COLORS) {
                currentColor = colors[selectedColorIndex];  
            }
        }
        else {
            isDrawing = 1; 
            int x = e->button.x / GRID_SIZE;
            int y = (e->button.y - 40) / GRID_SIZE;  
            if (x < WINDOW_WIDTH / GRID_SIZE && y < WINDOW_HEIGHT / GRID_SIZE) {
                grid[x][y] = currentColor;
            }
        }
    }
    else if (e->type == SDL_MOUSEBUTTONUP) {
        isDrawing = 0; 
    }
    if (e->type == SDL_MOUSEMOTION && isDrawing) {
        int x = e->motion.x / GRID_SIZE;
        int y = (e->motion.y - 40) / GRID_SIZE;  
        if (x < WINDOW_WIDTH / GRID_SIZE && y < WINDOW_HEIGHT / GRID_SIZE) {
            grid[x][y] = currentColor;
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
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return -1;
    }
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return -1;
    }
    font = TTF_OpenFont("/usr/share/fonts/FiraCodeNerdFont-Regular.ttf", 24);
    if (font == NULL) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return -1;
    }
    SDL_Window *window = SDL_CreateWindow("Pixel Art Maker", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    for (int i = 0; i < WINDOW_WIDTH / GRID_SIZE; i++) {
        for (int j = 0; j < WINDOW_HEIGHT / GRID_SIZE; j++) {
            grid[i][j] = currentColor;
        }
    }
    SDL_StartTextInput();
    int running = 1;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_s) {
                printf("here\n");
                saveDrawingToFile("drawing.png");
            }
            handleDrawing(&e);
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        drawColorPalette();
        drawGrid();
        updateDisplayColor();
        renderText("RGB: ", 450, 10);
        renderText(rgbInput, 500, 10);
        SDL_RenderPresent(renderer);

    }
    SDL_StopTextInput();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    IMG_Quit();
    SDL_Quit();
    TTF_Quit();
    return 0;
}
