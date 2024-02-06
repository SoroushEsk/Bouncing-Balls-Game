#include <iostream>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <cstdlib> // For rand and srand functions
#include <ctime>   // For time function

#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL2_gfx.h>
#include <SDL2/SDL_mixer.h>



#define WIDTH 1000
#define HEIGHT 1000
#define DotRadius 3
#define DotDistance 7
const int FONT_SIZE = 50;

/// ******************* Changeable Constraints ****************
int ballRadius = WIDTH / 30;
int FirstRowY ;
int leveNumber = 5;
double shootingBallSpeed = 25;
int NUMROWS = 1;
int numberOfTexture = 5;
int impactUp = -6;
double downSpeed = 0.2;
int numberBall;
int deletedRows;
int BombNumber = 3;
int MultiNumber = 3;
bool lostMuch = false;
/// ==================== Struct Definition =====================
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
    // for bfs
    bool isSeen;
    // for balls that cant be destroyed with shootingBall
    bool isBlack;

    bool twoColor;

    bool isChained;

    bool isMultiColor;

    bool isBomb = false;

    bool isLaser;

    Color color;
    Color color1;
    SDL_Texture * texture;
    SDL_Texture * texture1 = nullptr;
    Ball * ball[6];
};
struct Node{
    Ball * value;
    Node * next;
    Node * previous;
};

/// ==================== Enum Definition  ======================
enum GameMode {
    Random,
    TimeLimit,
    Infinite,
    Menu
};

/// -------------------- Global struct Variables --------------------------------------------
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
SDL_Texture * backGround[4];
int backGroundIndex = 0;
SDL_Texture * menuBack;
SDL_Texture * bomb;
SDL_Texture * multi;
Color Dot = {50, 70 , 120, 255};
Color BackGround = {10, 20, 30, 220};
std::unordered_map<SDL_Texture*, Color*> existingColor;
std::string Username = "";

SDL_Texture* gTextTexture = nullptr;
//////////////////////////////////////////////////////////////////////////////////////
//=============================== Function Decluration ===============================
void setTexture(SDL_Renderer * renderer, SDL_Window * window);
double lineCircleImpact(double cir_x , double cir_y,double radius, double slope , double bias);
void SDLInitialization(SDL_Renderer ** renderer, SDL_Window ** window, TTF_Font** gFont);
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
Node *  findSameColorBall(Ball* target, int *Count);
void startGame(SDL_Renderer *renderer, SDL_Window  * window, GameMode game_mode);
void shootingBallAroundBalls();
void deleteBalls(Node * sameColor);
Node * findFloatingBalls();
void addBarred(SDL_Renderer *renderer, SDL_Window  * window,bool isDisplay);
void renderText(SDL_Renderer* renderer, TTF_Font* gFont, const std::string& text, int x, int y);
void settingDisplay(SDL_Renderer * renderer, SDL_Window *window);
bool isXandYInRect(int x, int y, double min_x, double min_y, double max_x, double max_y);
Node * bombRadius();
GameMode selectGameMode(SDL_Renderer * renderer, SDL_Window * window, TTF_Font * font);
// ============= Implementation ===================================
int main(int argc, char* argv[]) {
//    srand(1);
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
    TTF_Font* gFont = nullptr;
    SDLInitialization(&renderer, &window, &gFont);
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
    bool isMusicStop = false;
    std::string inputText = "";
    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    SDL_GetMouseState(&mouse_x, &mouse_y);
                    if ( mouse_y > (.94)*(double)(HEIGHT) && mouse_x > (0.12) * (double)(WIDTH) && mouse_x < (0.8) * (double)WIDTH){
                        GameMode sel =selectGameMode(renderer, window, gFont);
                        if ( sel == Menu ) continue;
                        startGame(renderer, window, sel);
                    } else if ( mouse_y >= (0.47)*(double)(HEIGHT) && mouse_y <= (0.56)*(double)(HEIGHT) && mouse_x <= (0.16)*(double)(WIDTH) && mouse_x >= (0.06)*(double)(WIDTH)){
                        isMusicStop = !isMusicStop;
                        if (!isMusicStop) {
                            Mix_PlayMusic(music, -1);
                        }else {
                            Mix_HaltMusic();
                        }
                    } else if (  mouse_y >= (0.32)*(double)(HEIGHT) && mouse_y <= (0.41)*(double)(HEIGHT) && mouse_x <= (0.16)*(double)(WIDTH) && mouse_x >= (0.06)*(double)(WIDTH) ){
                        settingDisplay(renderer, window);
                    }

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
        addBarred(renderer, window, isMusicStop);
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
GameMode selectGameMode(SDL_Renderer * renderer, SDL_Window * window, TTF_Font * font){
    SDL_Texture* modeSel[2];
    modeSel[0] = loadTexture(renderer, "..\\StartGame\\GameModeP.png");
    if (modeSel == nullptr){
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(1);
    }
    modeSel[1] = loadTexture(renderer, "..\\StartGame\\username.png");
    if (modeSel == nullptr){
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(1);
    }
    SDL_Rect back = {0, 0, WIDTH, HEIGHT};
    SDL_Event event;
    bool quit = false;
    bool isUsername = false ;
    GameMode mode = Menu;

    std::string inputText = "";
    int mouse_x, mouse_y;
    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
            quit = true;
            exit(2);
            }else if(event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT ) {
                    SDL_GetMouseState(&mouse_x, &mouse_y);
                    std::cout << mouse_x << " " << mouse_y <<std::endl;
                    if (isXandYInRect(mouse_x, mouse_y, 0.03, 0.02 , 0.14, 0.12 )){
                        if ( !isUsername)
                            return Menu;
                        else
                            isUsername = false;

                    }else if (isXandYInRect(mouse_x, mouse_y, 0.17, 0.5, 0.83, 0.83)&& !isUsername){
                        mode =  TimeLimit;
                        isUsername = true;
                    }else if (isXandYInRect(mouse_x, mouse_y, 0.17, 0.39, 0.83, 0.48)&& !isUsername){
                        mode = Random;
                        isUsername = true;
                    }else if (isXandYInRect(mouse_x, mouse_y, 0.17, 0.62, 0.83, 0.8)&& !isUsername){
                        mode =  Infinite;
                        isUsername = true;
                    }
                }
            }else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
                exit(2);
            }else if (event.type == SDL_KEYDOWN && isUsername) {
                if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                    Username = inputText;
                    inputText = "";
                    return mode;
                } else if (event.key.keysym.sym == SDLK_BACKSPACE && !inputText.empty()) {
                    inputText.pop_back();
                } else if ((event.key.keysym.sym >= SDLK_a && event.key.keysym.sym <= SDLK_z) || (event.key.keysym.sym >= SDLK_0 && event.key.keysym.sym <= SDLK_9)) {
                    inputText += static_cast<char>(event.key.keysym.sym);
                }
            }
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, modeSel[isUsername], nullptr, &back);
        if (isUsername) {
            renderText(renderer, font, "Username: " , 0.2*WIDTH, 0.430*HEIGHT);
            renderText(renderer, font, inputText, 0.2*WIDTH, 0.5*HEIGHT);
        }
//
        //SDL_Delay(10);
        SDL_RenderPresent(renderer);
    }
    return mode;
}
void settingDisplay(SDL_Renderer * renderer, SDL_Window *window){
    SDL_Texture* settingPage = loadTexture(renderer, "..\\Setting\\setting.png");
    if (settingPage == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(2);
    }

    int mouse_x, mouse_y;

    SDL_Rect back = {0, 0, WIDTH, HEIGHT};
    SDL_Event event;
    bool quit = false;
    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
                exit(2);
            }else if(event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    SDL_GetMouseState(&mouse_x, &mouse_y);
                    if ( isXandYInRect(mouse_x, mouse_y, 0.09, 0.28, 0.13, 0.32)){
                        return;

                    }else if (isXandYInRect(mouse_x, mouse_y, 0.39, 0.3, 0.6, 0.42)){
                        backGroundIndex = 0;
                    } else if(isXandYInRect(mouse_x, mouse_y, 0.39, 0.44, 0.6, 0.55)){
                        backGroundIndex = 1;
                    } else if(isXandYInRect(mouse_x, mouse_y, 0.69, 0.44, 0.9, 0.55)){
                        backGroundIndex = 3;
                    } else if(isXandYInRect(mouse_x, mouse_y, 0.69, 0.3, 0.9, 0.41)){
                        backGroundIndex = 2;
                    }
                    std::cout << mouse_x << " " << mouse_y << std::endl;
                }
            }else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
                exit(2);
            }
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, settingPage, nullptr, &back);

        // Delay to control the loop speed
        //SDL_Delay(10);
        SDL_RenderPresent(renderer);
    }
}
bool isXandYInRect(int x, int y, double min_x, double min_y, double max_x, double max_y){
    if ( y >= (min_y)*(double)(HEIGHT) && y <= (max_y)*(double)(HEIGHT) && x <= (max_x)*(double)(WIDTH) && x >= (min_x)*(double)(WIDTH)){
        return true;

    }
    else
        return false;
}
void startGame(SDL_Renderer *renderer, SDL_Window  * window, GameMode game_mode){
    Mix_HaltMusic();
    // create a shooting ball
    level(1);
    drawBalls(renderer);
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
    SDL_Texture* victory = loadTexture(renderer, "..\\Game Background\\victory.jpg");
    if (victory == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(1);
    }
    SDL_Texture* fail = loadTexture(renderer, "..\\Game Background\\fail.jpg");
    if (fail == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(1);
    }
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

        // Delay to control the loop speed
        //SDL_Delay(10);
        if ( allBalls == nullptr ) {
            SDL_RenderCopy(renderer, victory, nullptr, &back);
            ///////////////////////////////////////////////////////////////////////////////
            SDL_RenderPresent(renderer);
            SDL_Delay(3000);
            return;
        }else if (lostMuch){
            SDL_RenderCopy(renderer, fail, nullptr, &back);
            ///////////////////////////////////////////////////////////////////////////////
            SDL_RenderPresent(renderer);
            SDL_Delay(3000);
            return;
        }else {
            SDL_RenderCopy(renderer, backGround[backGroundIndex], nullptr, &back);

            pointing(renderer, crossbow);drawBalls(renderer);

            SDL_Rect  rect = {0, HEIGHT - 130, WIDTH, 10};
            SDL_SetRenderDrawColor(renderer, 255,0,0,255);
            SDL_RenderFillRect(renderer,&rect);
        }
        SDL_RenderPresent(renderer);
    }
}
void addBarred(SDL_Renderer *renderer, SDL_Window  * window,bool isDiplay){
    SDL_Texture* bar = loadTexture(renderer, "..\\barred.png");
    if (bar == nullptr){
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(1);
    }
    if (isDiplay){
        SDL_Rect display = {static_cast<int>((0.06)*(double)(WIDTH)), static_cast<int>((0.47)*(double)(HEIGHT)), static_cast<int>((0.1)*(double)(WIDTH)), static_cast<int>((0.09)*(double)(HEIGHT))};
        SDL_RenderCopy(renderer, bar, nullptr, &display);
    }
}
void connectShootingBall(Node * target, int impactY, int impactX, double slope ){
    if ( target == nullptr ) return ;
    //findSameColorBall(target->value);
    int differenceX = target -> value -> x - impactX;
    int differenceY = target -> value -> y - impactY;
    Ball * targetBall = target -> value ;
    if ( slope >= 0) {
        if ( differenceX >  0  && target -> value -> x != ballRadius && target -> value -> x  != ( WIDTH - ballRadius ) ) {
            if ( targetBall->ball[3] ){
                shootingBall -> x = target -> value -> x + ballRadius;
                shootingBall -> y = target -> value -> y + 2 * ballRadius * cos(M_PI/6);
                shootingBall -> ball[5] = targetBall;
                targetBall -> ball[2] = shootingBall;
            }
            else {
                shootingBall->x = targetBall->x - ballRadius;
                shootingBall->y = targetBall->y + 2 * ballRadius * cos(M_PI / 6);
                shootingBall->ball[0] = targetBall;
                targetBall->ball[3] = shootingBall;
            }
        } else if ( differenceX < 0 ){
            if ( abs(differenceY) < ballRadius * sin(M_PI/6) ){
                if ( targetBall -> ball[1] ){
                    shootingBall -> x = target -> value -> x + ballRadius;
                    shootingBall -> y = target -> value -> y + 2 * ballRadius * cos(M_PI/6);
                    shootingBall -> ball[5] = targetBall;
                    targetBall -> ball[2] = shootingBall;
                }
                else {
                    shootingBall -> x = targetBall -> x + 2 * ballRadius;
                    shootingBall -> y = targetBall -> y ;
                    shootingBall -> ball[4] = targetBall;
                    targetBall -> ball[1] = shootingBall;
                }
            } else if ( differenceY >= ballRadius * sin(M_PI/6) && differenceY <= ballRadius) {
                if (targetBall -> ball[0]){
                    shootingBall -> x = targetBall -> x + 2 * ballRadius;
                    shootingBall -> y = targetBall -> y ;
                    shootingBall -> ball[4] = targetBall;
                    targetBall -> ball[1] = shootingBall;
                }
                else {
                    shootingBall->x = targetBall->x + ballRadius;
                    shootingBall->y = targetBall->y - 2 * ballRadius * cos(M_PI / 6);
                    shootingBall->ball[3] = targetBall;
                    targetBall->ball[0] = shootingBall;
                }
            } else if ( differenceY <= -1 * ballRadius * sin(M_PI/6) && differenceY >= -ballRadius) {
                ///////////////////////////////////////////////////////////////
                shootingBall -> x = target -> value -> x + ballRadius;
                shootingBall -> y = target -> value -> y + 2 * ballRadius * cos(M_PI/6);
                shootingBall -> ball[5] = targetBall;
                targetBall -> ball[2] = shootingBall;
            }
        }
    }
    else{
        if ( differenceX < 0 && target -> value -> x  != ( WIDTH - ballRadius )  && target -> value -> x != ballRadius ){
            if ( targetBall -> ball[2]){
                shootingBall -> x = targetBall -> x - ballRadius;
                shootingBall -> y = targetBall -> y + 2 * ballRadius * cos(M_PI/6);
                shootingBall -> ball[0] = targetBall;
                targetBall -> ball[3] = shootingBall;
            }
            else {
                shootingBall->x = targetBall->x + ballRadius;
                shootingBall->y = targetBall->y + 2 * ballRadius * cos(M_PI / 6);
                shootingBall->ball[5] = targetBall;
                targetBall->ball[2] = shootingBall;
            }
        } else if ( differenceX > 0 ){
            if ( abs(differenceY) < ballRadius * sin(M_PI/6) ){
                if ( targetBall->ball[4] ){
                    shootingBall -> x = targetBall -> x - ballRadius;
                    shootingBall -> y = targetBall -> y + 2 * ballRadius * cos(M_PI/6);
                    shootingBall -> ball[0] = targetBall;
                    targetBall -> ball[3] = shootingBall;
                }
                else{
                    shootingBall -> x = targetBall -> x - 2 * ballRadius;
                    shootingBall -> y = targetBall -> y ;
                    shootingBall -> ball[1] = targetBall;
                    targetBall -> ball[4] = shootingBall;
                }
            } else if ( differenceY >= ballRadius * sin(M_PI/6) && differenceY <= ballRadius) {
                if ( targetBall -> ball[5] ){
                    shootingBall -> x = targetBall -> x - 2 * ballRadius;
                    shootingBall -> y = targetBall -> y ;
                    shootingBall -> ball[1] = targetBall;
                    targetBall -> ball[4] = shootingBall;
                }
                else {
                    shootingBall->x = targetBall->x - ballRadius;
                    shootingBall->y = targetBall->y - 2 * ballRadius * cos(M_PI / 6);
                    shootingBall->ball[2] = targetBall;
                    targetBall->ball[5] = shootingBall;
                }
            } else if ( differenceY <= -1 * ballRadius * sin(M_PI/6) && differenceY >= -ballRadius) {
                shootingBall -> x = targetBall -> x - ballRadius;
                shootingBall -> y = targetBall -> y + 2 * ballRadius * cos(M_PI/6);
                shootingBall -> ball[0] = targetBall;
                targetBall -> ball[3] = shootingBall;
            }
        }
    }
    static Mix_Music *impactMusic = Mix_LoadMUS("..\\Sounds\\impact.mp3");
    if (!impactMusic) {
        exit(3);
    }
    Mix_PlayMusic(impactMusic, 1);
    allBalls = AddtoLinkedList(shootingBall, allBalls);
    visibleBalls = AddtoLinkedList(shootingBall, visibleBalls);
    shootingBallAroundBalls();
    int sameColorNumber = 0;
    Node * sameColor = findSameColorBall(shootingBall, &sameColorNumber);
    if ( sameColorNumber >= 3 ) {
        deleteBalls(sameColor);
        findFloatingBalls();
    }
    if ( shootingBall->isBomb){
        static Mix_Music *music = Mix_LoadMUS("..\\Sounds\\bomb.wav");
        if (!music) {
            exit(3);
        }
        Node * bombDestruction = bombRadius();
        if ( bombDestruction)
            deleteBalls(bombDestruction);
        Mix_PlayMusic(music, 1);
    }else if (shootingBall->isMultiColor){
        Node * multiDestruct;

        for ( int i = 0 ; i < 6 ; i++ ) {
            if ( shootingBall->ball[i] ){
                multiDestruct = findSameColorBall(shootingBall->ball[i], &sameColorNumber);
            }
            deleteBalls(multiDestruct);
        }
    }
}
Node * bombRadius(){
    Node * ball = allBalls;
    while ( ball != nullptr ){
        ball->value->isSeen = false;
        ball= ball->next;
    }
    Node * result = nullptr;
    for ( int i = 0 ; i < 6 ; i++ ){
        if ( shootingBall -> ball[i]){
            result = AddtoLinkedList(shootingBall->ball[i], result);
            shootingBall->ball[i]->isSeen = true;
            for (int j = 0; j < 6; j++) {
                if (shootingBall->ball[i]->ball[j]) {
                    result = AddtoLinkedList(shootingBall->ball[i]->ball[j], result);
                    shootingBall->ball[i]->ball[j]->isSeen = true;
                }
            }
        }
    }
    return result;
}
Node * findFloatingBalls(){
    Node * ball = allBalls;
    while ( ball != nullptr ) {
        ball->value->isSeen = false;
        ball = ball-> next;
    }
    Node * floatingBalls = nullptr;
    Node * temp;
    Ball * target;
    for ( int rowOneIndex = 0 ; rowOneIndex < (  WIDTH / ( 2 * ballRadius ) ) ; rowOneIndex++){
        if ( FirstRow[rowOneIndex] == nullptr ) continue;
        bfsQueue = nullptr;
        bfsQueue = AddtoLinkedList(FirstRow[rowOneIndex], bfsQueue);
        FirstRow[rowOneIndex] -> isSeen = true;
        target = FirstRow[rowOneIndex];

        while ( bfsQueue != nullptr ){
            for ( int i = 0 ; i < 6 ; i++ ) {
                if ( target -> ball[i] != nullptr ) {
                    if (!target->ball[i]->isSeen){
                        bfsQueue = AddtoLinkedList(target->ball[i], bfsQueue);
                        target->ball[i]->isSeen = true;
                    }
                }
            }
            temp = bfsQueue;
            bfsQueue = bfsQueue -> next;
            free(temp);
            if (bfsQueue != nullptr)
                target   = bfsQueue -> value;

        }
    }
    ball = allBalls;
    while(ball != nullptr){
        if ( !ball-> value-> isSeen ){
            floatingBalls = AddtoLinkedList(ball->value, floatingBalls);
        }
        ball = ball -> next;
    }
    if(floatingBalls)
        deleteBalls(floatingBalls);
    return floatingBalls;
}
void deleteBalls(Node * sameColor){
    if ( sameColor == nullptr ) return;
    Node * root = sameColor, * temp;
    while ( root != nullptr ){
        allBalls = deleteFromLinkedList(root->value, allBalls);
        for ( int ballAdj = 0 ; ballAdj < 6 ; ballAdj++ ){
            if ( root -> value -> ball[ballAdj] != nullptr) {
                root->value->ball[ballAdj]->ball[(ballAdj + 3) % 6] = nullptr;
                root->value->ball[ballAdj] = nullptr;
            }
        }
        temp = root;
        root = root -> next;
        free(temp->value);
        free(temp);
    }
    Node *ball = allBalls;
    while ( ball != nullptr ) {
        ball -> value -> y += impactUp;
        ball = ball -> next;
    }
}
void shootingBallAroundBalls(){
    Node * root = allBalls;
    int differenceX, differenceY;
    Ball * targetBall;
    while ( root != nullptr){
        differenceX =  shootingBall -> x - root -> value -> x;
        differenceY =  shootingBall -> y - root -> value -> y;
        targetBall  = root -> value;
        if ( abs(differenceY) <= 1 ){

            if ( differenceX < 3*ballRadius && differenceX > ballRadius ){
                shootingBall -> ball[4] = targetBall;
                targetBall -> ball[1] = shootingBall;
            }else if ( differenceX < -ballRadius && differenceX > -3 * ballRadius){
                shootingBall -> ball[1] = targetBall;
                targetBall -> ball[4] = shootingBall;
            }
        }else if ( differenceY <= (ballRadius * 2  ) && differenceY >= ( ballRadius )){

            if ( differenceX >= 0 && differenceX <= ballRadius){
                shootingBall -> ball[5] = targetBall;
                targetBall -> ball[2] = shootingBall;
            }else if ( differenceX <= 0 && differenceX >= -ballRadius ){
                shootingBall -> ball[0] = targetBall;
                targetBall -> ball[3] = shootingBall;
            }
        }else if( differenceY >= (-2 * ballRadius) && differenceY <= ( -1 * ballRadius )){

            if ( differenceX >= 0 && differenceX <= ballRadius){
                shootingBall -> ball[2] = targetBall;
                targetBall -> ball[5] = shootingBall;
            }else if ( differenceX <= 0 && differenceX >= -ballRadius ){
                shootingBall -> ball[3] = targetBall;
                targetBall -> ball[0] = shootingBall;
            }
        }

        root = root -> next;
    }


}
Node *  findSameColorBall(Ball* target, int *Count){
    Node * ball = allBalls;
    while( ball != nullptr ) {
        ball->value->isSeen = false;
        ball = ball->next;
    }
    Node * sameColor = nullptr, *temp;
    bfsQueue = nullptr;
    bfsQueue = AddtoLinkedList(target, bfsQueue);
    target -> isSeen = true;
    *Count = 1;
    while (bfsQueue != nullptr){
        for ( int i = 0 ; i < 6 ; i++){
            if(target->ball[i] != nullptr ) {
                if ( target -> ball[i] -> texture == target -> texture && !target -> ball[i] -> isSeen) {
                    bfsQueue = AddtoLinkedList(target->ball[i], bfsQueue);
                    target->ball[i]->isSeen = true;
                    ((*Count)) ++;
                }
            }
        }
        temp = bfsQueue;
        bfsQueue = bfsQueue -> next;
        free(temp);
        sameColor = AddtoLinkedList(target, sameColor);
        if (bfsQueue != nullptr)
             target   = bfsQueue -> value;

    }

    return sameColor;

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
    if(slope >  1000) slope = 0.001;
    if(slope < -1000) slope = 0.001;

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
        ballTemp->raduis = ballRadius;
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
        x+=2*ballRadius;
    }



}
void drawBalls(SDL_Renderer *renderer){
    Node * root = allBalls;
    Ball * currentBall;
    SDL_Rect texture;
    existingColor.clear();
    Color black = {0,0,0,255};
    Color white = {255, 255, 255, 255};
    if ( BombNumber != 0){
        existingColor[bomb] = &black;
    }
    existingColor[multi] = &white;
    while( root != nullptr ){
        currentBall = root -> value;
        texture = {static_cast<int>(currentBall->x - currentBall->raduis), static_cast<int>(currentBall->y - currentBall->raduis), static_cast<int>(2 * currentBall->raduis), static_cast<int>(2 * currentBall->raduis)};
        SDL_RenderCopy(renderer, currentBall->texture, nullptr, &texture);
        root -> value -> y += downSpeed;
        if ( root -> value -> texture != textures[0] )
            existingColor[root->value->texture] = &root->value->color;
        if ( root->value->y  + ballRadius > HEIGHT - 130) lostMuch = true;
        root = root -> next;
    }
    Node * floatBalls = findFloatingBalls();
//    if (floatBalls!= nullptr) deleteBalls(floatBalls);
}
void SDLInitialization(SDL_Renderer ** renderer, SDL_Window ** window, TTF_Font** gFont){
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        exit(1);
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        SDL_Quit();
        exit(1);
    }
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        exit(3);
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
    *gFont = TTF_OpenFont("..\\Font\\TechnoRaceItalic-eZRWe.otf", FONT_SIZE);
    if (gFont == nullptr) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        exit(3);
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
    int index = rand()%existingColor.size();
    int i = 0;
    SDL_Texture * properTexture;
    for (std::pair<SDL_Texture *const, Color *> pair : existingColor){
        properTexture = pair.first;
        if( i == index ) break;
        i++;
    }

    shootingBall = new Ball;
    shootingBall -> x = WIDTH/2;
    shootingBall -> y = HEIGHT - ballRadius;
    shootingBall ->color = *existingColor[properTexture];
    shootingBall -> raduis = ballRadius;
    shootingBall -> isBlack = false;
    shootingBall -> texture = properTexture;
    if ( properTexture == bomb ) {
        shootingBall->isBomb = true;
        BombNumber--;
    }else if (properTexture == multi){
        shootingBall->isMultiColor = true;
        MultiNumber--;
    }

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
    if(slope >  1000) slope = 0.001;
    if(slope < -1000) slope = 0.001;

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
            SDL_RenderCopy(renderer, backGround[backGroundIndex], nullptr, &back);

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
        temp -> raduis = ballRadius;
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
        x += 2*ballRadius;
        first = second;
        if( second != nullptr)
            second = second -> ball[1];

    }
    if(!isEven){
        Ball * temp = new Ball;
        temp -> x = x;
        temp -> y = y ;
        temp -> raduis = ballRadius;
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
    FirstRowY = -ballRadius*sqrt(3)*(NUMROWS - 1 - 5);
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
    SDL_Texture* background1 = loadTexture(renderer, "..\\Game Background\\GameBack.jpg");
    if (background1 == nullptr){
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(1);
    }     SDL_Texture* background2 = loadTexture(renderer, "..\\Game Background\\GameBack2.jpg");
    if (background2 == nullptr){
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(1);
    }    SDL_Texture* background3 = loadTexture(renderer, "..\\Game Background\\GameBack3.png");
    if (background3 == nullptr){
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(1);
    }    SDL_Texture* background4 = loadTexture(renderer, "..\\Game Background\\GameBack4.png");
    if (background4 == nullptr){
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(1);
    }
    backGround[0] = background1;
    backGround[1] = background2;
    backGround[2] = background3;
    backGround[3] = background4;
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
    SDL_Texture* blackBall = loadTexture(renderer, "..\\Balls\\BackBall.png");
    if (blackBall == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(1);
    }    SDL_Texture* blueBall = loadTexture(renderer, "..\\Balls\\BlueBall.png");
    if (blueBall == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(2);
    }    SDL_Texture* brownBall = loadTexture(renderer, "..\\Balls\\BrownBall.png");
    if (brownBall == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(3);
    }    SDL_Texture* greenBall = loadTexture(renderer, "..\\Balls\\GreenBall.png");
    if (greenBall == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(4);
    }    SDL_Texture* pinkBall = loadTexture(renderer, "..\\Balls\\pinkBall.png");
    if (pinkBall == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(5);
    }SDL_Texture* purpleBall = loadTexture(renderer, "..\\Balls\\purpleBall.png");
    if (purpleBall == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(6);
    }SDL_Texture* redBall = loadTexture(renderer, "..\\Balls\\RedBall.png");
    if (redBall == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(7);
    }SDL_Texture* yellowBall = loadTexture(renderer, "..\\Balls\\yellowBall.png");
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

    bomb = loadTexture(renderer, "..\\Balls\\bomb.png");
    if (bomb == nullptr){
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(8);
    }
    multi = loadTexture(renderer, "..\\Balls\\MultiColor.png");
    if (bomb == nullptr){
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        exit(8);
    }
}
void renderText(SDL_Renderer* renderer, TTF_Font* gFont, const std::string& text, int x, int y) {
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, text.c_str(), textColor);
    if (textSurface == nullptr) {
        std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
    } else {
        gTextTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if (gTextTexture == nullptr) {
            std::cerr << "Unable to create texture from rendered text! SDL_Error: " << SDL_GetError() << std::endl;
        } else {
            SDL_Rect renderQuad = {x, y, textSurface->w, textSurface->h};
            SDL_RenderCopy(renderer, gTextTexture, nullptr, &renderQuad);
        }
        SDL_FreeSurface(textSurface);
    }
}
//
//while( ball != nullptr ) {
//ball->value->isSeen = false;
//ball = ball->next;
//}
//for ( int i_firstBall_i = 0 ; i_firstBall_i < WIDTH / ( 2 * ballRadius ) + 1; i_firstBall_i++){
//
//}

//while (!quit) {
//// Handle events
//while (SDL_PollEvent(&event)) {
//if (event.type == SDL_QUIT) {
//quit = true;
//exit(2);
//}else if(event.type == SDL_MOUSEBUTTONDOWN) {
//if (event.button.button == SDL_BUTTON_LEFT) {
//SDL_GetMouseState(&mouse_x, &mouse_y);
//movingShootingBall(renderer, shootedCrossBow,  mouse_x, mouse_y);
//}
//}else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE) {
//quit = true;
//exit(2);
//}
//}
//SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
//SDL_RenderClear(renderer);
//SDL_RenderCopy(renderer, backGround, nullptr, &back);

////SDL_Delay(10);
//SDL_RenderPresent(renderer);
//}