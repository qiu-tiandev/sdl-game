cmake_minimum_required(VERSION 3.29)
project(SDL2TEST)

set(CMAKE_CXX_STANDARD 20)
include_directories("C:/Users/studentadmin/SDL2/include")
include_directories(${PROJECT_SOURCE_DIR})
link_directories("C:/Users/studentadmin/SDL2/lib/x64")
include_directories("C:/Users/studentadmin/Downloads/SDL2_image-devel-2.8.2-VC/SDL2_image-2.8.2/include")
link_directories("C:/Users/studentadmin/Downloads/SDL2_image-devel-2.8.2-VC/SDL2_image-2.8.2/lib/x64")
include_directories("C:/Users/studentadmin/Downloads/SDL2_ttf-devel-2.22.0-VC/SDL2_ttf-2.22.0/include")
link_directories("C:/Users/studentadmin/Downloads/SDL2_ttf-devel-2.22.0-VC/SDL2_ttf-2.22.0/lib/x64")
add_executable(SDL2TEST main.cpp
        RenderImage.h
        RenderText.h
        RenderText.cpp
        RenderImage.cpp
        Button.h
        Button.cpp
)
target_link_libraries(SDL2TEST PRIVATE
        SDL2
        SDL2main
        SDL2_Image
        SDL2_ttf
)

