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
#define DotRadius 3
#define DotDistance 7


int ballRadius = 40;
int FirstRowY = ballRadius;
int leveNumber = 5;
double shootingBallSpeed = 15;
int NUMROWS = 1;

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
    bool isBlack;
    bool twoColor;
    bool isChained;
    Color color [2];
    Ball * ball[6];
};

int numberBall;

Ball ** FirstRow = reinterpret_cast<Ball **>(new Ball[  WIDTH / ( 2 * ballRadius ) + 1 ] );
Ball * shootingBall = nullptr;

//----------------------------------------------------------------------------
// declaration of color struct
Color colors [20] = {{255,0,0,255},
                     {0,255,0,255},
                     {0,0,255,255},
                     {255,255,0,255},
                     {255,0,255,255},
                     {0,255,255,255},
                     {128,128,0,255},
                     {128,0,124,255},
                     {0,125,125,255},
                     {0,128,0,255},
                     {0,0,128,255},
                     {128,0,0,255},
                     {255,165,5,255},
                     {255,192,205,255},
                     {0,128,125,255},
                     {255,0,0,255},
                     {75,0,130,255},
                     {255,99,71,255},
                     {124,252,0,255},
                     {0,127,128,255}
};
Color Dot = {50, 70 , 120, 255};

double lineCircleImpact(double cir_x , double cir_y,double radius, double slope , double bias);
void SDLInitialization(SDL_Renderer ** renderer, SDL_Window ** window);
void movingShootingBall(SDL_Renderer * renderer, SDL_Texture * crossbowShooted, int mouse_x, int mouse_y);
void createShootingBall(int levelNum, SDL_Renderer * renderer);
void dotedLine(SDL_Renderer * renderer, double x0, double y0, double x1 , double y1);
void createBallRowOne (int);
SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path);
void renderTexture(SDL_Texture* texture, SDL_Renderer* renderer, int x, int y, double angle, SDL_Point* center, SDL_RendererFlip flip);
void drawBalls(SDL_Renderer *renderer);
void pointing(SDL_Renderer * renderer,SDL_Texture * crossbowShooted);



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

    SDL_Texture* crossbow = loadTexture(renderer, "..\\crossbow.png");
    if (crossbow == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Texture* shootedCrossBow = loadTexture(renderer, "..\\crossbowShooted.png");
    if (shootedCrossBow == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }


    createBallRowOne(6);

    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }else if(event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    SDL_GetMouseState(&mouse_x, &mouse_y);
                    movingShootingBall(renderer, shootedCrossBow,  mouse_x, mouse_y);
                }
            }else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
            }
        }
        SDL_SetRenderDrawColor(renderer, 150, 150, 130, 100);
        SDL_RenderClear(renderer);
        pointing(renderer, crossbow);
        // Delay to control the loop speed
        //SDL_Delay(10);
        drawBalls(renderer);
        SDL_RenderPresent(renderer);
    }
    outputFile.close();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
void pointing(SDL_Renderer * renderer,SDL_Texture * crossbow){
    double source_x, source_y;
    double dest_x, dest_y;
    double teta, slope;
    bool collisionHappened ;
    int lineCircle_x;
    int mouse_x, mouse_y;

    Ball * root;

    SDL_GetMouseState(&mouse_x, &mouse_y);

    source_x = shootingBall->x;
    source_y = shootingBall->y;
    dest_y   = mouse_y;
    dest_x   = mouse_x;

    slope    = (dest_y - source_y) / (dest_x - source_x);

    if(slope > -0.1 && source_x < dest_x) {
        slope = -0.1;
    }else if( slope < 0.1 && source_x > dest_x){
        slope =  0.1;
    }
    if(slope >  10000) slope = 0.001;
    if(slope < -10000) slope = 0.001;

    teta = atan(slope);
    double angle =  teta * 180 / M_PI + 90 ;  // Rotate based on time
    if( slope > 0 ) angle -= 180;
    SDL_Point center = { 180 / 2, 130 - ballRadius-5 };  // Set the center of rotation to the center of the character

    while(true){
        root = FirstRow[0];
        while( root != nullptr ){
            lineCircle_x =lineCircleImpact(root->x, root->y, root->raduis, slope, source_y - slope * source_x) ;
            if ( lineCircle_x != 0 ) break;
            root = root->ball[1];
        }

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
        renderTexture(crossbow, renderer, (WIDTH - 180)/2, HEIGHT - 130, angle, &center, SDL_FLIP_NONE);
        filledCircleRGBA(renderer, shootingBall->x, shootingBall->y, shootingBall->raduis, shootingBall->color[0].red,shootingBall->color[0].green,shootingBall->color[0].blue,shootingBall->color[0].alpha);
        if(dest_y <= 0 || mouse_y == shootingBall->y || collisionHappened) break;
    }
}
void createBallRowOne (int numColor){
    int x = ballRadius, y = FirstRowY;
    Ball *ballTemp, *previousBall = nullptr;

    for ( int ballIndex = 0 ; ballIndex <WIDTH/(2 * ballRadius) + 1 ; ballIndex++) {
        ballTemp = new Ball;
        ballTemp->x = x;
        ballTemp->y = y;
        ballTemp->raduis = 40;
        int colorIndex = rand() % numColor;
        ballTemp->color[0] = colors[colorIndex];
        for ( int ballptr = 0 ; ballptr < 6 ; ballptr++ ){
            ballTemp->ball[ballptr] = nullptr;
        }
        if(previousBall != nullptr){
            ballTemp -> ball[4] = previousBall;
            previousBall -> ball[1] = ballTemp;
        }
        FirstRow[ballIndex] =  ballTemp;
        previousBall = ballTemp;
        x+=80;
    }



}
void drawBalls(SDL_Renderer *renderer){
    Ball *root,* rootRow = FirstRow[0];
    bool isOdd = true;
    for(int i = 0 ; i < NUMROWS ; i++){
        root = rootRow;
        while(root->ball[1] != nullptr){
            filledCircleRGBA(renderer ,root->x,root->y,40,root->color[0].red, root->color[0].green, root->color[0].blue, root->color[0].alpha);
            root = root -> ball[1];
        }
        if(isOdd){
            rootRow = rootRow -> ball[2];
        }else{
            rootRow = rootRow -> ball[3];
        }
        isOdd = !isOdd;
    }
}
void SDLInitialization(SDL_Renderer ** renderer, SDL_Window ** window){
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        exit(1);
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        SDL_Quit();
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
    double slope = tan(teta);
    bool isY = false;
    if (abs(x1 - x0) < abs(y1-y0)){
        inc = 2 * DotRadius * sin(teta) + DotDistance  ;
        if( x1 > x0 ) inc += DotDistance * 2;
        sw_x = y0;
        y0 = x0;
        x0 = sw_x;

        sw_y = y1;
        y1 = x1;
        x1 = sw_y;
        slope = 1.0 / slope;
        isY = true;
    }
    if ( x0 > x1 ) {
        sw_x = x0;
        x0   = x1;
        x1   = sw_x;
        sw_y = y0;
        y0   = y1;
        y1   = sw_y;
    }

    int isDouble = rand() % 2;
    while(x0 < x1){
        if( isY ) filledCircleRGBA(renderer, y0, x0, DotRadius * (isDouble+1), Dot.red, Dot.green, Dot.blue, Dot.alpha);
        else      filledCircleRGBA(renderer, x0 , y0, DotRadius * (isDouble+1), Dot.red, Dot.green, Dot.blue, Dot.alpha);
        x0 += inc;
        y0 += inc * (slope);
        isDouble = (isDouble + 1) % 2;
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
void movingShootingBall(SDL_Renderer * renderer, SDL_Texture * crossbowShooted,  int mouse_x, int mouse_y){
    //finding the shooting light details
    double source_x, source_y;
    double dest_x, dest_y;
    double teta, slope;
    bool collisionHappened = false;
    int lineCircle_x;
    int incSign;
    Ball * root;
//
    source_x = shootingBall->x;
    source_y = shootingBall->y;
    dest_y   = mouse_y;
    dest_x   = mouse_x;
    slope    = (dest_y - source_y) / (dest_x - source_x);
    teta = atan(slope);

//
    source_x = shootingBall->x;
    source_y = shootingBall->y;
    dest_y   = mouse_y;
    dest_x   = mouse_x;
//
    slope    = (dest_y - source_y) / (dest_x - source_x);

    if(slope > -0.1 && source_x < dest_x) {
        slope = -0.1;
    }else if( slope < 0.1 && source_x > dest_x){
        slope =  0.1;
    }
    if(slope >  10000) slope = 0.001;
    if(slope < -10000) slope = 0.001;
//
    teta = atan(slope);
    double angle =  teta * 180 / M_PI + 90 ;  // Rotate based on time
    if( slope > 0 ) angle -= 180;
    SDL_Point center = { 180 / 2, 130 - ballRadius-5 };  // Set the center of rotation to the center of the character
    while(true){
        root = FirstRow[0];
        while( root != nullptr ){
            lineCircle_x =lineCircleImpact(root->x, root->y, root->raduis, slope, source_y - slope * source_x) ;
            if ( lineCircle_x != 0 ) break;
            root = root->ball[1];
        }
        collisionHappened = lineCircle_x != 0 ;
        if(collisionHappened){
            dest_x = lineCircle_x - ballRadius*cos(teta);
        }else {
            if(slope > 0){
                dest_x = ballRadius;

            }else{
                dest_x = WIDTH-ballRadius;
            }
        }

        dest_y = source_y + slope * (dest_x - source_x);
        incSign = (slope < 0) ? 1 : -1;
        if( abs(slope ) > 70 ) shootingBallSpeed = 0.1;
        else shootingBallSpeed = 15;
        while(true) {
            if ( slope < 0 ){
                if ( dest_x - source_x < 0 ) break;
            } else {
                if ( source_x - dest_x < 0 ) break;
            }
            SDL_SetRenderDrawColor(renderer, 150, 150, 130, 100);
            SDL_RenderClear(renderer);

            shootingBall->x = source_x;
            shootingBall->y = source_y;
            source_x += shootingBallSpeed * incSign ;
            source_y += shootingBallSpeed * incSign * slope ;

            filledCircleRGBA(renderer, shootingBall->x, shootingBall->y, shootingBall->raduis, shootingBall->color[0].red,shootingBall->color[0].green,shootingBall->color[0].blue,shootingBall->color[0].alpha);
            renderTexture(crossbowShooted, renderer, (WIDTH - 180)/2, HEIGHT - 130, angle, &center, SDL_FLIP_NONE);
            drawBalls(renderer);
            SDL_RenderPresent(renderer);

        }
        source_x = dest_x;
        source_y = dest_y;
        slope *= -1;
        teta = atan(slope);

        if( dest_y < 0 || mouse_y == shootingBall->y || collisionHappened) break;
    }
    createShootingBall(leveNumber, renderer);
    Dot.red = shootingBall->color[0].red;
    Dot.green = shootingBall->color[0].green;
    Dot.blue = shootingBall->color[0].blue;
    Dot.alpha = shootingBall->color[0].alpha;
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
SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path) {
    SDL_Texture* texture = nullptr;
    SDL_Surface* surface = IMG_Load(path);
    if (surface == nullptr) {
        printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
    } else {
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture == nullptr) {
            printf("Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError());
        }
        SDL_FreeSurface(surface);
    }
    return texture;
}
void renderTexture(SDL_Texture* texture, SDL_Renderer* renderer, int x, int y, double angle, SDL_Point* center, SDL_RendererFlip flip) {
    SDL_Rect destination = { x, y, 0, 0 };
    SDL_QueryTexture(texture, nullptr, nullptr, &destination.w, &destination.h);
    SDL_RenderCopyEx(renderer, texture, nullptr, &destination, angle, center, flip);

}