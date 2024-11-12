//
// Created by qiu_tian on 4/11/2024.
//

#ifndef RENDERTEXT_H
#define RENDERTEXT_H

#include <SDL_ttf.h>
namespace T
{
    class RenderText
    {
    public:
        RenderText() = default;
        ~RenderText() = default;
        static SDL_Texture* CreateText(SDL_Renderer* renderer,char* fontpath, char* text,SDL_Color colour,int size );

        static void DrawText(SDL_Renderer* renderer, SDL_Texture* texture, int x,int y,int w, int h);
    };
}
#endif //RENDERTEXT_H
