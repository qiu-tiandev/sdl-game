//
// Created by qiu_tian on 4/11/2024.
//

#ifndef RENDERIMAGE_H
#define RENDERIMAGE_H


#include <SDL_image.h>
#include <SDL.h>
namespace SDL_Render_Image
{
    class RenderImage
    {
    public:
        RenderImage() = default;
        ~RenderImage() = default;
        static SDL_Texture* createTexture(SDL_Renderer* renderer, char* path);

        static void showImage(SDL_Renderer* renderer, SDL_Texture* texture,int x, int y, int w,int h,int sourcex,int sourcey,int sourcew,int sourceh,SDL_RendererFlip flip);
    };
}
#endif //RENDERIMAGE_H