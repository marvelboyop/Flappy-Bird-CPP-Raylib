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
    bird.rect = {50, 200, 60, 60};
    bird.velocity = 0;

    const float gravity = 900.0f;
    const float jumpForce = -300.0f;

    // Pipes
    std::vector<Pipe> pipes;
    const float pipeSpeed = 200.0f;
    const int pipeWidth = 60;
    const int gap = 165;

    float pipeTimer = 0;
    int score = 0;
    bool gameOver = false;

    Texture2D birdTex = LoadTexture("opSprite.png");
    Texture2D pipeTex = LoadTexture("pipe.png");
    Texture2D pipetopTex = LoadTexture("top_pipe.png");
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

        // Bird
        // DrawRectangleRec(bird.rect, YELLOW);

        // Pipes
        // // for (auto &p : pipes)
        // {
        //     DrawRectangleRec(p.top, GREEN);
        //     DrawRectangleRec(p.bottom, GREEN);
        // }

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
        for (auto &p : pipes)
        {
            float neededHeight = p.bottom.height;

            // ---- If pipe height <= texture height ----
            if (neededHeight <= pipeTex.height)
            {
                Rectangle pipe_source = {
                    0.0f,
                    0.0f,
                    (float)pipeTex.width,
                    neededHeight};

                Rectangle pipe_dest = {
                    p.bottom.x,
                    p.bottom.y,
                    p.bottom.width,
                    neededHeight};

                DrawTexturePro(pipeTex,
                               pipe_source,
                               pipe_dest,
                               {0, 0},
                               0.0f,
                               WHITE);
            }
            else
            {
                // ---- If pipe height is larger than texture ----
                // Draw full texture stretched to pipe height

                Rectangle pipe_source = {
                    0.0f,
                    0.0f,
                    (float)pipeTex.width,
                    (float)pipeTex.height};

                Rectangle pipe_dest = {
                    p.bottom.x,
                    p.bottom.y,
                    p.bottom.width,
                    p.bottom.height};

                DrawTexturePro(pipeTex,
                               pipe_source,
                               pipe_dest,
                               {0, 0},
                               0.0f,
                               WHITE);
            }
        }

        // ---- TOP PIPE ----
        for (auto &p : pipes)
        {
            float neededHeight = p.top.height;

            if (neededHeight <= pipeTex.height)
            {
                // Slice from top downward (include cap)
                Rectangle pipe_source = {
                    0.0f,
                    0.0f,
                    (float)pipeTex.width,
                    neededHeight};

                // Flip vertically by using negative height
                Rectangle pipe_dest = {
                    p.top.x,
                    p.top.y + neededHeight, // move down to gap edge
                    p.top.width,
                    -neededHeight // grow upward
                };

                DrawTexturePro(pipeTex,
                               pipe_source,
                               pipe_dest,
                               {0, 0},
                               0.0f,
                               WHITE);
            }
            else
            {
                // If pipe taller than texture
                Rectangle pipe_source = {
                    0.0f,
                    0.0f,
                    (float)pipeTex.width,
                    (float)pipeTex.height};

                Rectangle pipe_dest = {
                    p.top.x,
                    p.top.y + p.top.height,
                    p.top.width,
                    -p.top.height};

                DrawTexturePro(pipeTex,
                               pipe_source,
                               pipe_dest,
                               {0, 0},
                               0.0f,
                               WHITE);
            }
        }

        //     for (auto &p : pipes)
        //   {
        /////////////////////////////////////////////////
        // Rectangle flipped_source = {
        //     0,
        //     (float)pipeTex.height,
        //     (float)pipeTex.width,
        //     -(float)pipeTex.height};

        // Rectangle pipe_dest_top = {
        //     p.top.x,
        //     p.top.y + p.top.height,
        //     p.top.width,
        //     p.top.height,
        // };

        // DrawTexturePro(pipeTex,
        //                flipped_source,
        //                pipe_dest_top,
        //                {0, 0},
        //                0.0f,
        //                WHITE);
        //       }
        EndDrawing();
    }
    UnloadTexture(birdTex);
    UnloadTexture(pipeTex);
    UnloadTexture(pipetopTex);
    CloseWindow();
    return 0;
}