//
// Created by qiu_tian on 5/11/2024.
//
#include <iostream>
#include <RenderImage.h>
#include <Button.h>
#include <vector>
using namespace SDL_Render_Image;
using namespace button;
using namespace std;
vector<Button*> Button::buttons;
Button::Button(SDL_Renderer* renderer, char* path, int x, int y, int w, int h)
{
    surface = IMG_Load(path);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!w|!h)
    {
        rect.w = surface->w;
        rect.h = surface->h;
    }else
    {
        rect.w = w;
        rect.h = h;
    }
    rect.x = x;
    rect.y = y;
    buttons.push_back(this);
}
void Button::RenderButton(SDL_Renderer* renderer,int srcx,int srcy,int srcw,int srch)
{
        SDL_Rect src = { srcx, srcy, srcw, srch };
        SDL_RenderCopy(renderer,texture,&src,&rect);
}
void Button::load(SDL_Renderer* renderer, char* path, int x, int y, int w, int h)
{
    surface = IMG_Load(path);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!w|!h)
    {
        rect.w = surface->w;
        rect.h = surface->h;
    }else
    {
        rect.w = w;
        rect.h = h;
    }
    rect.x = x;
    rect.y = y;
}

bool Button::checkHover(int mousex, int mousey)
{
    return mousex >= rect.x && mousex <= rect.x + 200 &&
           mousey >= rect.y && mousey <= rect.y + 95;
}
bool Button::checkClick(SDL_Event event, int mousex, int mousey)
{
    return event.type== SDL_MOUSEBUTTONDOWN&&mousex >= rect.x && mousex <= rect.x + 150 &&
           mousey >= rect.y && mousey <= rect.y + 100;
}
void Button::destroyAll()
{
    for (auto i:buttons)
    {
        SDL_DestroyTexture(i->texture);
    }
    buttons.clear();
}

