#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib> // For rand and srand functions
#include <ctime>   // For time function

#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL2_gfx.h>
#include <SDL2/SDL_mixer.h>



#define WIDTH 1080
#define HEIGHT 720
#define DotRadius 3
#define DotDistance 7


int ballRadius = 40;
int FirstRowY ;
int leveNumber = 5;
double shootingBallSpeed = 7;
int NUMROWS = 1;
int numberOfTexture = 8;
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
    bool isSeen;
    bool isBlack;
    bool twoColor;
    bool isChained;
    Color color;
    SDL_Texture * texture;
    Ball * ball[6];
};
struct Node{
    Ball * value;
    Node * next;
    Node * previous;
};

int numberBall;

Ball ** FirstRow = reinterpret_cast<Ball **>(new Ball[  WIDTH / ( 2 * ballRadius ) + 1 ] );
Ball * shootingBall = nullptr;
Node * visibleBalls;
Node * allBalls;
Node * bfsQueue;
//----------------------------------------------------------------------------
// declaration of color struct
Color colors [20] = {{1,0,20,255},
                     {0,89,155,255},
                     {154,61,0,255},
                     {89,154,0,255},
                     {255,74,151,255},
                     {89,0,154,255},
                     {154,0,62,255},
                     {254,151,74,255},
                     {0,125,125,255},{0,128,0,255},{0,0,128,255},
                     {128,0,0,255},{255,165,5,255},{255,192,205,255},
                     {0,128,125,255},{255,0,0,255},{75,0,130,255},
                     {255,99,71,255},{124,252,0,255},{0,127,128,255}
};
SDL_Texture * textures [8];
SDL_Texture * backGround;
SDL_Texture * menuBack;
Color Dot = {50, 70 , 120, 255};
Color BackGround = {10, 20, 30, 220};
//////////////////////////////////////////////////////////////////////////////////////
//=============================== Function Decluration ===============================
void setTexture(SDL_Renderer * renderer, SDL_Window * window);
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
Node * AddtoLinkedList(Ball* value, Node * root);
Node * deleteFromLinkedList(Ball * value, Node * root);
Node * findNodeLinkedList(Ball * value, Node * root);
void nextRow(Ball *first, bool isEven, int numColor);
void addLastRowToLinkedList(Ball * root);
void level(int levelNumber);
void connectShootingBall(Node * target, int impactY, int impactX, double slope);
int  findSameColorBall(Ball* root);
void startGame(SDL_Renderer *renderer, SDL_Window  * window);

// ============= Implementation ===================================
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

    // ------------- SDL Initialization ------------
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDLInitialization(&renderer, &window);
    setTexture(renderer, window);
    // Load music
    Mix_Music *music = Mix_LoadMUS("..\\Menu.mp3");
    if (!music) {
        exit(3);
    }


    // additional details of mouse
    int mouse_x, mouse_y;

    SDL_Rect back = {0, 0, WIDTH, HEIGHT};
    SDL_Event event;
    bool quit = false;
    bool isMusicPlaying = false;
    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    SDL_GetMouseState(&mouse_x, &mouse_y);
                    if ( mouse_y > (.94)*(double)(HEIGHT) && mouse_x > (0.12) * (double)(WIDTH) && mouse_x < (0.8) * (double)WIDTH){
                        SDL_Delay(1000);
                        startGame(renderer, window);
                    }
                    std::cout << mouse_x << " " << mouse_y << std::endl;
                }
            } else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
            }
        }
        // Play music if it's not playing
        if (!isMusicPlaying) {
            Mix_PlayMusic(music, -1);
            isMusicPlaying = true;
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, menuBack, nullptr, &back);

        SDL_RenderPresent(renderer);
    }
    outputFile.close();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    SDL_Quit();
    return 0;
}
void startGame(SDL_Renderer *renderer, SDL_Window  * window){
    Mix_HaltMusic();
    // create a shooting ball
    createShootingBall(leveNumber, renderer);
    Dot.red = shootingBall->color.red;
    Dot.green = shootingBall->color.green;
    Dot.blue = shootingBall->color.blue;
    Dot.alpha = shootingBall->color.alpha;
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
        exit(1);
    }
    SDL_Texture* shootedCrossBow = loadTexture(renderer, "..\\crossbowShooted.png");
    if (shootedCrossBow == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(1);
    }

    level(2);
    SDL_Rect back = {0, 0, WIDTH, HEIGHT};
    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
                exit(2);
            }else if(event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    SDL_GetMouseState(&mouse_x, &mouse_y);
                    movingShootingBall(renderer, shootedCrossBow,  mouse_x, mouse_y);
                }
            }else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
                exit(2);
            }
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, backGround, nullptr, &back);

        pointing(renderer, crossbow);drawBalls(renderer);
        // Delay to control the loop speed
        //SDL_Delay(10);
        SDL_RenderPresent(renderer);
    }
}
void connectShootingBall(Node * target, int impactY, int impactX, double slope ){
    if ( target == nullptr ) return ;
    //findSameColorBall(target->value);
    int differenceX = target -> value -> x - impactX;
    int differenceY = target -> value -> y - impactY;

    if ( slope >= 0) {
        if ( differenceX >  0  && target -> value -> x != ballRadius ) {

            shootingBall -> x = target -> value -> x - ballRadius;
            shootingBall -> y = target -> value -> y + 2 * ballRadius * cos(M_PI/6);
            shootingBall -> ball[5] = target -> value;
            target -> value -> ball[3] = shootingBall;
            allBalls = AddtoLinkedList(shootingBall, allBalls);
            visibleBalls = AddtoLinkedList(shootingBall, visibleBalls);
        } else if ( differenceX < 0 ){
            if ( abs(differenceY) < ballRadius * sin(M_PI/6) ){

                shootingBall -> x = target -> value -> x + 2 * ballRadius;
                shootingBall -> y = target -> value -> y ;
                shootingBall -> ball[3] = target -> value;
                target -> value -> ball[0] = shootingBall;
                allBalls = AddtoLinkedList(shootingBall, allBalls);
                visibleBalls = AddtoLinkedList(shootingBall, visibleBalls);
            } else if ( differenceY >= ballRadius * sin(M_PI/6) && differenceY <= ballRadius) {
                shootingBall -> x = target -> value -> x + ballRadius;
                shootingBall -> y = target -> value -> y + 2 * ballRadius * cos(M_PI/6);
                shootingBall -> ball[5] = target -> value;
                target -> value -> ball[2] = shootingBall;
                allBalls = AddtoLinkedList(shootingBall, allBalls);
                visibleBalls = AddtoLinkedList(shootingBall, visibleBalls);
            } else if ( differenceY <= -1 * ballRadius * sin(M_PI/6) && differenceY >= -ballRadius) {
                shootingBall -> x = target -> value -> x + ballRadius;
                shootingBall -> y = target -> value -> y - 2 * ballRadius * cos(M_PI/6);
                shootingBall -> ball[4] = target -> value;
                target -> value -> ball[1] = shootingBall;
                allBalls = AddtoLinkedList(shootingBall, allBalls);
                visibleBalls = AddtoLinkedList(shootingBall, visibleBalls);
            }
        }
    }else{
        if ( differenceX < 0 && target -> value -> x  != ( WIDTH - ( WIDTH / ( 2 * ballRadius ) + 1  )) ){

            shootingBall -> x = target -> value -> x + ballRadius;
            shootingBall -> y = target -> value -> y + 2 * ballRadius * cos(M_PI/6);
            shootingBall -> ball[4] = target -> value;
            target -> value -> ball[1] = shootingBall;
            allBalls = AddtoLinkedList(shootingBall, allBalls);
            visibleBalls = AddtoLinkedList(shootingBall, visibleBalls);
        } else if ( differenceX > 0 ){
            shootingBall -> x = target -> value -> x - ballRadius;
            shootingBall -> y = target -> value -> y + 2 * ballRadius * cos(M_PI/6);
            shootingBall -> ball[5] = target -> value;
            target -> value -> ball[3] = shootingBall;
            allBalls = AddtoLinkedList(shootingBall, allBalls);
            visibleBalls = AddtoLinkedList(shootingBall, visibleBalls);
        }
    }


}
int  findSameColorBall(Ball* root){
    Node * ball = allBalls;
    while( ball != nullptr ) {
        ball->value->isSeen = false;
        ball = ball->next;
    }


}
void pointing(SDL_Renderer * renderer,SDL_Texture * crossbow){
    double source_x, source_y;
    double dest_x, dest_y;
    double teta, slope;
    bool collisionHappened ;
    int lineCircle_x;
    int mouse_x, mouse_y;
    double minY;
    Node * minBall;
    Node * root = visibleBalls;
    double minLineCircle;

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
        minY = 0 ;
        minBall = nullptr;
        minLineCircle = 0;
        root = allBalls;
//        if ( slope > 0 ) {
//            while(root->next != nullptr){
//                root = root -> next;
//            }
//        }
        while( root != nullptr ){
            lineCircle_x =lineCircleImpact(root->value->x, root->value->y, root->value->raduis, slope, source_y - slope * source_x) ;
            if ( lineCircle_x != 0 )
            {
                if ( minY < source_y + slope * (lineCircle_x - source_x)) {
                    minY = source_y + slope * (lineCircle_x - source_x);
                    minBall = root ;
                    minLineCircle = lineCircle_x;
                }
            }
            root = root->next;
        }
        root = minBall;
        lineCircle_x = minLineCircle;
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
        SDL_Rect texture = {static_cast<int>(shootingBall->x - shootingBall->raduis),static_cast<int>(shootingBall->y - shootingBall->raduis), static_cast<int>(2 * shootingBall->raduis), static_cast<int>(2 * shootingBall->raduis)};
        SDL_RenderCopy(renderer, shootingBall->texture, nullptr, &texture);
        if(dest_y <= 0 || mouse_y == shootingBall->y || collisionHappened) break;
    }
}
void createBallRowOne (int numColor){
    int x = ballRadius, y = FirstRowY;
    Ball *ballTemp, *previousBall = nullptr;
    int isBlack;
    for ( int ballIndex = 0 ; ballIndex <WIDTH/(2 * ballRadius)  ; ballIndex++) {
        ballTemp = new Ball;
        ballTemp->x = x;
        ballTemp->y = y;
        ballTemp->raduis = 40;
        isBlack = rand()%(numberOfTexture-1) + 1;
        ballTemp->color = colors[isBlack];
        ballTemp->texture = textures[isBlack];
        ballTemp->isBlack = isBlack == 0;
        allBalls = AddtoLinkedList(ballTemp, allBalls);
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
    Node * root = allBalls;
    Ball * currentBall;
    SDL_Rect texture;
    while( root != nullptr ){
        currentBall = root -> value;
        texture = {static_cast<int>(currentBall->x - currentBall->raduis), static_cast<int>(currentBall->y - currentBall->raduis), static_cast<int>(2 * currentBall->raduis), static_cast<int>(2 * currentBall->raduis)};
        SDL_RenderCopy(renderer, currentBall->texture, nullptr, &texture);
        root = root -> next;
    }
}
void SDLInitialization(SDL_Renderer ** renderer, SDL_Window ** window){
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        exit(1);
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        SDL_Quit();
        exit(1);
    }
    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        exit(2);
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
    int index = rand()%7 + 1;
    shootingBall ->color = colors[index];
    shootingBall -> raduis = ballRadius;
    shootingBall -> isBlack = false;
    shootingBall -> texture = textures[index];

    for ( int  b = 0 ; b < 6 ; b++ ){
        shootingBall-> ball[b] = nullptr;
    }
}
void movingShootingBall(SDL_Renderer * renderer, SDL_Texture * crossbowShooted,  int mouse_x, int mouse_y){
    //finding the shooting light details
    double source_x, source_y;
    double dest_x, dest_y;
    double impact_y;
    double teta, slope;
    bool collisionHappened = false;
    int lineCircle_x;
    int incSign;
    Node * root;
    Node * minBall;
    double minY;
    double minLineCircle;
    bool isIncX = true;
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
        minY = 0 ;
        minBall = nullptr;
        minLineCircle = 0;
        root = allBalls;
        while( root != nullptr ){
            lineCircle_x =lineCircleImpact(root->value->x, root->value->y, root->value->raduis, slope, source_y - slope * source_x) ;
            if ( lineCircle_x != 0 )
            {
                if ( minY < source_y + slope * (lineCircle_x - source_x)) {
                    minY = source_y + slope * (lineCircle_x - source_x);
                    minBall = root ;
                    minLineCircle = lineCircle_x;
                }
            }
            root = root->next;
        }
        root = minBall;
        lineCircle_x = minLineCircle;
        collisionHappened = lineCircle_x != 0 ;

        collisionHappened = lineCircle_x != 0 ;
        if(collisionHappened){
            dest_x = lineCircle_x - 2 * ballRadius*cos(teta);
            if ( slope > 0 ) dest_x = lineCircle_x + 2*ballRadius*cos(teta);
        }else {
            if(slope > 0){
                dest_x = ballRadius;

            }else{
                dest_x = WIDTH-ballRadius;
            }
        }

        dest_y = source_y + slope * (dest_x - source_x);
        impact_y = source_y + slope * (lineCircle_x - source_x);
        incSign = (slope < 0) ? 1 : -1;
        if ( abs(dest_x - source_x) < abs(dest_y - source_y)) isIncX = false;
        else isIncX = true;

        SDL_Rect back = {0, 0, WIDTH, HEIGHT};

        while(true) {
            if ( slope < 0 ){
                if ( dest_x - source_x < 0 ) {
                    connectShootingBall(root, (int)impact_y, lineCircle_x, slope);
                    break;
                }
            } else {
                if ( source_x - dest_x < 0 ) {
                    connectShootingBall(root, (int)impact_y, lineCircle_x, slope);
                    break;
                }
            }
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, backGround, nullptr, &back);

            drawBalls(renderer);
            shootingBall->x = source_x;
            shootingBall->y = source_y;

            // is the slope is very high we need to decrement y axis
            // so that's why we're using -1 instead of incSign
            // cause y only need to be decremented
            source_x += isIncX ? shootingBallSpeed * incSign : shootingBallSpeed * -1 / slope;
            source_y += isIncX ? shootingBallSpeed * incSign * slope : -1 * shootingBallSpeed ;
            SDL_Rect texture = {static_cast<int>(shootingBall->x - shootingBall->raduis),static_cast<int>(shootingBall->y - shootingBall->raduis), static_cast<int>(2 * shootingBall->raduis), static_cast<int>(2 * shootingBall->raduis)};
            SDL_RenderCopy(renderer, shootingBall->texture, nullptr, &texture);
            renderTexture(crossbowShooted, renderer, (WIDTH - 180)/2, HEIGHT - 130, angle, &center, SDL_FLIP_NONE);
            SDL_RenderPresent(renderer);

        }
        source_x = dest_x;
        source_y = dest_y;
        slope *= -1;
        teta = atan(slope);

        if( dest_y < FirstRowY || mouse_y == shootingBall->y || collisionHappened) break;
    }
    createShootingBall(leveNumber, renderer);
    Dot.red = shootingBall->color.red;
    Dot.green = shootingBall->color.green;
    Dot.blue = shootingBall->color.blue;
    Dot.alpha = shootingBall->color.alpha;
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
Node * AddtoLinkedList(Ball* value, Node * root){
    Node * newNode = new Node;
    newNode->value = value;
    newNode->next  = nullptr;
    newNode->previous = nullptr;
    if ( root == nullptr ){
        return  newNode;
    }
    Node * currentNode = root;
    while( currentNode -> next != nullptr ){
        currentNode = currentNode -> next;
    }
    newNode -> previous = currentNode;
    currentNode -> next = newNode;
    return root;
}
Node * deleteFromLinkedList(Ball * value, Node * root){
    Node * currentNode = root;
    while( currentNode != nullptr ) {
        if ( currentNode -> value == value){
            break;
        }
        currentNode = currentNode -> next;
        if ( currentNode == nullptr ) break;
    }
    if ( currentNode != nullptr ){
        if ( currentNode -> next != nullptr )
            currentNode -> next -> previous = currentNode -> previous;


        if ( currentNode -> previous != nullptr ) {
            currentNode->previous->next = currentNode->next;
        } else {
            Node * temp = currentNode -> next;
            free(currentNode);
            return temp;
        }
        free(currentNode);
    }
    return root;

}
Node * findNodeLinkedList(Ball * value, Node * root){
    while ( root != nullptr ) {
        if ( root -> value == value )
            return root ;
    }
    return root;
}
void nextRow(Ball *first, bool isEven, int numColor){
    double x , y ;
    Ball *second = first->ball[1];
    x = first->x + first->raduis;
    y = first->y + sqrt(3) * first->raduis;
    if(!isEven){
        Ball *firstSave = first;
        x = first->x - first->raduis;
        first = nullptr;
        second = firstSave;

    }
    int black;
    Ball * prev = nullptr;
    while(second != nullptr){
        Ball * temp = new Ball;
        temp -> x = x;
        temp -> y = y ;
        temp -> raduis = 40;
        black = rand()%numberOfTexture;
        temp -> color = colors[black];
        temp -> texture = textures[black];
        temp -> isBlack = black == 0;
        allBalls = AddtoLinkedList(temp, allBalls);
        for ( int ballptr = 0 ; ballptr < 6 ; ballptr++ ){
            temp->ball[ballptr] = nullptr;
        }

        temp-> ball[5] = first;
        if(first != nullptr)
            first-> ball[2] = temp;
        temp->ball[0] = second;
        if ( second != nullptr)
            second->ball[3] = temp;


        if( prev != nullptr ){
            temp->ball[4] = prev;
            prev->ball[1] = temp;
        }
        prev = temp;
        x += 80;
        first = second;
        if( second != nullptr)
            second = second -> ball[1];

    }
    if(!isEven){
        Ball * temp = new Ball;
        temp -> x = x;
        temp -> y = y ;
        temp -> raduis = 40;
        black = rand()%numberOfTexture;
        temp -> color = colors[black];
        temp -> texture = textures[black];
        temp -> isBlack = black == 0;
        allBalls = AddtoLinkedList(temp, allBalls);
        for ( int ballptr = 0 ; ballptr < 6 ; ballptr++ ){
            temp->ball[ballptr] = nullptr;
        }
        temp->ball[4] = prev;
        prev->ball[1] = temp;
        temp-> ball[5] = first;
        first-> ball[2] = temp;
    }
}
void level(int levelNumber){
    NUMROWS = levelNumber * 3 ;
    int numColor = levelNumber ;
    FirstRowY = -40*sqrt(3)*(NUMROWS - 1 - 5);
    createBallRowOne(numColor);
    Ball * rootRow = FirstRow[0], *lastRowStart;
    bool isOdd = true;
    for(int i = 0 ; i < NUMROWS ; i++){
        nextRow(rootRow, isOdd, numColor);
        lastRowStart = rootRow;
        if(isOdd){
            rootRow = rootRow -> ball[2];
        }else{
            rootRow = rootRow -> ball[3];
        }
        isOdd = !isOdd;
    }
    lastRowStart = rootRow;
    addLastRowToLinkedList(lastRowStart);
}
void addLastRowToLinkedList(Ball * root){
    if( root != nullptr ){
        if (root -> ball[5] != nullptr )
            visibleBalls = AddtoLinkedList(root -> ball[5], visibleBalls);
        if (root -> ball[0] != nullptr )
            visibleBalls = AddtoLinkedList(root -> ball[0], visibleBalls);
    }
    while(root -> ball[1]  != nullptr){
        visibleBalls = AddtoLinkedList(root, visibleBalls);
        root = root -> ball[1];
    }
    visibleBalls = AddtoLinkedList(root, visibleBalls);
    if (root -> ball[5] != nullptr )
        visibleBalls = AddtoLinkedList(root -> ball[5], visibleBalls);
    if (root -> ball[0] != nullptr )
        visibleBalls = AddtoLinkedList(root -> ball[0], visibleBalls);
}
void setTexture(SDL_Renderer * renderer, SDL_Window * window){
    // --------------- setting the back ground color ----------------
    SDL_Texture* background = loadTexture(renderer, "..\\GameBack.jpg");
    if (background == nullptr){
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(1);
    } backGround = background;
    // --------------- setting the menu background color ----------------
    SDL_Texture* menu_background = loadTexture(renderer, "..\\photo_2024-01-29_16-03-55.jpg");
    if (menu_background == nullptr){
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(1);
    } menuBack = menu_background;
    // ---------------- set other balls texture in the array ----------------
    SDL_Texture* blackBall = loadTexture(renderer, "..\\BackBall.png");
    if (blackBall == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(1);
    }    SDL_Texture* blueBall = loadTexture(renderer, "..\\BlueBall.png");
    if (blueBall == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(2);
    }    SDL_Texture* brownBall = loadTexture(renderer, "..\\BrownBall.png");
    if (brownBall == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(3);
    }    SDL_Texture* greenBall = loadTexture(renderer, "..\\GreenBall.png");
    if (greenBall == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(4);
    }    SDL_Texture* pinkBall = loadTexture(renderer, "..\\pinkBall.png");
    if (pinkBall == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(5);
    }SDL_Texture* purpleBall = loadTexture(renderer, "..\\purpleBall.png");
    if (purpleBall == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(6);
    }SDL_Texture* redBall = loadTexture(renderer, "..\\RedBall.png");
    if (redBall == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(7);
    }SDL_Texture* yellowBall = loadTexture(renderer, "..\\yellowBall.png");
    if (yellowBall == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(8);
    }
    textures[0] = blackBall;
    textures[1] = blueBall;
    textures[2] = brownBall;
    textures[3] = greenBall;
    textures[4] = pinkBall;
    textures[5] = purpleBall;
    textures[6] = redBall;
    textures[7] = yellowBall;
}

//
//while( ball != nullptr ) {
//ball->value->isSeen = false;
//ball = ball->next;
//}
//for ( int i_firstBall_i = 0 ; i_firstBall_i < WIDTH / ( 2 * ballRadius ) + 1; i_firstBall_i++){
//
//}