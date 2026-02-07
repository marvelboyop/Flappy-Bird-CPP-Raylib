#include <iostream>
#include <raylib.h>

using namespace std;

const int screen_width = 100;
const int screen_height = 100;

int main()
{
    InitWindow(100, 100, "Flappy Bird");
    while (!WindowShouldClose())
        BeginDrawing();
    ClearBackground(BLACK);

    EndDrawing();
    return 0;
}