#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib> // For rand and srand functions
#include <ctime>   // For time function

#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL2_gfx.h>



#define WIDTH  900
#define HEIGHT 800
#define DotRadius 5
#define DotDistance 7

int ballRadius = 40;
int leveNumber = 5;

struct Color{
    int red;
    int green;
    int blue;
    int alpha;
};
struct Ball {
    double x;
    double y ;
    double raduis;
    Color color [2];
    Ball * ball[6];
};

int numberBall;

Ball * FirstRow [10];

Ball * shootingBall = nullptr;

//----------------------------------------------------------------------------
// declaration of color struct
Color colors [20] = {{255,0,0,128},
                     {0,255,0,128},
                     {0,0,255,128},
                     {255,255,0,128},
                     {255,0,255,128},
                     {0,255,255,128},
                     {128,128,0,128},
                     {128,0,124,128},
                     {0,125,125,128},
                     {0,128,0,128},
                     {0,0,128,128},
                     {128,0,0,128},
                     {255,165,5,128},
                     {255,192,205,128},
                     {0,128,125,64},
                     {255,0,0,179},
                     {75,0,130,128},
                     {255,99,71,128},
                     {124,252,0,128},
                     {0,127,128,128}
};
Color Dot = {50, 70 , 120, 255};

double lineCircleImpact(double cir_x , double cir_y,double radius, double slope , double bias);
void SDLInitialization(SDL_Renderer ** renderer, SDL_Window ** window);
void movingShootingBall(SDL_Renderer * renderer, int mouse_x, int mouse_y);
void createShootingBall(int levelNum, SDL_Renderer * renderer);
void dotedLine(SDL_Renderer * renderer, double x0, double y0, double x1 , double y1);
int main(int argc, char* argv[]) {

    std::string filename = "output.txt";

    // Create an ofstream object and open the file
    std::ofstream outputFile(filename);

    // Check if the file is opened successfully
    if (outputFile.is_open()) {
        // Redirect cout to the output file
        std::streambuf *coutbuf = std::cout.rdbuf();
        std::cout.rdbuf(outputFile.rdbuf());
    }


    SDL_Window* window;
    SDL_Renderer* renderer;
    SDLInitialization(&renderer, &window);

    // create a shooting ball
    createShootingBall(leveNumber, renderer);
    // looping the windows
    SDL_Event event;
    bool quit = false;

    // additional details of mouse
    int mouse_x, mouse_y;


    double source_x, source_y;
    double dest_x, dest_y;
    double teta, slope;
    bool collisionHappened = false;
    int lineCircle_x;
    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }else if(event.type == SDL_MOUSEBUTTONDOWN) {
                //shooting the Ball
                if (event.button.button == SDL_BUTTON_LEFT) {
                    SDL_GetMouseState(&mouse_x, &mouse_y);
                    movingShootingBall(renderer, mouse_x, mouse_y);

                }
            }else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
            }
        }
        SDL_SetRenderDrawColor(renderer, 150, 150, 100, 100);
        SDL_RenderClear(renderer);
        SDL_GetMouseState(&mouse_x, &mouse_y);

        filledCircleRGBA(renderer, shootingBall->x, shootingBall->y, shootingBall->raduis, 0,0,0,255);
        filledCircleRGBA(renderer, 800, 400, 50, 40, 40 , 90, 200);
        source_x = shootingBall->x;
        source_y = shootingBall->y;
        dest_y   = mouse_y;
        dest_x   = mouse_x;

        slope    = (dest_y - source_y) / (dest_x - source_x);
        teta = atan(slope);

        if(slope > -0.1 && source_x < dest_x) {
            slope = -0.1;
        }else if( slope < 0.1 && source_x > dest_x){
            slope =  0.1;
        }
        if(slope >  10000) slope = 0.001;
        if(slope < -10000) slope = 0.001;
        collisionHappened = false;

        while(true){
            lineCircle_x =lineCircleImpact(800, 400, 50, slope, source_y - slope * source_x) ;
            collisionHappened = lineCircle_x != 0 ;
            if(collisionHappened){
                dest_x = lineCircle_x;
            }else {
                if(slope > 0){
                    dest_x = 0;

                }else{
                    dest_x = WIDTH;
                }
            }
            dest_y = source_y + slope * (dest_x - source_x);
            dotedLine(renderer, source_x, source_y, dest_x, dest_y);
            source_x = dest_x;
            source_y = dest_y;
            slope *= -1;
            teta = atan(slope);

            if(dest_y <= 0 || mouse_y == shootingBall->y || collisionHappened) break;
        }
        // Delay to control the loop speed
        SDL_Delay(16);
        SDL_RenderPresent(renderer);
    }
    outputFile.close();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
void SDLInitialization(SDL_Renderer ** renderer, SDL_Window ** window){
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        exit(1);
    }

    // Create a window
     *window = SDL_CreateWindow(
            "SDL Image Example",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            WIDTH,
            HEIGHT,
            SDL_WINDOW_SHOWN
    );
    if (!*window) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    // Create a renderer
    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!*renderer) {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        exit(1);
    }
}
void dotedLine(SDL_Renderer * renderer, double x0, double y0, double x1, double y1){
    double teta = atan((y1-y0)/(x1-x0));
    int inc = 2 * DotRadius * cos(teta) + DotDistance ;
    double sw_x, sw_y;
    if ( x0 > x1 ) {
        sw_x = x0;
        x0   = x1;
        x1   = sw_x;
        sw_y = y0;
        y0   = y1;
        y1   = sw_y;
    }
    while(x0 < x1){
        filledCircleRGBA(renderer, x0, y0, DotRadius, Dot.red, Dot.green, Dot.blue, Dot.alpha);
        x0 += inc;
        y0 += inc * tan(teta);
    }
}
void createShootingBall(int levelNum, SDL_Renderer * renderer){

    shootingBall = new Ball;
    shootingBall -> x = WIDTH/2;
    shootingBall -> y = HEIGHT - 40;

    shootingBall ->color[0] = colors[rand() % (levelNum/3 + 4)];
    shootingBall -> raduis = ballRadius;

    for ( int  b = 0 ; b < 6 ; b++ ){

        shootingBall-> ball[b] = nullptr;
    }
}
void movingShootingBall(SDL_Renderer * renderer, int mouse_x, int mouse_y){
    //finding the shooting light details
    double source_x, source_y;
    double dest_x, dest_y;
    double teta, slope;

    source_x = shootingBall->x;
    source_y = shootingBall->y;
    dest_y   = mouse_y;
    dest_x   = mouse_x;
    slope    = (dest_y - source_y) / (dest_x - source_x);
    teta = atan(slope);


}
double lineCircleImpact(double cir_x , double cir_y,double radius, double slope , double bias){

    double a = 1+ slope * slope;
    double b = -2*cir_x+2*slope*(bias - cir_y);
    double c = cir_x * cir_x + cir_y * cir_y -2*cir_y*bias - radius * radius + bias*bias  ;//circle function
    double delta = (b*b - 4*a*c);

    if( delta < 0){
        return 0;
    }else if ( delta == 0){
        return  -1*b/(2*a);
    }else {
        double x1 , x2;
        x1 = ( (-1 * b - sqrt(delta)) / (2 * a));
        x2 =  ((-1 * b + sqrt(delta)) / (2 * a));
        if( (x1 * slope + bias ) > (x2 * slope + bias)){
            return x1;
        }else {
            return x2;
        }
    }
}