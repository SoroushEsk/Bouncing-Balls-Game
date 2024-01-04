#include <iostream>
#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL2/SDL2_gfx.h>

int main(int argc, char* argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    // Create a window
    SDL_Window* window = SDL_CreateWindow(
            "SDL Image Example",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            800,
            600,
            SDL_WINDOW_SHOWN
    );
    if (!window) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Load an image
    SDL_Surface* imageSurface = IMG_Load("G:\\Saved Pictures\\New folder\\Space Wallpapers.www.fileniko.com (70).jpg");
    if (!imageSurface) {
        SDL_Log("Unable to load image: %s", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Create a texture from the loaded image
    SDL_Texture* imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    SDL_FreeSurface(imageSurface);
    if (!imageTexture) {
        SDL_Log("Unable to create texture: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Main loop
    bool quit = false;
    SDL_Event event;

    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Clear the renderer
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Render the texture
        //SDL_RenderCopy(renderer, imageTexture, NULL, NULL);

        // Present the renderer
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        // Draw a filled circle
        filledCircleColor(renderer, 400, 300, 100, SDL_MapRGB(SDL_GetWindowSurface(window)->format, 0, 0, 0));

        // Present the renderer
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    SDL_DestroyTexture(imageTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
