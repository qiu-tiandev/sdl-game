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
#include <map>
#include <algorithm>
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
char* sans_path = "../sans.ttf";
int mousex;
int mousey;
map<int, pair<int,int>>inventory; // We initialise the inventory in the form of:{Slot No.:item id,count}

// Functions
// Checks if a specific key is pressed
struct normalTrader
{
    int scamrate;
    vector<int> trades;//array of trade ids
};
struct NPC
{
    int x;
    int y;
    steady_clock::time_point spawntime;
    struct type;
};
struct Monster
{
    int x;
    int y;
    steady_clock::time_point spawntime;
    int damage;
    int speed;
    int despawntime;
};
void addInventory(int item,int count)
{
    auto found = false;
    for (auto& i:inventory)
    {
        if (i.second.first == item)
        {
            i.second.second+=count;
            cout << i.first << ": " << i.second.first <<", "<< i.second.second << endl;
            found = true;
            break;
        }
    }
    if (!found)
    {
        for (auto& i:inventory)
        {
            if (i.second.first == -1)
            {
                i.second.first = item;
                i.second.second = count;
                cout << i.first << ": " << i.second.first <<", "<< i.second.second << endl;
                break;
            }
        }
    }
}
int getTextHeight(char* text,int width)
{
    return width/0.8*strlen(text);
}
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
void moveMonster(Monster& monster,int playerx,int playery,mt19937 gen,discrete_distribution<>move)
{
    if (move(gen))
    {
        if (monster.x < playerx)
        {
            monster.x+=monster.speed;
        }else if (monster.x > playerx)
        {
            monster.x-=monster.speed;
        }
        if (monster.y < playery)
        {
            monster.y+=monster.speed;
        }else if (monster.y > playery)
        {
            monster.y-=monster.speed;
        }
    }
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
    unordered_map<int,array<int,4>> healthpos;
    unordered_map<char*,array<int,4>>potionpos;
    map<int,array<int,4>>selectedpos;
    unordered_map<char*,char*>decmsgs;
    array<array<int,2>,3>tradeguipos = {{{0,0},{0,96},{0,192}}};
    decmsgs["Tp"] = "Oh no! It looks like the lucky block\nwas placed by the monster!\n You have been teleported to the monster.";
    decmsgs["Speed"] = "Oh no! The lucky block\n was a evil lucky block!\n The lucky block gave the monster a speed increase\nfor 7s";
    decmsgs["Slow"] = "Oopsies! The lucky block\ndispersed mud! You have been\nslowed down for 5s";
    for (int i=1;i<10;i++)
    {
        inventory[i] = {-1,0};
        selectedpos[i] = {(i-1)*55+47,500,22,22};
    }
    unordered_map<int,char*>items;
    map<int,map<int,int>> trades; //store trades available here
    map<int,int> temp1;
    map<int,int> temp2;
    //TODO: Add trades into here
    map<int,SDL_Texture*> itemTextures; //store sdl textures of items inside this map
    vector<int> invkey = {1,2,3,4,5,6,7,8,9};
    items[0] = "a";items[1] ="b",items[2] ="c";items[3]="d";items[4] = "e";items[5]="f";items[6]="g";items[7]="h";
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
    bool tradeguiopen = false;
    bool luckyblockopened = false;
    int luckyblockcycle = 1;
    int obtained;
    bool displayObtained = false;
    int healthstate = 1;
    int damagecooldown = 400;
    int selectedSlot = 1;
    int playerSpeed = 7;
    char buffer[2];
    bool slownessactive = false;
    bool speedactive = false;
    char buffer2[3];
    bool decept = false;
    int storyopacity = 0;
    bool luckydec =false;
    char* luckydectp;
    bool slowplayer = false;
    bool fastmonster = false;
    bool applyluckydeceff = false;
    int tradeguistatus = 0;
    bool tradeselected = false;;
    random_device rd;
    mt19937 gen(rd());
    discrete_distribution<> npcdistribuition {99.999 ,0.1};
    uniform_int_distribution<> npccoordsgen(30,370);
    discrete_distribution<> luckyblockgen{99.99,0.1};
    uniform_int_distribution<> luckyblockcoordgen{20,380};
    uniform_int_distribution<> number{0,1};
    discrete_distribution<>move{98,2};
    vector<int> traderTypepos = {43,15,25,17};
    discrete_distribution<> luckyblockdecept{0,100};
    discrete_distribution<> traderTypegen(traderTypepos.begin(),traderTypepos.end());
    array<char*,9>luckydecconsq = {"Tp","Speed","Slow","Damage","Death","Monster","Steal","Clear","MulSteal"};
    discrete_distribution<>luckydeccongen{30,60,60,60,10,30,60,60,30};
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
    for (int i = 0;i<7;i++)healthpos[i+1] = {0,8*i,20,8};
    potionpos["Speed"] = {0,0,32,32};
    potionpos["Slow"] = {0,32,32,32};
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
    SDL_Texture* menu = render_image.createTexture(renderer,"../gui.png");
    SDL_Texture* luckyblockmsg = rendertext.CreateText(renderer,font_path,"You got: ",SDL_Color{255,255,255,255},500);
    SDL_Texture* luckyblockresult;
    SDL_Texture* menubackground = render_image.createTexture(renderer,"../black.png");
    SDL_Texture* monster = render_image.createTexture(renderer,"../image.png");
    SDL_Texture* health = render_image.createTexture(renderer,"../health.png");
    SDL_Texture* inventorypng = render_image.createTexture(renderer,"../inventory.png");
    SDL_Texture* selected = render_image.createTexture(renderer,"../selected.png");
    SDL_Texture* potions = render_image.createTexture(renderer,"../Potions.png");
    SDL_Texture* effectsText = rendertext.CreateText(renderer,font_path,"Effects:", SDL_Color{65,105,225,255},100);
    SDL_Texture* invcount;
    SDL_Texture* predecept;
    SDL_Texture* luckyblockresulttitle = rendertext.CreateText(renderer,"../px.ttf","Lucky Block Result",SDL_Color{77,64,52,255},16);
    SDL_Texture* postdecept;
    SDL_Texture* tradeui = render_image.createTexture(renderer,"../trade.png");
    SDL_SetTextureAlphaMod(menubackground,175);
    uint32_t frameStart = SDL_GetTicks();
    uint32_t frameCount = 0;
    Monster monster1;
    monster1.damage = 100;
    monster1.x = 300;
    monster1.y = 300;
    monster1.spawntime = steady_clock::now();
    monster1.damage = 10;
    monster1.speed = 1;
    monster1.despawntime = 10000000;
    //Home Screen
    steady_clock::time_point EnterclickTime;
    steady_clock::time_point ExitclickTime;
    steady_clock::time_point luckyblockpickup;
    steady_clock::time_point displaytime;
    steady_clock::time_point lastDamage = steady_clock::now();
    steady_clock::time_point speedPotionStartTime;
    steady_clock::time_point slowPotionStartTime;
    steady_clock::time_point deceptionTextShow;
    steady_clock::time_point monsterSpeedGain;
    steady_clock::time_point playerSlow;
    steady_clock::time_point timenow;
    int elapsed;
    int elapsed2;
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
    itemTextures[0] = render_image.createTexture(renderer,"../potion.png");
    Button trade1(renderer,nullptr,162,205,270,64);
    Button trade2(renderer,nullptr,162,276,270,64);
    //In Game
    while (true)
    {
        timenow = steady_clock::now();
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
            updatemousepos(event);
            if (event.type == SDL_MOUSEMOTION) {
                // Check hover status for trade buttons
                if (tradeguiopen) {
                    if (trade1.checkHover(mousex, mousey)&& !tradeselected) {
                        cout << "a" << endl; // Hovering over trade1
                        tradeguistatus = 2;
                    }else if (trade2.checkHover(mousex, mousey) && !tradeselected) {
                        cout << "b" << endl; // Hovering over trade2
                        tradeguistatus = 1;
                    } else if (!tradeselected){
                        tradeguistatus = 0; // Not hovering over any trade button
                    }
                }
            }
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (trade1.checkClick(event,mousex,mousey))
                {
                    tradeguistatus = 2;
                    tradeselected = true;
                }else if (trade2.checkClick(event,mousex,mousey))
                {
                    tradeguistatus = 1;
                    tradeselected = true;
                }
            }
            //Handle Movement Keys
            if (event.type == SDL_KEYDOWN)
            {
                //Check if input is a number
                if (event.key.keysym.sym >= SDLK_1 && event.key.keysym.sym <= SDLK_9)
                {
                    selectedSlot = event.key.keysym.sym - SDLK_1 + 1;
                    cout << selectedSlot << endl;
                }
                if (checkKey("X",event)&&inventory[selectedSlot].first !=-1)
                {
                    if (inventory[selectedSlot].first == 0)
                    {
                        slownessactive = true;
                        move = discrete_distribution<>{99,1};
                        if (inventory[selectedSlot].second == 1)
                        {
                            inventory[selectedSlot].first = -1;
                            inventory[selectedSlot].second = 0;
                        }else
                        {
                            inventory[selectedSlot].second--;
                        }
                        slowPotionStartTime = steady_clock::now();
                    }else if (inventory[selectedSlot].first == 1)
                    {
                        speedactive = true;
                        playerSpeed = 10;
                        inventory[selectedSlot].first = -1;
                        speedPotionStartTime = steady_clock::now();
                        if (inventory[selectedSlot].second == 1)
                        {
                            inventory[selectedSlot].first = -1;
                            inventory[selectedSlot].second = 0;
                        }else
                        {
                            inventory[selectedSlot].second--;
                        }
                    }
                }
                if ((checkKey("Left", event) || checkKey("A", event)) && !tradeguiopen && !luckyblockopened)
                {
                    direction = 1;
                    playerx -=playerx >15?playerSpeed:0;

                }else if ((checkKey("Right", event)|| checkKey("D", event))&& !tradeguiopen && !luckyblockopened)
                {
                    direction = 2;
                    playerx +=playerx<330?playerSpeed:0;
                }else if ((checkKey("Up", event) || checkKey("W", event))&& !tradeguiopen&& !luckyblockopened)
                {
                    direction = 3;
                    playery -=playerSpeed;
                }else if ((checkKey("Down", event) || checkKey("S", event))&& !tradeguiopen&& !luckyblockopened)
                {
                    direction = 0;
                    playery +=playerSpeed;
                }else if (trade&&checkKey("E", event)&& !luckyblockopened)
                {
                    trade = false;
                    tradeguiopen = true;
                    cout << "Open" << endl;
                }else if (tradeguiopen && checkKey("Escape", event))
                {
                    tradeguiopen = false;
                    trade=true;
                    tradeselected =false;
                    tradeguistatus = 0;
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
        moveMonster(monster1,playerx,playery,gen,move);
        if (slowplayer&&duration_cast<milliseconds>(timenow - playerSlow).count() > 5000 && duration_cast<milliseconds>(timenow - playerSlow).count() < 5200)
        {
            playerSpeed+=3;
            slowplayer = false;
        }
        if (fastmonster&&duration_cast<milliseconds>(timenow - monsterSpeedGain).count() > 7000&&duration_cast<milliseconds>(timenow - monsterSpeedGain).count() <7200)
        {
            move = discrete_distribution<>{98,2};
            fastmonster = true;
        }
        if (playerNearNpc(playerx,playery,monster1.x,monster1.y,30))
        {
            if (duration_cast<milliseconds>(steady_clock::now() - lastDamage).count() >= damagecooldown)
            {
                healthstate%=7;
                healthstate++;
                lastDamage = steady_clock::now();
            }
        }
        if (speedactive)
        {
            elapsed = duration_cast<seconds>(steady_clock::now() - speedPotionStartTime).count();
            if (elapsed >= 3 && elapsed <=4)
            {
                speedactive = false;
                playerSpeed = 7;
            }
        }
        if (slownessactive)
        {
            elapsed2 = duration_cast<seconds>(steady_clock::now()-slowPotionStartTime).count();
            if (elapsed2 >=7 && elapsed2 <=8)
            {
                slownessactive = false;
                move = discrete_distribution<>{98,2};
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
            if (playerNearNpc(playerx, playery, i.second.x,i.second.y,50) && !tradeguiopen)
            {
                trade =true;
            }else
            {
                trade=false;
            }
        }
        for (auto i = luckyblockpos.begin();i!=luckyblockpos.end();)
        {
            if (checkCollision(playerx,playery,50,50,i->second.first,i->second.second,20,20)&& !luckyblockopened)
            {
                if (luckyblockresult)
                {
                    SDL_DestroyTexture(luckyblockresult);
                    luckyblockresult = nullptr;
                }
                luckyblockresult = rendertext.CreateText(renderer,font_path,items[number(gen)],SDL_Color{255,255,255,255},500);
                luckyblockcount--;
                decept = luckyblockdecept(gen);
                i =luckyblockpos.erase(i);
                luckyblockpickup = steady_clock::now();
                luckyblockopened = true;
                if (decept)
                {
                    luckydec = true;
                    luckydectp = luckydecconsq[luckydeccongen(gen)];
                    cout << luckydectp << endl;
                    decept = false;
                }else
                {
                    luckydectp = nullptr;
                }
            }else{
                ++i;
            }
        }
        //Show All Textures
        SDL_SetRenderDrawColor(renderer, 196, 220, 195, 255);
        //Render Things below this codee
        SDL_RenderClear(renderer);
        render_image.showImage(renderer,background,0,0,600,600,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
        if (trade)rendertext.DrawText(renderer,tradetexture,175,400,250,60);
        rendertext.DrawText(renderer,effectsText,505,10,60,30);
        if (slownessactive)
        {
            if (speedactive)
            {
                render_image.showImage(renderer,potions,535,32,32,32,potionpos["Slow"][0],potionpos["Slow"][1],potionpos["Slow"][2],potionpos["Slow"][3],SDL_FLIP_NONE); // Complete
            }else
            {
                render_image.showImage(renderer,potions,505,32,32,32,potionpos["Slow"][0],potionpos["Slow"][1],potionpos["Slow"][2],potionpos["Slow"][3],SDL_FLIP_NONE);
            }
        }
        if (speedactive)
        {
            render_image.showImage(renderer,potions,505,32,32,32,potionpos["Speed"][0],potionpos["Speed"][1],potionpos["Speed"][2],potionpos["Speed"][3],SDL_FLIP_NONE);
        }
        //Render the inventory slot
        render_image.showImage(renderer,inventorypng,50,500,500,60,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
        render_image.showImage(renderer,selected,selectedpos[selectedSlot][0],497,500,60,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
        //we render the items of the inventory
        for (auto i: inventory)
        {
            if (i.second.first !=-1)// check if slot is occupied
            {
                if (i.second.first == 0){//if item is potion
                    render_image.showImage(renderer,potions,50+(i.first-1)*55,500,55,55,potionpos["Slow"][0],potionpos["Slow"][1],potionpos["Slow"][2],potionpos["Slow"][3],SDL_FLIP_NONE);
                }else if (i.second.first == 1)
                {
                    render_image.showImage(renderer,potions,50+(i.first-1)*55,500,55,55,potionpos["Speed"][0],potionpos["Speed"][1],potionpos["Speed"][2],potionpos["Speed"][3],SDL_FLIP_NONE);
                }
            }
        }
            render_image.showImage(renderer,player,playerx,playery,50,50,shown[0],shown[1],shown[2],shown[3],spriteflip);
            render_image.showImage(renderer,health,playerx,playery-15,50,13,healthpos[healthstate][0],healthpos[healthstate][1],healthpos[healthstate][2],healthpos[healthstate][3],SDL_FLIP_NONE);
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
            render_image.showImage(renderer,monster,monster1.x,monster1.y,50,50,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
            for (auto i :luckyblockpos)render_image.showImage(renderer,luckyblock,i.second.first,i.second.second,20,20,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
            if (displayObtained && duration_cast<milliseconds>(steady_clock::now()-displaytime).count() <1600)
            {
                SDL_DestroyTexture(luckyblockresult);
                rendertext.DrawText(renderer,luckyblockmsg,150,20,200,50);
                luckyblockresult = rendertext.CreateText(renderer,font_path,items[obtained],SDL_Color{255,255,255,255},500);
                rendertext.DrawText(renderer,luckyblockresult,350,20,50,50);
                decept = luckyblockdecept(gen);
                if (decept)deceptionTextShow = steady_clock::now();
            }else if (duration_cast<milliseconds>(steady_clock::now()-displaytime).count() >1600 && displayObtained)
            {
                displayObtained = false;
                SDL_DestroyTexture(luckyblockresult);
            }
            if (luckyblockopened)
            {
                auto elapsed = duration_cast<milliseconds>(steady_clock::now() - luckyblockpickup).count();
                if (luckyblockcycle <  50 )
                {
                    render_image.showImage(renderer,menubackground,0,0,600,600,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
                    render_image.showImage(renderer,menu,125,200,340,200,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
                    rendertext.DrawText(renderer,luckyblockresulttitle,200,220,200,30);
                    rendertext.DrawText(renderer,luckyblockmsg,240,250,130,40);
                    if (elapsed > 50)
                    {
                        SDL_DestroyTexture(luckyblockresult);
                        luckyblockresult = nullptr;
                        obtained = number(gen);
                        luckyblockresult = rendertext.CreateText(renderer,font_path,items[obtained],SDL_Color{255,255,255,255},500);
                        luckyblockpickup = steady_clock::now();
                        luckyblockcycle++;
                    }
                    rendertext.DrawText(renderer,luckyblockresult,280,285,39,40);

                }else if (!luckydectp)
                {
                    SDL_DestroyTexture(luckyblockresult);
                    luckyblockresult = nullptr;
                    addInventory(obtained,1);
                    luckyblockcycle =1;
                    luckyblockopened = false;
                    displayObtained = true;
                    displaytime = steady_clock::now();
                }else
                {
                    luckyblockopened = false;
                }
            }
            if (duration_cast<milliseconds>(steady_clock::now()-deceptionTextShow).count() <=3000 && duration_cast<milliseconds>(steady_clock::now()-displaytime).count() >=500)
            {
                SDL_DestroyTexture(predecept);
                predecept = nullptr;
                if (storyopacity <255)storyopacity+=2;
                SDL_Texture* predecept = rendertext.CreateText(renderer,"../dec.ttf","Or did you?", SDL_Color{200,20,0,255},100);
                SDL_SetTextureAlphaMod(predecept,storyopacity);
                rendertext.DrawText(renderer,predecept,165,50,275,95);
            }else if (duration_cast<milliseconds>(steady_clock::now()-deceptionTextShow).count() >=3000 && duration_cast<milliseconds>(steady_clock::now()-deceptionTextShow).count()<=3200)
            {
                storyopacity = 0;
                SDL_DestroyTexture(predecept);

            }else if (duration_cast<milliseconds>(timenow-deceptionTextShow).count() >=3000 && duration_cast<milliseconds>(timenow-deceptionTextShow).count() <3200)
            {
                applyluckydeceff = true;
            }
            for (auto i:inventory)
            {
                if (i.second.first !=-1)
                {
                    if (invcount)
                    {
                        SDL_DestroyTexture(invcount);
                    }
                    sprintf(buffer2,"%d",i.second.second);
                    invcount = rendertext.CreateText(renderer,sans_path,buffer2,SDL_Color{255,255,255,255},50);
                    rendertext.DrawText(renderer,invcount,77+(i.first-1)*55,528,20,25);
                }
            }
            if (applyluckydeceff&&luckydec && duration_cast<milliseconds>(steady_clock::now()-deceptionTextShow).count() >=3000)
            {
                if (luckydectp == "Tp")
                {
                    playerx = monster1.x;
                    playery = monster1.y;
                    cout << "Player tp to monster" <<endl;
                }else if (luckydectp == "Speed")
                {
                    move = discrete_distribution<>{97,3};
                    monsterSpeedGain = steady_clock::now();
                    fastmonster = true;
                    cout << "Monster Speed increase" << endl;
                }else if (luckydectp == "Slow")
                {
                    playerSpeed -=3;
                    playerSlow = steady_clock::now();
                    slowplayer = true;
                    cout << "Player Speed decrease" << endl;
                }else if (luckydectp == "Clear")
                {

                    luckyblockpos.clear();
                    npcpos.clear();
                    cout << "All Npc and Luckyblocks cleared" << endl;
                }
                luckydectp = nullptr;
                luckydec = false;
            }
            //WARNING:The texture below should be the last thing that is rendered
            if (tradeguiopen)
            {
                render_image.showImage(renderer,menubackground,0,0,600,600,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
                render_image.showImage(renderer,menu,135,110,325,325,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
                render_image.showImage(renderer,tradeui,135,110,325,325,tradeguipos[tradeguistatus][0],tradeguipos[tradeguistatus][1],96,96,SDL_FLIP_NONE);
                //TODO: Render the internal parts of the menu(trade spaces, trade button, add. info)
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