#include "raylib.h"
#include <vector>
#include <cstdlib>

struct Bird
{
    Rectangle rect;
    float velocity;
};

struct Pipe
{
    Rectangle top;
    Rectangle bottom;
    bool passed;
};

int main()
{
    const int screenWidth = 400;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Flappy Bird - raylib");
    SetTargetFPS(60);

    // Bird
    Bird bird;
    bird.rect = {50, 200, 30, 30};
    bird.velocity = 0;

    const float gravity = 900.0f;
    const float jumpForce = -300.0f;

    // Pipes
    std::vector<Pipe> pipes;
    const float pipeSpeed = 200.0f;
    const int pipeWidth = 60;
    const int gap = 150;

    float pipeTimer = 0;
    int score = 0;
    bool gameOver = false;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        /* ---------------- UPDATE ---------------- */
        if (!gameOver)
        {
            // Jump
            if (IsKeyPressed(KEY_SPACE))
            {
                bird.velocity = jumpForce;
            }

            // Gravity
            bird.velocity += gravity * dt;
            bird.rect.y += bird.velocity * dt;

            // Pipe spawn
            pipeTimer += dt;
            if (pipeTimer > 1.5f)
            {
                pipeTimer = 0;

                int topHeight = GetRandomValue(50, 300);
                Pipe p;
                p.top = {(float)screenWidth, 0, (float)pipeWidth, (float)topHeight};
                p.bottom = {(float)screenWidth, (float)(topHeight + gap),
                            (float)pipeWidth,
                            (float)(screenHeight - topHeight - gap)};
                p.passed = false;

                pipes.push_back(p);
            }

            // Move pipes
            for (auto &p : pipes)
            {
                p.top.x -= pipeSpeed * dt;
                p.bottom.x -= pipeSpeed * dt;

                // Score
                if (!p.passed && p.top.x + pipeWidth < bird.rect.x)
                {
                    score++;
                    p.passed = true;
                }

                // Collision
                if (CheckCollisionRecs(bird.rect, p.top) ||
                    CheckCollisionRecs(bird.rect, p.bottom))
                {
                    gameOver = true;
                }
            }

            // Ground & ceiling
            if (bird.rect.y < 0 || bird.rect.y + bird.rect.height > screenHeight)
            {
                gameOver = true;
            }
        }

        // Restart
        if (gameOver && IsKeyPressed(KEY_R))
        {
            bird.rect.y = 200;
            bird.velocity = 0;
            pipes.clear();
            score = 0;
            gameOver = false;
        }

        /* ---------------- DRAW ---------------- */
        BeginDrawing();
        ClearBackground(SKYBLUE);

        // Bird
        DrawRectangleRec(bird.rect, YELLOW);

        // Pipes
        for (auto &p : pipes)
        {
            DrawRectangleRec(p.top, GREEN);
            DrawRectangleRec(p.bottom, GREEN);
        }

        // Score
        DrawText(TextFormat("Score: %d", score), 20, 20, 20, BLACK);

        if (gameOver)
        {
            DrawText("GAME OVER", 110, 250, 30, RED);
            DrawText("Press R to Restart", 95, 290, 20, BLACK);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
