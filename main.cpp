//Definitions
#define SDL_MAIN_HANDLED
#define fpscap 60
//Include Libraries
#include <iostream>
#include <SDL.h>
#include <RenderImage.h>
#include <RenderText.h>
#include <Button.h>
#include <unordered_map>
#include <array>
#include <random>
//Namespaces
using namespace std;
using namespace SDL_Render_Image;
using namespace T;
using namespace button;
//Global Variables
char* font_path = "C:/Users/studentadmin/CLionProjects/SDL2TEST/font.ttf";
int mousex;
int mousey;

// FUnctions
// Checks if a specific key is pressed
bool checkKey(char* key, SDL_Event event)
{
    if (event.type == SDL_KEYDOWN)
    {
        SDL_Keycode pressed = event.key.keysym.sym;
        const char* keyStr = SDL_GetKeyName(pressed);
        bool return_val = !strcmp(keyStr, key);
        return return_val;
    }
    return false;
}
//Updates mouse position
void updatemousepos(SDL_Event event)
{
    if (event.type == SDL_MOUSEMOTION)
    {
        mousex = event.motion.x;
        mousey = event.motion.y;
    }
}
int main(int argc, char* argv[])
{
    //Variables (Local)
    unordered_map<int,array<int,2>> npcpos;
    unordered_map<int, unordered_map<int, array<int, 4>>> spritePos;
    int direction = 3; //Forward:0, Left:1,Right:2,Back:3
    int animationnum = 1;
    int prevdirection = NULL;
    bool moving = false;
    int playerx =10;
    int playery = 10;
    int movementcycles = 0;
    int maxnpc = 3;
    random_device rd;
    mt19937 gen(rd());
    discrete_distribution<> npcdistribuition {99.999 ,0.001};
    uniform_int_distribution<> npccoordsgen;
    int npccount = 0;
    char* mode = "home";
    //Init sprite animations and positions
    //Front
    spritePos[0][1] ={0,0,16,24};
    spritePos[0][2] ={16,0,16,24};
    spritePos[0][3] ={32,0,16,24};
    spritePos[0][4] ={48,0,16,24};
    //Left
    spritePos[1][1] ={0,24,16,24};
    spritePos[1][2] ={16,24,16,24};
    spritePos[1][3] ={32,24,16,24};
    spritePos[1][4] ={48,24,16,24};
    //Right
    spritePos[2][1] ={0,48,16,24};
    spritePos[2][2] ={16,48,16,24};
    spritePos[2][3] ={32,48,16,24};
    spritePos[2][4] ={48,48,16,24};
    //Back
    spritePos[3][1] ={0,72,16,24};
    spritePos[3][2] ={16,72,16,24};
    spritePos[3][3] ={32,72,16,24};
    spritePos[3][4] ={48,72,16,24};

    //SDL INIT
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG|IMG_INIT_JPG|IMG_INIT_WEBP);
    SDL_Window* window = SDL_CreateWindow("Game",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,400,400,SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Event event;
    //SELF DEV LIB INIT
    Button EnterGame(renderer,"button.png",100,100,100,100);
    RenderImage render_image;
    RenderText rendertext;
    SDL_RendererFlip spriteflip = SDL_FLIP_NONE;
    SDL_Texture* npc = render_image.createTexture(renderer,"C:/Users/studentadmin/CLionProjects/SDL2TEST/image.png");
    SDL_Texture* player = render_image.createTexture(renderer,"C:/Users/studentadmin/CLionProjects/SDL2TEST/player_updated.png");
    uint32_t frameStart = SDL_GetTicks();
    uint32_t frameCount = 0;
    //Home Screen
    while (mode == "home")
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                SDL_Quit();
            }

        }
        mode = "game";
    }
    //In Game
    while (true)
    {
        // Generate NPC
        if (npcdistribuition(gen))
        {
            npcpos[npccount] = {npccoordsgen(gen)%401,npccoordsgen(gen)%401};
            npccount++;
            cout << "NPC Number " << npccount << " generated at ("
     << npcpos[npccount-1][0] << ", "
     << npcpos[npccount-1][1] << ")" << endl;


        }
        while (SDL_PollEvent(&event))
        {
            //Handle Movement Keys
            if (event.type == SDL_KEYDOWN)
            {
                if (checkKey("Left", event) || checkKey("A", event))
                {
                    direction = 1;
                    playerx -=playerx >15?7:0;

                }else if (checkKey("Right", event)|| checkKey("D", event))
                {
                    direction = 2;
                    playerx +=playerx<330?7:0;
                }else if (checkKey("Up", event) || checkKey("W", event))
                {
                    direction = 3;
                    playery +=playery<350?7:0;
                }else if (checkKey("Down", event) || checkKey("S", event))
                {
                    direction = 0;
                    playery -=playery>15?7:0;
                }

                moving =true;
            }
            // Handle Quiting
            if (event.type == SDL_QUIT)
            {
                SDL_Quit();
                return 0;
            }
        }
        //Handle Movement animations
        if (moving && movementcycles % 150 == 0)
        {
            moving = false;
            if (direction == prevdirection)
            {
                animationnum++;
                if (animationnum > 4)
                {
                    animationnum = 1;
                }
            }
            else
            {
                animationnum = 1;
                prevdirection = direction;
            }
        }
        array<int ,4> shown =spritePos[direction][animationnum];
        //Show All Textures
        SDL_SetRenderDrawColor(renderer, 255, 120, 0, 255);
        SDL_RenderClear(renderer);

        render_image.showImage(renderer,player,playerx,400-playery-50,50,50,shown[0],shown[1],shown[2],shown[3],spriteflip);
        for (auto i:npcpos)
                {
                    render_image.showImage(renderer,npc,i.second[0],i.second[1],50,50,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
                }
        SDL_RenderPresent(renderer);
        //Limit Fps
        uint32_t frameTimeEnd = SDL_GetTicks();
        uint32_t frameDuration = frameTimeEnd - frameStart;
        if (frameDuration < (1000/fpscap))
        {
            SDL_Delay((1000/fpscap) - frameDuration);
        }
        //Increment per frame
        frameCount++;
        movementcycles++;

    }
}
