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
#include <chrono>
#include <thread>
#include <filesystem>
#include <set>
#include <vector>
//Namespaces
using namespace std;
using namespace SDL_Render_Image;
using namespace T;
using namespace button;
using namespace std::chrono;
using namespace std::this_thread;
using namespace std::filesystem;
//Global Variables
char* font_path = "../font.ttf";
int mousex;
int mousey;

// Functions
// Checks if a specific key is pressed
struct NPC
{
    int x;
    int y;
    steady_clock::time_point spawntime;
};
void showLoading(SDL_Renderer* renderer,SDL_Event event, char* loadtext,char* fontpath, auto textrenderer,int duration,auto textures)
{
    auto begin = high_resolution_clock::now();
    SDL_Texture* done = textrenderer.CreateText(renderer,fontpath,"Done!",SDL_Color{255,255,255,255},1000);
    SDL_Texture* text = textrenderer.CreateText(renderer,fontpath,loadtext,SDL_Color{255,255,255,255},1000);
    while (true)
    {
        auto elapsed = duration_cast<milliseconds>(high_resolution_clock::now() - begin).count();
        if (elapsed>=duration) {break;}
        int x = (600 - 200) / 2;
        int y = 40;
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        textrenderer.DrawText(renderer,text,x,y,225,70);
        SDL_RenderPresent(renderer);
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                SDL_DestroyTexture(text);
                SDL_DestroyRenderer(renderer);
                SDL_Quit();
                exit(0);
            }
        }
    }
    begin = high_resolution_clock::now();
    while (true)
    {
        auto elapsed = duration_cast<milliseconds>(high_resolution_clock::now() - begin).count();
        if (elapsed>=300)break;
        SDL_RenderClear(renderer);
        textrenderer.DrawText(renderer,done,240,40,130,70);
        SDL_RenderPresent(renderer);
    }
    SDL_DestroyTexture(done);
    SDL_DestroyTexture(text);

}
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
//Check for collision
bool checkCollision(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2){

    return !(x1 + width1 <= x2 || x2 + width2 <= x1 || y1 + height1 <= y2 || y2 + height2 <= y1);
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
/*
bool playerNearNpc(int playerx,int playery, auto near)
{
    cout << "Player Position: (" << playerx << ", " << playery << ")" << endl;
    for (auto i:near)
    {
        if (i.first == playerx && i.second == playery)
        {
            cout << "det" << endl;
            return true;
        }
    }
    return false;
}
*/

bool playerNearNpc(int playerx, int playery, int npcx, int npcy, int dist)
{
    auto ydist = (playery - npcy)* (playery-npcy);
    auto xdist = (playerx - npcx) * (playerx - npcx);
    return xdist + ydist < dist*dist;
}

/*
//Check If Player is close to a npc, this is kind of hard to understand so step by step comments will be provided
bool playerNearNpc(int playerx, int playery, int npcx, int npcy)
{
    int gridsize = 25; //we draw invisible grids on the window
    set<pair<int,int>> npcgrids; // we store all places where player can be considered near to the npc in this array
    //we append all grids
    int originalgridnpcx = npcx/gridsize;
    int originalgridnpcy = npcy/gridsize;
    for (int x = 0; x < 4; x++)
    {
        for (int y = 0; y < 4; y++)
        {
            npcgrids.insert(make_pair(originalgridnpcx+x, originalgridnpcy+y));
        }
    }
    set<pair<int,int>>playergrids;
    //We do the same for the player
    int originalgridplayerx = playerx/gridsize;
    int originalgridplayery = playery/gridsize;
    for (int x=0; x<2;x++)
    {
        for (int y=0; y<2;y++)
        {
            playergrids.insert(make_pair(originalgridplayerx+x, originalgridplayery+y));
        }
    }
    for(auto i:npcgrids)
    {
        if (playergrids.find(i) != playergrids.end()) // for every grid that the player occupy, we check if it contains the same grid that the player occupy`
        {
            cout << "Detected" << endl;
            return true;
        }
    }
    return false;
}
*/
int main(int argc, char* argv[])
{
    //Variables (Local)
    unordered_map<int,NPC> npcpos;
    unordered_map<int, unordered_map<int, array<int, 4>>> spritePos;
    unordered_map<int, pair<int,int>> luckyblockpos;
    int direction = 3; //Forward:0, Left:1,Right:2,Back:3
    int animationnum = 1;
    int prevdirection = NULL;
    bool moving = false;
    int playerx =10;
    int playery = 10;
    int movementcycles = 0;
    int maxnpc = 1;
    int maxluckyblock = 10;
    int luckyblockcount = 0;
    bool trade = false;
    random_device rd;
    mt19937 gen(rd());
    discrete_distribution<> npcdistribuition {99.999 ,0.1};
    uniform_int_distribution<> npccoordsgen(30,370);
    discrete_distribution<> luckyblockgen{99.99,0.1};
    uniform_int_distribution<> luckyblockcoordgen{20,380};
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
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
    IMG_Init(IMG_INIT_PNG|IMG_INIT_JPG|IMG_INIT_WEBP);
    SDL_Window* window = SDL_CreateWindow("Game",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,600,600,SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Event event;
    //SELF DEV LIB INIT
    Button EnterGame(renderer,"../enter.png",190,300,200,100);
    Button ExitGame(renderer,  "../quit.png",190,430,200,100);
    RenderImage render_image;
    RenderText rendertext;
    TTF_Init();
    SDL_RendererFlip spriteflip = SDL_FLIP_NONE;
    SDL_Texture* background = render_image.createTexture(renderer, "../Grassfield.png");
    SDL_Texture* npc = render_image.createTexture(renderer,"../image.png");
    SDL_Texture* player = render_image.createTexture(renderer,"../player_updated.png");
    SDL_Texture* luckyblock = render_image.createTexture(renderer,"../image.png");
    SDL_Texture* tradetexture = rendertext.CreateText(renderer, font_path,"Press e to trade",SDL_Color{255,255,255,255},500);
    uint32_t frameStart = SDL_GetTicks();
    uint32_t frameCount = 0;
    //Home Screen
    steady_clock::time_point EnterclickTime;
    steady_clock::time_point ExitclickTime;
    while (mode == "home")
    {
        bool Enterclick = false;
        bool Exitclick = false;
        while (SDL_PollEvent(&event))
        {

            updatemousepos(event);
            if (event.type == SDL_QUIT)
            {
                SDL_Quit();
                return 0;
            }
            if (EnterGame.checkClick(event,mousex,mousey))
            {
                EnterclickTime = steady_clock::now();
                Enterclick = true;
            }
            if (ExitGame.checkClick(event,mousex,mousey))
            {
                ExitclickTime = steady_clock::now();
                Exitclick = true;
            }
        }


        SDL_RenderClear(renderer);
        // Handling Enter Game Click
        auto EntertimeElapsed = duration_cast<milliseconds>(steady_clock::now() - EnterclickTime).count();
        if (EntertimeElapsed >400 && EntertimeElapsed < 450)
        {
            showLoading(renderer,event,"Loading...",font_path,rendertext,500,nullptr);
            mode = "game";
        }
        if (Enterclick || EntertimeElapsed < 400)
        {
            EnterGame.RenderButton(renderer,128,0,64,32);
        }else if (EnterGame.checkHover(mousex, mousey))
        {
            EnterGame.RenderButton(renderer,64,0,64,32);
        }else
        {
            EnterGame.RenderButton(renderer, 0, 0, 64, 32);
        }
        //Handle Exit Click
        auto ExittimeElapsed = duration_cast<milliseconds>(steady_clock::now() - ExitclickTime).count();
        if (ExittimeElapsed >100 && ExittimeElapsed < 150)
        {
            Button::destroyAll();
            SDL_DestroyTexture(npc);
            SDL_DestroyTexture(player);
            SDL_DestroyWindow(window);
            SDL_DestroyRenderer(renderer);
            SDL_Quit();
            return 0;
        }
        if (Exitclick || ExittimeElapsed < 100)
        {
            ExitGame.RenderButton(renderer,128,0,64,32);
        }else if (ExitGame.checkHover(mousex, mousey))
        {
            ExitGame.RenderButton(renderer,64,0,64,32);
        }else
        {
            ExitGame.RenderButton(renderer, 0, 0, 64, 32);
        }
        //set background col
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        SDL_RenderPresent(renderer);
    }

        //In Game
        while (true)
        {
            // Generate NPC
            if (npcdistribuition(gen) &&  npccount < maxnpc)
            {
                auto generatex = npccoordsgen(gen);
                auto generatey = npccoordsgen(gen);
                npcpos[npccount] = NPC{generatex,generatey,steady_clock::now()};
                npccount++;
                cout << "NPC Number " << npccount << " generated at ("
         << npcpos[npccount-1].x << ", "
         << npcpos[npccount-1].y << ")" << endl;
            }
            //Generate Lucky blocks
            if (luckyblockgen(gen)&&luckyblockcount < maxluckyblock)
            {
                luckyblockpos[luckyblockcount] = make_pair<int,int>(luckyblockcoordgen(gen),luckyblockcoordgen(gen));
                luckyblockcount++;
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
                        playery -=7;
                    }else if (checkKey("Down", event) || checkKey("S", event))
                    {
                        direction = 0;
                        playery +=7;
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
            if (moving && movementcycles % 100 == 0)
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
            //Check if the player is near the npc
            for (auto i:npcpos)
            {
                if (playerNearNpc(playerx, playery, i.second.x,i.second.y,50))
                {
                    trade =true;
                    cout <<"det" << endl;
                }
            }
            for (auto i = luckyblockpos.begin();i!=luckyblockpos.end();)
            {
                if (playerNearNpc(playerx,playery,i->second.first,i->second.first,50))
                {
                    cout << "near" << endl;
                     luckyblockcount--;
                    i =luckyblockpos.erase(i);
                }else{
                    ++i;
                }
            }
            //Show All Textures
            SDL_SetRenderDrawColor(renderer, 196, 220, 195, 255);
            //Render Things below this code
            SDL_RenderClear(renderer);
            render_image.showImage(renderer,background,0,0,600,600,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
            if (trade)rendertext.DrawText(renderer,tradetexture,175,400,250,60);trade=false;
            render_image.showImage(renderer,player,playerx,playery,50,50,shown[0],shown[1],shown[2],shown[3],spriteflip);
            for (auto i = npcpos.begin(); i != npcpos.end();)
            {
                auto lifespanelapsed = duration_cast<seconds>(steady_clock::now() - i->second.spawntime).count();
                //Despawn npc after 30s
                if (lifespanelapsed > 10000)
                {
                    npccount--;
                    cout << "Despawned" << endl;
                    i = npcpos.erase(i);
                }else {
                    render_image.showImage(renderer,npc,i->second.x,i->second.y,50,50,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
                    ++i;
                }
            }
            for (auto i :luckyblockpos)render_image.showImage(renderer,luckyblock,i.second.first,i.second.second,20,20,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
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
