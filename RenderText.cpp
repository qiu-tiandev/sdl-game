#include <RenderText.h>
//
// Created by qiu_tian on 5/11/2024.
//
using namespace T;

SDL_Texture* RenderText::CreateText(SDL_Renderer* renderer, char* fontpath, char* text, SDL_Color colour, int size)
{
    {
            TTF_Font* font = TTF_OpenFont(fontpath, size);
            SDL_Surface* surface = TTF_RenderText_Solid(font,text, colour);
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer,surface);
            return texture;
        }
}
void RenderText::DrawText(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y, int w, int h)
{

        SDL_Rect rect;
        if (!w|!h)
        {
            SDL_QueryTexture(texture,NULL,NULL,&rect.w,&rect.h);
        }else
        {
            rect.w = w;
            rect.h = h;
        }
        rect.x = x;
        rect.y = y;
        SDL_RenderCopy(renderer, texture, nullptr, &rect);

}


