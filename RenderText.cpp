#include "RenderText.h"
#include <iostream>
//
// Created by qiu_tian on 5/11/2024.
//
using namespace T;
using namespace std;

SDL_Texture* RenderText::CreateText(SDL_Renderer* renderer, char* fontpath, char* text, SDL_Color colour, int resolution) {
    TTF_Font* font = TTF_OpenFont(fontpath, resolution);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return nullptr;
    }

    SDL_Surface* surface = TTF_RenderText_Solid(font, text, colour);
    if (!surface) {
        std::cerr << "Failed to create surface: " << TTF_GetError() << std::endl;
        TTF_CloseFont(font);
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
    }

    SDL_FreeSurface(surface);
    TTF_CloseFont(font);
    return texture;
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


