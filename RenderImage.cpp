//
// Created by qiu_tian on 5/11/2024.
//
#include "RenderImage.h"
#include <iostream>
using namespace SDL_Render_Image;
using namespace std;
SDL_Texture* RenderImage::createTexture(SDL_Renderer* renderer, char* path)
{
    SDL_Surface* surface = IMG_Load(path);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void RenderImage::showImage(SDL_Renderer* renderer, SDL_Texture* texture,int x, int y, int w,int h,int sourcex,int sourcey,int sourcew,int sourceh,SDL_RendererFlip flip)
{
    SDL_Rect rect,srcrect;
    if (!w || !h)
    {
        SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
    }else
    {
        rect.w = w;
        rect.h = h;
    }
    rect.x = x;
    rect.y = y;
    if (sourceh && sourcew)
    {
        srcrect.w = sourcew;
        srcrect.h = sourceh;
        srcrect.x = sourcex;
        srcrect.y = sourcey;
        SDL_RenderCopyEx(renderer, texture, &srcrect, &rect,0,NULL, flip);
    }else
    {
        SDL_RenderCopyEx(renderer, texture, nullptr, &rect,0,NULL, flip);
    }
}