#include "raylib.h"
#include <vector>
#include <cstdlib>
#include <algorithm>

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
    bird.rect = {50, 200, 60, 60};
    bird.velocity = 0;

    const float gravity = 900.0f;
    const float jumpForce = -300.0f;

    // Pipes
    std::vector<Pipe> pipes;
    const float pipeSpeed = 200.0f;
    const int pipeWidth = 60;
    const int gap = 165;

    const float CAP_TEX_H = 24.0f; // pixels of cap inside pipeTex
    const float CAP_DST_H = 24.0f; // how tall to draw the cap on screen

    float pipeTimer = 0;
    int score = 0;
    bool gameOver = false;

    Texture2D birdTex = LoadTexture("opSprite.png");
    Texture2D pipeTex = LoadTexture("pipe.png");
    // Pre-flipped version for top pipes
    Image pipeImg = LoadImage("pipe.png");
    ImageFlipVertical(&pipeImg);
    Texture2D pipeFlippedTex = LoadTextureFromImage(pipeImg);
    UnloadImage(pipeImg); // free the CPU image, keep GPU texture

    const int frameCount = 3; // number of frames in sprite sheet
    const int frameWidth = birdTex.width / frameCount;
    const int frameHeight = birdTex.height;

    int currentFrame = 0;
    float frameTime = 0.0f;
    const float frameDelay = 0.1f; // seconds per frame

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
            bird.velocity += gravity * dt; // 1s = 15 pixel drop per frame
            bird.rect.y += bird.velocity * dt;

            // Pipe spawn
            pipeTimer += dt;
            if (pipeTimer > 1.5f) // every 1.5 seconds pipe will be spawn
            {
                pipeTimer = 0;

                int topHeight = GetRandomValue(50, 230); // this is top pipe height
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
            pipes.erase(
                std::remove_if(pipes.begin(), pipes.end(),
                               [](const Pipe &p)
                               { return p.top.x + p.top.width < 0; }),
                pipes.end());

            // Ground & ceiling
            if (bird.rect.y < 0 || bird.rect.y + bird.rect.height > screenHeight)
            {
                gameOver = true;
            }

            // update animation
            frameTime += dt;

            if (frameTime >= frameDelay)
            {
                frameTime = 0.0f;
                currentFrame++;

                if (currentFrame >= frameCount)
                {
                    currentFrame = 0;
                }
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

        // Score
        DrawText(TextFormat("Score: %d", score), 20, 20, 20, BLACK);

        if (gameOver)
        {
            DrawText("GAME OVER", 110, 250, 30, RED);
            DrawText("Press R to Restart", 95, 290, 20, BLACK);
        }

        Rectangle source = {
            (float)(currentFrame * frameWidth),
            0,
            (float)frameWidth,
            (float)frameHeight};
        // DrawRectangleRec(bird.rect, YELLOW);
        Rectangle dest = {
            bird.rect.x,
            bird.rect.y,
            bird.rect.width,
            bird.rect.height};

        DrawTexturePro(
            birdTex,
            source,
            dest,
            {0, 0},
            0.0f,
            WHITE);

        // now pipe texture
        // BOTTOM PIPES — just two slices
        for (auto &p : pipes)
        {
            float bodyH = p.bottom.height - CAP_DST_H;

            // Cap — fixed
            Rectangle capSrc = {0, 0, (float)pipeTex.width, CAP_TEX_H};
            Rectangle capDest = {p.bottom.x, p.bottom.y, p.bottom.width, CAP_DST_H};
            DrawTexturePro(pipeTex, capSrc, capDest, {0, 0}, 0.0f, WHITE);

            // Body — everything below the cap, stretched
            Rectangle bodySrc = {0, CAP_TEX_H,
                                 (float)pipeTex.width,
                                 (float)pipeTex.height - CAP_TEX_H};
            Rectangle bodyDest = {p.bottom.x, p.bottom.y + CAP_DST_H,
                                  p.bottom.width, bodyH};
            DrawTexturePro(pipeTex, bodySrc, bodyDest, {0, 0}, 0.0f, WHITE);
        }

        // ── TOP PIPES ──
        // After vertical flip: body is at top, cap is at bottom of flipped texture
        for (auto &p : pipes)
        {
            float bodyH = p.top.height - CAP_DST_H;

            // Body — top portion of flipped texture, stretched
            Rectangle bodySrc = {0, 0,
                                 (float)pipeFlippedTex.width,
                                 (float)pipeFlippedTex.height - CAP_TEX_H};
            Rectangle bodyDest = {p.top.x, p.top.y, p.top.width, bodyH};
            DrawTexturePro(pipeFlippedTex, bodySrc, bodyDest, {0, 0}, 0.0f, WHITE);

            // Cap — bottom portion of flipped texture, fixed height
            Rectangle capSrc = {0, (float)pipeFlippedTex.height - CAP_TEX_H,
                                (float)pipeFlippedTex.width, CAP_TEX_H};
            Rectangle capDest = {p.top.x, p.top.y + bodyH, p.top.width, CAP_DST_H};
            DrawTexturePro(pipeFlippedTex, capSrc, capDest, {0, 0}, 0.0f, WHITE);
        }
        EndDrawing();
    }
    UnloadTexture(birdTex);
    UnloadTexture(pipeTex);
    UnloadTexture(pipeFlippedTex); // add this alongside UnloadTexture(pipeTex)
    CloseWindow();
    return 0;
}