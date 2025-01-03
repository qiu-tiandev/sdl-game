//Definitions
#define SDL_MAIN_HANDLED
#define fpscap 60
//Include Libraries
#include <iostream>
#include <SDL.h>
#include "RenderImage.h"
#include "RenderText.h"
#include "Button.h"
#include <unordered_map>
#include <array>
#include <random>
#include <chrono>
#include <thread>
#include <filesystem>
#include <map>
#include <algorithm>
#include <vector>
#include <SDL_mixer.h>
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
    vector<vector<int>>trades;
    bool scammer;
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
void stealTwoOfFirstItem(map<int, pair<int, int>>& inventory) {
    // Find the first item in the inventory that is not empty
    for (auto& slot : inventory) {
        if (slot.second.first != -1 && slot.second.second > 0) {
            // This is the first item to steal
            int itemToSteal = slot.second.first;

            // Remove two of that item from the inventory
            slot.second.second -= 2; // Remove two items
            if (slot.second.second <= 0) {
                slot.second.first = -1; // Mark slot as empty if count is zero
            }
            return;
        }
    }

    // If no items were found to steal
    cout << "No items to steal!" << endl;
}
void addInventory(int item,int count)
{
    auto found = false;
    for (auto& i:inventory)
    {
        if (i.second.first == item && i.second.second+count <=16)
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
array<int,2> queryTextDimensions(char* text, char* fontpath)
{
    int w,h;
    TTF_Font* font = TTF_OpenFont(fontpath, 16);
    SDL_Surface* surface = TTF_RenderText_Solid(font,text,SDL_Color{255,255,255});
    TTF_CloseFont(font);
    w = surface->w;
    h = surface->h;
    SDL_FreeSurface(surface);
    return {w,h};
}
bool hasRequiredItems(const map<int, pair<int, int>>& inventory, int itemID, int requiredCount) {
    for (const auto& slot : inventory) {
        if (slot.second.first == itemID && slot.second.second >= requiredCount) {
            return true; // Found the required item with enough count
        }
    }
    return false; // Required item not found
}
void showLoading(SDL_Renderer* renderer,SDL_Event event, char* loadtext,char* fontpath, RenderText textrenderer,int duration)
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
        cout << keyStr << endl;
        bool return_val = !strcmp(keyStr, key);
        return return_val;
    }
    return false;
}
void moveMonster(Monster& monster,int playerx,int playery,int& cycle,SDL_RendererFlip& flip, int reqcycle)
{
    if (cycle == reqcycle)
    {
        cycle =0;
        if (monster.x < playerx)
        {
            monster.x+=monster.speed;
            flip = SDL_FLIP_HORIZONTAL;

        }else if (monster.x > playerx)
        {
            monster.x-=monster.speed;
            flip = SDL_FLIP_NONE;
        }
        if (monster.y < playery)
        {
            monster.y+=monster.speed;
        }else if (monster.y > playery)
        {
            monster.y-=monster.speed;
        }
    }
    cycle++;
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
vector<int> generateLevel()
{
    random_device rd;
    mt19937 gen(rd());
    discrete_distribution<>leveltypegen{30,70};
    uniform_int_distribution<>survivaltime{50,70};
    uniform_int_distribution<>getitem{0,2};
    vector<int> ids = {2,5,6};
    discrete_distribution<>count{0,0,0,0,50,30,20};
    int leveltype = leveltypegen(gen);
    return {1,ids[getitem(gen)],count(gen)};
}
int checkLevelRequirements(const map<int, pair<int, int>>& inventory, const vector<int>& levelreq)
{
    int count = 0; // Counter for satisfied requirements

    // Check if the level requirement is valid
    if (levelreq.size() < 3) {
        cout << "Invalid level requirements!" << endl;
        return count;
    }

    int requiredItemID = levelreq[1]; // The item ID required
    int requiredCount = levelreq[2];   // The required count of that item

    // Iterate through the inventory to count the items
    for (const auto& slot : inventory) {
        if (slot.second.first == requiredItemID) {
            count += slot.second.second; // Add the count of this item to the total
        }
    }
    return count >= requiredCount;
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
    unordered_map<int,char*>itemidNames;
    map<int,SDL_Texture*>numbers;
    unordered_map<int,SDL_Texture*>levelnums;
    unordered_map<int,SDL_Texture*>itemNameTexture;
    itemidNames[0] = "Slowness Potion";
    itemidNames[1] = "Speed Potion";
    itemidNames[2] = "Diamond";
    itemidNames[3] = "Trash";
    itemidNames[4] = "Dirt ball";
    itemidNames[5] = "Gem";
    itemidNames[6] = "Money";
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
    map<int,vector<int>> trades; //store trades available here
    map<int,vector<int>>scamtrades;
    vector<vector<int>> tradebutpos;
    for (int i = 0; i<3;i++)
    {
        tradebutpos.push_back({80*i,0,80,19});
    }
    trades[1] = {3,12,5,1};
    trades[2] = {4,7,3,2};
    trades[3] = {5,3,2,1};
    trades[4] = {5,1,0,1};
    trades[5] = {5,1,1,1};
    trades[6] = {3,1,3,1};//troll trade
    trades[7] = {4,1,4,1};//troll trade
    trades[8] = {2,1,4,1};//troll trade
    trades[9] = {0,1,3,8};
    trades[10] = {1,1,3,8};
    trades[11] = {6,5,5,1};
    trades[12] = {6,5,1,1};
    trades[13] = {6,5,0,1};
    trades[14] = {4,6,5,1};
    scamtrades[1] = {3,6,0,1};
    scamtrades[2] = {3,9,5,1};
    scamtrades[3] = {5,2,2,1};
    scamtrades[4] = {4,5,3,2};
    scamtrades[5] = {0,1,3,10};
    scamtrades[6] = {1,1,3,10};
    scamtrades[7] = {3,8,0,1};
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
    int maxnpc = 3;
    int maxluckyblock = 7;
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
    bool tradeselected = false;
    bool deceptiontimerset = false;
    int currentNpcId = -1;
    bool npcnearfound = false;
    bool tradebuttonclick = false;
    bool handletrade = false;
    bool showTraderScamMsg = false;
    int scamdisopacity = 0;
    int monstermovementcycle = 0;
    int monstercyclereq = 30;
    int level = 1;
    bool obtainedgenerated = true;
    bool escfakedeath = false;
    bool canPlay = false;
    int movepx = 0;
    int trashmovepx = 0;
    bool showeffects = true;
    bool mutemusic = false;
    vector<int> tradeno1;
    vector<int> tradeno2;
    random_device rd;
    mt19937 gen(rd());
    discrete_distribution<> npcdistribuition {99.999, 0.03};
    uniform_int_distribution<> npccoordsgen(30,370);
    discrete_distribution<> luckyblockgen{99.99,0.01};
    uniform_int_distribution<> luckyblockcoordgen{20,380};
    uniform_int_distribution<> number{0,1};
    discrete_distribution<>move{98,2};
    vector<int> traderTypepos = {43,15,25,17};
    discrete_distribution<> luckyblockdecept{93,7};
    discrete_distribution<> traderTypegen(traderTypepos.begin(),traderTypepos.end());
    array<char*,9>luckydecconsq = {"Tp","Speed","Slow","Damage","Death","Monster","Steal","Clear","MulSteal"};
    discrete_distribution<>luckydeccongen{30,60,60,60,0,30,0,60,30};
    uniform_int_distribution<>tradegen{1,14};
    discrete_distribution<> scam{80,20};
    uniform_int_distribution<>scamtradesgen{1,7};
    uniform_int_distribution<>luckyblockpre{0,6};
    discrete_distribution<>luckyblockitemgen{53,20,9,6,6,4,2};
    discrete_distribution<>trashgetchance{65,35};
    uniform_int_distribution<>luckydecptmsgdist{0,2};
    int npccount = 0;
    char* mode = "tutorial";
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
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG|IMG_INIT_JPG|IMG_INIT_WEBP);
    SDL_Window* window = SDL_CreateWindow("Game",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,600,600,SDL_WINDOW_RESIZABLE|SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Event event;
    //SELF DEV LIB INIT
    Button EnterGame(renderer,"../enter.png",190,350,200,100);
    Button ExitGame(renderer,  "../quit.png",190,450,200,100);
    Button Info(renderer,"../Info.png",190,250,200,100);
    RenderImage render_image;
    RenderText rendertext;
    TTF_Init();
    SDL_Surface* icon = IMG_Load("../icon.png");
    SDL_SetWindowIcon(window,icon);
    SDL_RendererFlip spriteflip = SDL_FLIP_NONE;
    SDL_RendererFlip monstermoveflip = SDL_FLIP_NONE;
    SDL_Texture* background = render_image.createTexture(renderer, "../Grassfield.png");
    SDL_Texture* npc = render_image.createTexture(renderer,"../Trader.png");
    SDL_Texture* player = render_image.createTexture(renderer,"../player_updated.png");
    SDL_Texture* luckyblock = render_image.createTexture(renderer,"../Lucky_Block.png");
    SDL_Texture* tradetexture = rendertext.CreateText(renderer, font_path,"Press e to trade",SDL_Color{255,255,255,255},500);
    SDL_Texture* menu = render_image.createTexture(renderer,"../gui.png");
    SDL_Texture* luckyblockmsg = rendertext.CreateText(renderer,font_path,"You got: ",SDL_Color{255,255,255,255},500);
    SDL_Texture* luckyblockresult;
    SDL_Texture* menubackground = render_image.createTexture(renderer,"../black.png");
    SDL_Texture* monster = render_image.createTexture(renderer,"../Monster.png");
    SDL_Texture* health = render_image.createTexture(renderer,"../health.png");
    SDL_Texture* inventorypng = render_image.createTexture(renderer,"../inventory.png");
    SDL_Texture* selected = render_image.createTexture(renderer,"../selected.png");
    SDL_Texture* potions = render_image.createTexture(renderer,"../Potions.png");
    SDL_Texture* effectsText = rendertext.CreateText(renderer,font_path,"Effects:", SDL_Color{65,105,225,255},100);
    SDL_Texture* predecept;
    SDL_Texture* luckyblockresulttitle = rendertext.CreateText(renderer,"../px.ttf","Lucky Block Result",SDL_Color{77,64,52,255},16);
    SDL_Texture* postdecept;
    SDL_Texture* tradeui = render_image.createTexture(renderer,"../trade.png");
    SDL_Texture* showitemName;
    SDL_Texture* tradeno1count;
    SDL_Texture* tradeno2count;
    SDL_Texture* scamText1 = rendertext.CreateText(renderer,sans_path,"You got scammed by the trader!",SDL_Color{153,0,0,255},300);
    SDL_Texture* scamText2 = rendertext.CreateText(renderer,sans_path,"The trader also stole 2 other items from your inventory.",SDL_Color{153,0,0,255},500);
    SDL_Texture* womp =rendertext.CreateText(renderer,sans_path,"Womp Womp", SDL_Color{255,255,255,255},200);
    SDL_Texture* traderText = rendertext.CreateText(renderer,"../trade.ttf","TRADER",SDL_Color{77,64,52,255},200);
    SDL_Texture* luckyblockexit = rendertext.CreateText(renderer,sans_path,"Press ESC to exit",SDL_Color{255,255,255},100);
    SDL_Texture* lvlcompletedmsg = rendertext.CreateText(renderer,"../Roboto.ttf","Congrats! You have completed this level.",SDL_Color{255,255,255},100);
    SDL_Texture* objectivetext = rendertext.CreateText(renderer,"../Roboto.ttf","Objective: Collect ",SDL_Color{255,255,255,255},100);
    SDL_Texture* slash = rendertext.CreateText(renderer,"../Roboto.ttf","/",SDL_Color{255,255,255,255},30);
    SDL_Texture* leveltext = rendertext.CreateText(renderer,"../Roboto.ttf","Level ",SDL_Color{255,255,255,255},100);
    SDL_Texture* homebackground = render_image.createTexture(renderer,"../home_background.png");
    SDL_Texture* joke = rendertext.CreateText(renderer,"../Roboto.ttf","Jk Its a joke, press esc to start playing",SDL_Color{255,255,255,255},100);
    SDL_Texture* deathText;
    SDL_Texture* restarttoplay = rendertext.CreateText(renderer,"../Roboto.ttf","Restart Game to play again.", SDL_Color{255,255,255,255},100);
    vector<SDL_Texture*>luckyblockdeceptmsgs = {rendertext.CreateText(renderer,"../px.ttf","Deception Rule 329: Lucky Blocks are not always lucky",SDL_Color{255,255,255,255},100),rendertext.CreateText(renderer,"../px.ttf","Deception Rule 421: Lucky rewards do not mean great rewards",SDL_Color{255,255,255,255},100),rendertext.CreateText(renderer,"../px.ttf","Deception Rule 68419: Not all lucky blocks give you good luck",SDL_Color{255,255,255,255},100)};
    SDL_Texture* gamelogo = render_image.createTexture(renderer,"../icon.png");
    SDL_Texture* chosenmsg;
    SDL_Texture* infotext = render_image.createTexture(renderer,"../Info_Text.png");
    SDL_Texture* esctocontinue = rendertext.CreateText(renderer,"../Roboto.ttf", "Press Esc to continue",SDL_Color{255,255,255,255},100);
    Mix_Music* bgm = Mix_LoadMUS("../bgm.mp3");
    for (int i =0; i<7;i++)
    {
        itemNameTexture[i] = rendertext.CreateText(renderer,sans_path,itemidNames[i],SDL_Color{255,255,255,255},100);
    }
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
    steady_clock::time_point traderScam;
    steady_clock::time_point givetime;
    steady_clock::time_point showCompleted;
    steady_clock::time_point fakeDeathRealisation;
    steady_clock::time_point decepteffectshow;
    steady_clock::time_point regenTime;
    steady_clock::time_point InfoClickTime;
    int elapsed;
    int elapsed2;
    itemTextures[2] = render_image.createTexture(renderer,"../Diamond.png");
    itemTextures[3] = render_image.createTexture(renderer,"../Trash.png");
    itemTextures[4] = render_image.createTexture(renderer,"../Dirt.png");
    itemTextures[5] = render_image.createTexture(renderer,"../Ruby.png");
    itemTextures[6] = render_image.createTexture(renderer,"../Money.png");
    vector<SDL_Texture*>numbersroboto;
    levelnums[1] = rendertext.CreateText(renderer,"../Roboto.ttf","1",SDL_Color{255,255,255,255},50);
    for (int i=0;i<10;i++)
    {
        sprintf(buffer2,"%d",i);
        numbersroboto.push_back(rendertext.CreateText(renderer,"../Roboto.ttf",buffer2,SDL_Color{255,255,255,255},50));
    }
    for (int i =1;i<17;i++)
    {
        sprintf(buffer2,"%d",i);
        numbers[i] = rendertext.CreateText(renderer,sans_path,buffer2,SDL_Color{255,255,255,255},30);
    }
    while (mode=="tutorial")
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                SDL_Quit();
                return 0;
            }
            if (checkKey("Escape",event))
            {
                mode = "home";
            }
        }
        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderClear(renderer);
        render_image.showImage(renderer,menu,0,0,600,600,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
        render_image.showImage(renderer,infotext,70,100,500,189,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
        rendertext.DrawText(renderer,esctocontinue,200,300,200,20);
        SDL_RenderPresent(renderer);
    }
    while (mode == "home")
    {
        bool Enterclick= false;
        bool Exitclick  = false;
        bool infoclick = false;
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
            if (Info.checkClick(event,mousex,mousey))
            {
                InfoClickTime = steady_clock::now();
                infoclick = true;
            }
        }
        SDL_RenderClear(renderer);
        // Handling Enter Game Click
        render_image.showImage(renderer,homebackground,0,0,600,600,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
        render_image.showImage(renderer,gamelogo,190,80,200,200,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
        auto EntertimeElapsed = duration_cast<milliseconds>(steady_clock::now() - EnterclickTime).count();
        if (EntertimeElapsed >400 && EntertimeElapsed < 450)
        {
            showLoading(renderer,event,"Loading...",font_path,rendertext,500);
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
    Button tradeBut(renderer,"../Trade_Button.png",162,345,270,64);
    //In Game
    vector<int>levelreq = generateLevel();//set level requirement
    int deathOpacity =0;
    steady_clock::time_point fakeplay = steady_clock::now();
    while (duration_cast<milliseconds>(steady_clock::now() - fakeplay).count() < 600)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                SDL_Quit();
                return 0;
            }
        }
        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderClear(renderer);
        render_image.showImage(renderer,background,0,0,600,600,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
        SDL_RenderPresent(renderer);
    }
    fakeDeathRealisation = steady_clock::now();
    while (!canPlay)
    {
        deathText = rendertext.CreateText(renderer,"../dec.ttf","YOU DIED",SDL_Color{136,8,8,255},200);
        SDL_SetTextureAlphaMod(deathText,deathOpacity);
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                SDL_Quit();
                return 0;
            }
            if (escfakedeath&&checkKey("Escape",event))
            {
                canPlay = true;
            }
        }
        if (deathOpacity <255)deathOpacity++;
        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderClear(renderer);
        if (duration_cast<milliseconds>(steady_clock::now()-fakeDeathRealisation).count() >3000)
        {
            rendertext.DrawText(renderer,joke,100,250,400,40);
            escfakedeath = true;
        }
        rendertext.DrawText(renderer,deathText,85,50,400,100);
        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(deathText);
    }
    givetime = steady_clock::now();
    Mix_VolumeMusic(60);
    Mix_PlayMusic(bgm,-1);
    regenTime = steady_clock::now();
    while (true)
    {
        timenow = steady_clock::now();
        // Generate NPC
        if (npcdistribuition(gen) &&  npccount < maxnpc)
        {
            auto generatex = npccoordsgen(gen);
            auto generatey = npccoordsgen(gen);
            tradeno1 = trades[tradegen(gen)];
            tradeno2 = trades[tradegen(gen)];
            bool scamming = scam(gen);
            if (scamming)
            {
                npcpos[npccount] = NPC{generatex,generatey,steady_clock::now(),{scamtrades[scamtradesgen(gen)],scamtrades[scamtradesgen(gen)]},scamming};
            }else
            {
                npcpos[npccount] = NPC{generatex,generatey,steady_clock::now(),{tradeno1,tradeno2},scamming};
            }
            npccount++;
            cout << "NPC Number " << npccount << " generated at ("
     << npcpos[npccount-1].x << ", "
     << npcpos[npccount-1].y << ")" << endl;
        }
        if (duration_cast<milliseconds>(timenow-regenTime).count() == 5000)
        {
            if (healthstate!=1)
            {
                regenTime = steady_clock::now();
                healthstate--;
            }
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
                if (tradeBut.checkClick(event,mousex,mousey))
                {
                    tradebuttonclick = true;
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
                        monstercyclereq = 32;
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
                    movepx+=playerSpeed;
                    trashmovepx += playerSpeed;
                }else if ((checkKey("Right", event)|| checkKey("D", event))&& !tradeguiopen && !luckyblockopened)
                {
                    direction = 2;
                    playerx +=playerx<585?playerSpeed:0;
                    movepx+=playerSpeed;
                    trashmovepx += playerSpeed;
                }else if ((checkKey("Up", event) || checkKey("W", event))&& !tradeguiopen&& !luckyblockopened)
                {
                    direction = 3;
                    playery =playery>15?playerSpeed:0;
                    movepx+=playerSpeed;
                    trashmovepx += playerSpeed;
                }else if ((checkKey("Down", event) || checkKey("S", event))&& !tradeguiopen&& !luckyblockopened)
                {
                    direction = 0;
                    playery +=playery<450?playerSpeed:0;
                    movepx+=playerSpeed;
                    trashmovepx += playerSpeed;
                }else if (checkKey("E", event)&& !luckyblockopened)
                {
                    for (const auto& npc : npcpos)
                    {
                        if (playerNearNpc(playerx, playery, npc.second.x, npc.second.y, 50)) {
                            currentNpcId = npc.first;
                            tradeguiopen = true;
                            trade = false;
                            break;
                        }
                    }
                }else if (tradeguiopen && checkKey("Escape", event))
                {
                    tradeguiopen = false;
                    trade=true;
                    tradeselected =false;
                    tradeguistatus = 0;
                }else if (checkKey("Z",event))
                {
                    tradeguiopen = true;
                }else if (displayObtained && checkKey("Escape",event))
                {
                    displayObtained = false;
                }else if (checkKey("M",event))
                {
                    if (mutemusic)
                    {
                        Mix_ResumeMusic();
                        mutemusic = false;
                    }else
                    {
                        Mix_PauseMusic();
                        mutemusic = true;
                    }
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
        int reqitemsobtained = 0;
        for (auto i:inventory)
        {
            if (i.second.first == levelreq[1])
            {
                reqitemsobtained+=i.second.second;
            }
        }
        if (movepx >=750)
        {
            movepx-=750;
            addInventory(4,1);
        }
        if (trashmovepx >=750)
        {
            trashmovepx-=750;
            if (trashgetchance(gen))
            {
                addInventory(3,1);
            }
        }
        npcnearfound = false;
        for (auto i:npcpos)
        {
            if (playerNearNpc(playerx,playery,i.second.x, i.second.y, 50))
            {
                trade = true;
                npcnearfound = true;
            }
        }
        if (!npcnearfound)
        {
            trade=false;
        }
        moveMonster(monster1,playerx,playery,monstermovementcycle,monstermoveflip,monstercyclereq);
        if (slowplayer&&duration_cast<milliseconds>(timenow - playerSlow).count() > 5000 && duration_cast<milliseconds>(timenow - playerSlow).count() < 5200)
        {
            playerSpeed+=3;
            slowplayer = false;
        }
        if (fastmonster&&duration_cast<milliseconds>(timenow - monsterSpeedGain).count() > 7000&&duration_cast<milliseconds>(timenow - monsterSpeedGain).count() <7200)
        {
            monstercyclereq = 28;
            fastmonster = false;
        }
        if (playerNearNpc(playerx,playery,monster1.x,monster1.y,35))
        {
            if (duration_cast<milliseconds>(steady_clock::now() - lastDamage).count() >= damagecooldown)
            {
                healthstate++;
                lastDamage = steady_clock::now();
            }
        }
        if (healthstate == 7)
        {
            deathOpacity = 0;
            while (true){
                deathText = rendertext.CreateText(renderer,"../dec.ttf","YOU DIED",SDL_Color{136,8,8,255},200);
                SDL_SetTextureAlphaMod(deathText,deathOpacity);
                while (SDL_PollEvent(&event))
                {
                    if (event.type == SDL_QUIT)
                    {
                        SDL_Quit();
                        return 0;
                    }
                }
                if (deathOpacity <255)deathOpacity++;
                if (deathOpacity == 255)
                {
                    SDL_Quit();
                    return 0;
                }
                SDL_SetRenderDrawColor(renderer,0,0,0,255);
                SDL_RenderClear(renderer);
                rendertext.DrawText(renderer,deathText,85,50,400,100);
                rendertext.DrawText(renderer,restarttoplay,150,150,250,50);
                SDL_RenderPresent(renderer);
                SDL_DestroyTexture(deathText);
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
                monstercyclereq = 30;
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
        for (auto i = luckyblockpos.begin();i!=luckyblockpos.end();)
        {
            if (checkCollision(playerx,playery,50,50,i->second.first,i->second.second,20,20)&& !luckyblockopened && !tradeguiopen)
            {
                if (luckyblockresult)
                {
                    SDL_DestroyTexture(luckyblockresult);
                    luckyblockresult = nullptr;
                }
                obtainedgenerated = false;
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
                    deceptionTextShow = steady_clock::now();
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
        if (luckydec)
        {
            auto luckydecdisplay= duration_cast<milliseconds>(timenow-deceptionTextShow).count();
            if (luckydecdisplay > 1000&&luckydecdisplay < 4000)
            {
                if (predecept == nullptr)
                {
                    SDL_DestroyTexture(predecept);
                    predecept = nullptr;
                }
                predecept = rendertext.CreateText(renderer,"../dec.ttf","Or did you?", SDL_Color{136,8,8,255},100);
                if (storyopacity <255)storyopacity++;
                SDL_SetTextureAlphaMod(predecept,storyopacity);
                rendertext.DrawText(renderer,predecept,200,100,200,60);
            }else if (luckydecdisplay == 4000)
            {
                decepteffectshow = steady_clock::now();
                showeffects = true;
                chosenmsg = luckyblockdeceptmsgs[luckydecptmsgdist(gen)];
                applyluckydeceff = true;
                storyopacity = 0;
            }
        }
        if (showeffects)
        {
            if (duration_cast<milliseconds>(timenow-decepteffectshow).count()<1000)
            {
                rendertext.DrawText(renderer,chosenmsg,125,20,350,20);
            }
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
                }else
                {
                    render_image.showImage(renderer,itemTextures[i.second.first],50+(i.first-1)*55,500,55,55,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
                }
            }
        }
        render_image.showImage(renderer,player,playerx,playery,50,50,shown[0],shown[1],shown[2],shown[3],spriteflip);
        render_image.showImage(renderer,health,playerx,playery-15,50,13,healthpos[healthstate][0],healthpos[healthstate][1],healthpos[healthstate][2],healthpos[healthstate][3],SDL_FLIP_NONE);
        for (auto i = npcpos.begin(); i != npcpos.end();)
        {
            auto lifespanelapsed = duration_cast<seconds>(steady_clock::now() - i->second.spawntime).count();
            //Despawn npc after 30s
            if (lifespanelapsed > 45)
            {
                npccount--;
                cout << "Despawned" << endl;
                i = npcpos.erase(i);
            }else {
                render_image.showImage(renderer,npc,i->second.x,i->second.y,50,50,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
                ++i;
            }
        }
        render_image.showImage(renderer,monster,monster1.x,monster1.y,70,70,NULL,NULL,NULL,NULL,monstermoveflip);
        for (auto i :luckyblockpos)render_image.showImage(renderer,luckyblock,i.second.first,i.second.second,20,20,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
        if (displayObtained && duration_cast<milliseconds>(steady_clock::now()-displaytime).count() <3000)
        {
            SDL_DestroyTexture(luckyblockresult);
            render_image.showImage(renderer,menubackground,0,0,600,600,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
            render_image.showImage(renderer,menu,125,200,340,200,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
            rendertext.DrawText(renderer,luckyblockresulttitle,200,220,200,30);
            rendertext.DrawText(renderer,luckyblockmsg,240,250,130,40);
            rendertext.DrawText(renderer,luckyblockexit,200,320,200,40);
            if (obtained == 0)
            {
                render_image.showImage(renderer,potions,280,285,39,40,potionpos["Slow"][0],potionpos["Slow"][1],potionpos["Slow"][2],potionpos["Slow"][3],SDL_FLIP_NONE);
            }else if (obtained == 1)
            {
                render_image.showImage(renderer,potions,280,285,39,40,potionpos["Speed"][0],potionpos["Speed"][1],potionpos["Speed"][2],potionpos["Speed"][3],SDL_FLIP_NONE);
            }else
            {
                rendertext.DrawText(renderer,itemTextures[obtained],280,285,39,40);
            }
        }else if (duration_cast<milliseconds>(steady_clock::now()-displaytime).count() ==1600 && displayObtained)
        {
            displayObtained = false;
            SDL_DestroyTexture(luckyblockresult);
        }
        if (luckyblockopened)
        {
            auto elapsed = duration_cast<milliseconds>(steady_clock::now() - luckyblockpickup).count();
            if (!deceptiontimerset&&luckydectp)
            {
                deceptiontimerset = true;
                deceptionTextShow = timenow;
            }
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
                    obtained = luckyblockpre(gen);
                    luckyblockpickup = steady_clock::now();
                    luckyblockcycle++;
                }
                if (obtained == 0)
                {
                    render_image.showImage(renderer,potions,280,285,39,40,potionpos["Slow"][0],potionpos["Slow"][1],potionpos["Slow"][2],potionpos["Slow"][3],SDL_FLIP_NONE);
                }else if (obtained == 1)
                {
                    render_image.showImage(renderer,potions,280,285,39,40,potionpos["Speed"][0],potionpos["Speed"][1],potionpos["Speed"][2],potionpos["Speed"][3],SDL_FLIP_NONE);
                }else
                {
                    rendertext.DrawText(renderer,itemTextures[obtained],280,285,39,40);
                }
            }else if (!luckydectp)
            {
                SDL_DestroyTexture(luckyblockresult);
                luckyblockresult = nullptr;
                addInventory(obtained,1);
                luckyblockcycle =1;
                luckyblockopened = false;
                displayObtained = true;
                displaytime = steady_clock::now();
                if (!obtainedgenerated)
                {
                    obtained = luckyblockitemgen(gen);
                    obtainedgenerated = true;
                }
            }else
            {
                displaytime = steady_clock::now();
                SDL_DestroyTexture(luckyblockresult);
                luckyblockresult = nullptr;
                displayObtained = true;
                if (!obtainedgenerated)
                {
                    obtained = luckyblockitemgen(gen);
                    obtainedgenerated = true;
                }
                luckyblockopened = false;
                luckyblockcycle =1;
            }
        }

        for (auto i:inventory)
        {
            if (i.second.first !=-1)
            {
                rendertext.DrawText(renderer,numbers[i.second.second],77+(i.first-1)*55,528,20,25);
            }
            if (inventory[selectedSlot].first !=-1)
            {
                rendertext.DrawText(renderer,itemNameTexture[inventory[selectedSlot].first],200,450,(strlen(itemidNames[inventory[selectedSlot].first])*14),45);
            }
        }
        if (checkLevelRequirements(inventory,levelreq))
        {
            showCompleted = steady_clock::now();
            levelreq = generateLevel();
            cout << "cleared" << endl;
        }
        if (duration_cast<milliseconds>(timenow-showCompleted).count()<1500)
        {
            rendertext.DrawText(renderer,lvlcompletedmsg,160,25,260,30);
        }else if (duration_cast<milliseconds>(timenow-showCompleted).count()<1600)
        {
            healthstate=1;
            playerx,playery = 10;
            npcpos.clear();
            luckyblockpos.clear();
            inventory.clear();
            npccount =0;
            luckyblockcount =0;
            monster1.x = 300;
            monster1.y = 300;
            showLoading(renderer,event,"Loading lvl", sans_path, rendertext,1000);
            levelnums.clear();
            level++;
            sprintf(buffer2,"%d",level);
            levelnums[level] = rendertext.CreateText(renderer,"../Roboto.ttf",buffer2,SDL_Color{255,255,255,255},100);
        }
        rendertext.DrawText(renderer,objectivetext,200,50,100,15);
        rendertext.DrawText(renderer,numbersroboto[reqitemsobtained],300,50,10,15);
        rendertext.DrawText(renderer,slash,310,50,8,15);
        rendertext.DrawText(renderer,numbersroboto[levelreq[2]],318,50,10,15);
        rendertext.DrawText(renderer,itemTextures[levelreq[1]],328,50,15,15);
        rendertext.DrawText(renderer,leveltext,350,50,35,15);
        rendertext.DrawText(renderer,levelnums[level],385,50,8,15);
        if (applyluckydeceff&&luckydec && duration_cast<milliseconds>(steady_clock::now()-deceptionTextShow).count() >=3000)
        {
            if (luckydectp == "Tp")
            {
                playerx = monster1.x;
                playery = monster1.y;
                cout << "Player tp to monster" <<endl;
            }else if (luckydectp == "Speed")
            {
                monstercyclereq = 28;
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
                luckyblockcount = 0;
                npccount = 0;
                luckyblockpos.clear();
                npcpos.clear();
                cout << "All Npc and Luckyblocks cleared" << endl;
            }else if (luckydectp == "MultiSteal")
            {
                stealTwoOfFirstItem(inventory);
            }else if (luckydectp == "Damage")
            {
                healthstate+=1;
            }
            luckydectp = nullptr;
            luckydec,deceptiontimerset = false;
        }
        //WARNING:The texture below should be the last thing that is rendered
        if (tradeguiopen)
        {
            if (currentNpcId != -1) {
                auto tradesForNpc = npcpos[currentNpcId].trades; // This is a vector of vectors

                // Check if there are at least two trades available
                if (tradesForNpc.size() >= 2) {
                    // Load first trade
                    vector<int> firstTrade = tradesForNpc[0]; // Get the first trade vector
                    tradeno1 = firstTrade; // Assign the first trade to tradeno1

                    // Load second trade
                    vector<int> secondTrade = tradesForNpc[1]; // Get the second trade vector
                    tradeno2 = secondTrade; // Assign the second trade to tradeno2
                }
            }
            render_image.showImage(renderer,menubackground,0,0,600,600,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
            render_image.showImage(renderer,menu,135,110,325,325,NULL,NULL,NULL,NULL,SDL_FLIP_NONE);
            render_image.showImage(renderer,tradeui,135,110,325,325,tradeguipos[tradeguistatus][0],tradeguipos[tradeguistatus][1],96,96,SDL_FLIP_NONE);
            rendertext.DrawText(renderer,traderText,200,150,200,50);
            //render first trade
            if (tradeno1[0] == 0)
            {
                render_image.showImage(renderer, potions, 190, 210, 50, 50, potionpos["Slow"][0], potionpos["Slow"][1], potionpos["Slow"][2], potionpos["Slow"][3], SDL_FLIP_NONE);
            }else if (tradeno1[0] == 1)
            {
                render_image.showImage(renderer, potions, 190, 210, 50, 50, potionpos["Speed"][0], potionpos["Speed"][1], potionpos["Speed"][2], potionpos["Speed"][3], SDL_FLIP_NONE);
            }else
            {
                render_image.showImage(renderer, itemTextures[tradeno1[0]], 190, 210, 50, 50, NULL, NULL, NULL, NULL, SDL_FLIP_NONE);
            }
            sprintf(buffer2, "%d", tradeno1[1]); // Count of the first item
            tradeno1count = rendertext.CreateText(renderer, sans_path, buffer2, SDL_Color{255, 255, 255, 255}, 30);
            rendertext.DrawText(renderer, tradeno1count, 215, 230, 20, 20);
            if (tradeno1[2] == 0)
            {
                render_image.showImage(renderer, potions, 355, 210, 50, 50, potionpos["Slow"][0], potionpos["Slow"][1], potionpos["Slow"][2], potionpos["Slow"][3], SDL_FLIP_NONE);
            }else if (tradeno1[2] == 1)
            {
                render_image.showImage(renderer, itemTextures[tradeno1[2]], 355, 210, 50, 50, potionpos["Speed"][0], potionpos["Speed"][1], potionpos["Speed"][2], potionpos["Speed"][3], SDL_FLIP_NONE);
            }else
            {
                render_image.showImage(renderer, itemTextures[tradeno1[2]], 355, 210, 50, 50, NULL, NULL, NULL, NULL, SDL_FLIP_NONE);
            }
            SDL_DestroyTexture(tradeno1count);
            sprintf(buffer2, "%d", tradeno1[3]); // Count of the second item
            tradeno2count = rendertext.CreateText(renderer, sans_path, buffer2, SDL_Color{255, 255, 255, 255}, 30);
            rendertext.DrawText(renderer, tradeno2count, 380, 230, 20, 20);
            // Render second trade
            if (tradeno2[0] == 0)
            {
                render_image.showImage(renderer,potions,190,280,50,50,potionpos["Slow"][0], potionpos["Slow"][1], potionpos["Slow"][2], potionpos["Slow"][3],SDL_FLIP_NONE);
            }else if(tradeno2[0] == 1)
            {
                render_image.showImage(renderer,potions,190,280,50,50,potionpos["Speed"][0], potionpos["Speed"][1], potionpos["Speed"][2], potionpos["Speed"][3],SDL_FLIP_NONE);
            }else
            {
                render_image.showImage(renderer, itemTextures[tradeno2[0]], 190, 280, 50, 50, NULL, NULL, NULL, NULL, SDL_FLIP_NONE);
            }
            sprintf(buffer2, "%d", tradeno2[1]); // Count of the first item in the second trade
            tradeno2count = rendertext.CreateText(renderer, sans_path, buffer2, SDL_Color{255, 255, 255, 255}, 30);
            rendertext.DrawText(renderer, tradeno2count, 215, 300, 20, 20);
            if (tradeno2[2] == 0)
            {
                render_image.showImage(renderer,potions,355, 280, 50, 50,potionpos["Slow"][0], potionpos["Slow"][1], potionpos["Slow"][2], potionpos["Slow"][3],SDL_FLIP_NONE);
            }else if(tradeno2[2] ==1)
            {
                render_image.showImage(renderer,potions,355, 280, 50, 50,potionpos["Speed"][0], potionpos["Speed"][1], potionpos["Speed"][2], potionpos["Speed"][3], SDL_FLIP_NONE);
            }else
            {
                render_image.showImage(renderer, itemTextures[tradeno2[2]], 355, 280, 50, 50, NULL, NULL, NULL, NULL, SDL_FLIP_NONE);
            }
            sprintf(buffer2, "%d", tradeno2[3]); // Count of the second item in the second trade
            tradeno2count = rendertext.CreateText(renderer, sans_path, buffer2, SDL_Color{255, 255, 255, 255}, 30);
            rendertext.DrawText(renderer, tradeno2count, 380, 300, 20, 20);
            SDL_DestroyTexture(tradeno1count);
            SDL_DestroyTexture(tradeno2count);
            //render trade button
            if (tradebuttonclick)
            {
                tradeBut.RenderButton(renderer,tradebutpos[2][0],tradebutpos[2][1],tradebutpos[2][2],tradebutpos[2][3]);
            }
            else if (tradeBut.checkHover(mousex,mousey))
            {
                tradeBut.RenderButton(renderer,tradebutpos[1][0],tradebutpos[1][1],tradebutpos[1][2],tradebutpos[1][3]);
            }else
            {
                tradeBut.RenderButton(renderer,tradebutpos[0][0],tradebutpos[0][1],tradebutpos[0][2],tradebutpos[0][3]);
            }
        }
        if (tradebuttonclick) {
            bool tradeSuccessful = false;

            if (tradeguistatus == 1) {
                int itemToGive = tradeno2[0];
                int countToGive = tradeno2[1];
                int itemToReceive = tradeno2[2];
                int countToReceive = tradeno2[3];

                if (hasRequiredItems(inventory, itemToGive, countToGive)) {
                    for (auto& slot : inventory) {
                        if (slot.second.first == itemToGive) {
                            slot.second.second -= countToGive;
                            if (slot.second.second <= 0) {
                                slot.second.first = -1;
                            }
                            break;
                        }
                    }
                    if (npcpos[currentNpcId].scammer)
                    {
                        traderScam = steady_clock::now();
                        showTraderScamMsg = true;
                        stealTwoOfFirstItem(inventory);
                        cout << "Trade failed! This NPC is a scammer." << endl;
                        tradebuttonclick = false; // Reset the button click flag
                        tradeguiopen = false;
                        trade = true;
                        continue; // Skip the trade logic
                    }
                    tradeSuccessful = true;
                    addInventory(itemToReceive, countToReceive); // Add the new item

                }
            } else if (tradeguistatus == 2) { // If trade 2 is selected
                int itemToGive = tradeno1[0]; // Item ID to give
                int countToGive = tradeno1[1]; // Count to give
                int itemToReceive = tradeno1[2]; // Item ID to receive
                int countToReceive = tradeno1[3]; // Count to receive

                // Check if the player has the required item to give
                if (hasRequiredItems(inventory, itemToGive, countToGive)) {
                    // Update inventory: remove the traded item and add the new item
                    for (auto& slot : inventory) {
                        if (slot.second.first == itemToGive) {
                            slot.second.second -= countToGive; // Remove the required item
                            if (slot.second.second <= 0) {
                                slot.second.first = -1; // Mark slot as empty if count is zero
                            }
                            break; // Exit loop after finding the item
                        }
                    }
                    tradeSuccessful = true;
                    if (npcpos[currentNpcId].scammer)
                        {
                        traderScam = steady_clock::now();
                        showTraderScamMsg = true;
                        stealTwoOfFirstItem(inventory);
                        cout << "Trade failed! This NPC is a scammer." << endl;
                        tradebuttonclick = false; // Reset the button click flag
                        tradeguiopen = false;
                        trade = true;
                        continue; // Skip the trade logic
                    }
                    addInventory(itemToReceive, countToReceive); // Add the new item
                }
            }

            // Provide feedback to the player
            if (tradeSuccessful) {
                cout << "Trade successful!" << endl;
            } else {
                cout << "Trade failed! You do not have the required items." << endl;
            }

            tradebuttonclick = false; // Reset the button click flag
        }
        if (showTraderScamMsg)
        {
            if (duration_cast<milliseconds>(timenow-traderScam).count()< 2000)
            {
                cout << "showing" << endl;
                rendertext.DrawText(renderer,scamText1,130,35,350,30);
            }else if (duration_cast<milliseconds>(timenow-traderScam).count()< 4000)
            {
                rendertext.DrawText(renderer,scamText2,90,35,400,30);
            }else if (duration_cast<milliseconds>(timenow-traderScam).count()< 5000)
            {
                rendertext.DrawText(renderer,womp,330,35,100,30);
            }else
            {
                showTraderScamMsg = false;
            }

        }
        SDL_RenderPresent(renderer);
        //Limit Fps
        uint32_t frameTimeEnd = SDL_GetTicks();
        uint32_t frameDuration = frameTimeEnd - frameStart;
        //Increment per frame
        frameCount++;
        movementcycles++;
    }
}