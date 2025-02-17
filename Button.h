//
// Created by qiu_tian on 5/11/2024.
//
#include <vector>
using namespace std;
#ifndef BUTTON_H
#define BUTTON_H
namespace button
{
    class Button
    {
    public:
        SDL_Rect rect;
        SDL_Texture* texture;
        SDL_Surface* surface;
        int x;
        int y;
        Button(SDL_Renderer* renderer, char* path, int x,int y,int w, int h);
        void RenderButton(SDL_Renderer* renderer,int srcx,int srcy,int srcw,int srch);
        bool checkClick(SDL_Event event,int mousex,int mousey);
        bool checkHover(int mousex,int mousey);
        void load(SDL_Renderer* renderer, char* path, int x, int y, int w, int h);
        static void destroyAll();
    private:
        static vector<Button*> buttons;
    };
}
#endif //BUTTON_H
