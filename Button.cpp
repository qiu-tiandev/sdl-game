//
// Created by qiu_tian on 5/11/2024.
//
#include <RenderImage.h>
#include <Button.h>
using namespace SDL_Render_Image;
using namespace button;

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
}
void Button::RenderButton(SDL_Renderer* renderer,int srcx,int srcy,int srcw,int srch)
{
    if (srcx&&srcy&&srcw&&srch)
    {
        SDL_Rect src = { srcx, srcy, srcw, srch };
        SDL_RenderCopy(renderer,texture,&src,&rect);
    }else
    {
        SDL_RenderCopy(renderer,texture,NULL,&rect);
    }
}
bool Button::checkClick(SDL_Event event, int mousex, int mousey)
{
    if (event.type == SDL_MOUSEBUTTONDOWN&& event.button.button == SDL_BUTTON_LEFT)
    {
        return (mousex > rect.x && mousex < rect.x + rect.w && mousey > rect.y && mousey < rect.y + rect.h);
    }
    return false;
}
bool Button::Hover(int mousex, int mousey)
{
    return (mousex > rect.x && mousex < rect.x + rect.w && mousey > rect.y && mousey < rect.y + rect.h);
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


