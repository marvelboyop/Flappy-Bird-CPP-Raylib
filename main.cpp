#include "raylib.h"
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <fstream>

enum GameState
{
    STATE_START,
    STATE_PLAYING,
    STATE_GAMEOVER
};

struct Bird
{
    Rectangle rect;
    float velocity;
    float radius;
    float rotation;
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
    bird.radius = 24.0f;
    bird.rotation = 0.0f;

    const float gravity = 900.0f;
    const float jumpForce = -300.0f;

    // Pipes
    std::vector<Pipe> pipes;
    const float pipeSpeed = 200.0f;
    const int pipeWidth = 60;
    const int gap = 130;

    const float CAP_TEX_H = 24.0f; // pixels of cap inside pipeTex
    const float CAP_DST_H = 24.0f; // how tall to draw the cap on screen

    float pipeTimer = 0;
    int score = 0;
    int bestScore = 0;

    // Load best score from file
    {
        std::ifstream inFile("bestscore.txt");
        if (inFile.is_open())
        {
            inFile >> bestScore;
            inFile.close();
        }
    }

    GameState gameState = STATE_START;

    Texture2D birdTex = LoadTexture("opSprite.png");
    Texture2D pipeTex = LoadTexture("pipe.png");
    // Pre-flipped version for top pipes
    Image pipeImg = LoadImage("pipe.png");
    ImageFlipVertical(&pipeImg);
    Texture2D pipeFlippedTex = LoadTextureFromImage(pipeImg);
    UnloadImage(pipeImg); // free the CPU image, keep GPU texture

    // Background parallax layers
    Texture2D bgLayers[6];
    bgLayers[0] = LoadTexture("1.png"); // sky — far, slowest
    bgLayers[1] = LoadTexture("2.png");
    bgLayers[2] = LoadTexture("3.png");
    bgLayers[3] = LoadTexture("4.png");
    bgLayers[4] = LoadTexture("5.png");
    bgLayers[5] = LoadTexture("6.png"); // near, fastest

    // Scroll offsets and speeds for each layer
    float bgOffsets[6] = {0, 0, 0, 0, 0, 0};
    const float bgSpeeds[6] = {5.0f, 15.0f, 25.0f, 40.0f, 60.0f, 80.0f};

    // Scaled dimensions — fit width to screen, maintain aspect ratio
    const float bgW = (float)screenWidth;      // 400
    const float bgH = bgW * (324.0f / 576.0f); // ~225
    const float bgY = screenHeight - bgH;      // pin to bottom

    const int frameCount = 3; // number of frames in sprite sheet
    const int frameWidth = birdTex.width / frameCount;
    const int frameHeight = birdTex.height;

    int currentFrame = 0;
    float frameTime = 0.0f;
    const float frameDelay = 0.1f; // seconds per frame

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        // Scroll background layers on all screens
        for (int i = 0; i < 6; i++)
        {
            bgOffsets[i] -= bgSpeeds[i] * dt;
            if (bgOffsets[i] <= -bgW)
                bgOffsets[i] = 0;
        }

        /* ---------------- UPDATE ---------------- */
        if (gameState == STATE_START)
        {
            if (IsKeyPressed(KEY_ENTER))
                gameState = STATE_PLAYING;

            // Animate bird on start screen
            frameTime += dt;
            if (frameTime >= frameDelay)
            {
                frameTime = 0.0f;
                currentFrame = (currentFrame + 1) % frameCount;
            }
        }
        else if (gameState == STATE_PLAYING)
        {
            // Jump — snap to nose up instantly like classic flappy bird
            if (IsKeyPressed(KEY_SPACE))
            {
                bird.velocity = jumpForce;
                bird.rotation = -25.0f; // snap up on jump
            }

            // Gravity
            bird.velocity += gravity * dt; // 1s = 15 pixel drop per frame
            bird.rect.y += bird.velocity * dt;

            // Smoothly rotate down as bird falls
            if (bird.velocity > 0)
            {
                bird.rotation += 200.0f * dt; // rotate toward ground
                if (bird.rotation > 90.0f)
                    bird.rotation = 90.0f; // cap at nosedive
            }

            // Pipe spawn
            pipeTimer += dt;
            if (pipeTimer > 1.5f) // every 1.5 seconds pipe will be spawn
            {
                pipeTimer = 0;

                int topHeight = GetRandomValue(30, 300); // this is top pipe height
                Pipe p;
                p.top = {(float)screenWidth, 0, (float)pipeWidth, (float)topHeight};
                p.bottom = {(float)screenWidth, (float)(topHeight + gap),
                            (float)pipeWidth,
                            (float)(screenHeight - topHeight - gap)};
                p.passed = false;

                pipes.push_back(p);
            }

            Vector2 birdCenter = {
                bird.rect.x + bird.rect.width / 2.0f,
                bird.rect.y + bird.rect.height / 2.0f};

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

                    // update live as player beats it
                    if (score > bestScore)
                        bestScore = score; // just update variable
                }

                // Collision
                if (CheckCollisionCircleRec(birdCenter, bird.radius, p.top) ||
                    CheckCollisionCircleRec(birdCenter, bird.radius, p.bottom))
                {
                    gameState = STATE_GAMEOVER;
                    // Save to file on game over
                    std::ofstream outFile("bestscore.txt");
                    if (outFile.is_open())
                    {
                        outFile << bestScore;
                        outFile.close();
                    }
                }
            }

            pipes.erase(
                std::remove_if(pipes.begin(), pipes.end(),
                               [](const Pipe &p)
                               { return p.top.x + p.top.width < 0; }),
                pipes.end());

            // Ground & ceiling collision
            if (birdCenter.y - bird.radius < 0 ||
                birdCenter.y + bird.radius > screenHeight)
            {
                gameState = STATE_GAMEOVER;
                // Save to file on game over
                std::ofstream outFile("bestscore.txt");
                if (outFile.is_open())
                {
                    outFile << bestScore;
                    outFile.close();
                }
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
        else if (gameState == STATE_GAMEOVER)
        {
            // Restart
            if (IsKeyPressed(KEY_R))
            {
                bird.rect.y = 200;
                bird.velocity = 0;
                pipes.clear();
                pipeTimer = 0;
                score = 0;
                currentFrame = 0;
                frameTime = 0.0f;
                bird.rotation = 0.0f;
                gameState = STATE_START;
            }
        }

        /* ---------------- DRAW ---------------- */
        BeginDrawing();

        // Sky color fill for top portion not covered by bg image
        ClearBackground({255, 180, 150, 255}); // matches 1.png sky color

        // Draw all background layers — each drawn twice for seamless scroll
        // Layer 0 (sky) fills full screen, others pin to bottom
        for (int i = 0; i < 6; i++)
        {
            float y = (i == 0) ? 0 : bgY;
            float h = (i == 0) ? (float)screenHeight : bgH;

            // First copy
            Rectangle src = {0, 0, (float)bgLayers[i].width, (float)bgLayers[i].height};
            Rectangle dst1 = {bgOffsets[i], y, bgW, h};
            DrawTexturePro(bgLayers[i], src, dst1, {0, 0}, 0.0f, WHITE);

            // Second copy seamlessly follows the first
            Rectangle dst2 = {bgOffsets[i] + bgW, y, bgW, h};
            DrawTexturePro(bgLayers[i], src, dst2, {0, 0}, 0.0f, WHITE);
        }
        // ↑ background loop ends here

        if (gameState == STATE_START)
        {
            // Title
            DrawText("FLAPPY BIRD", 70, 150, 40, RED);

            // Bird sprite displayed on start screen
            Rectangle startSource = {(float)(currentFrame * frameWidth), 0,
                                     (float)frameWidth, (float)frameHeight};
            Rectangle startDest = {170, 220, 60, 60};
            DrawTexturePro(birdTex, startSource, startDest, {0, 0}, 0.0f, WHITE);

            // Best score
            DrawText(TextFormat("Best: %d", bestScore), 160, 310, 20, BLACK);

            // Instructions
            DrawText("Press ENTER to Start", 75, 360, 20, BLACK);
        }
        else
        {
            Rectangle source = {
                (float)(currentFrame * frameWidth),
                0,
                (float)frameWidth,
                (float)frameHeight};

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

            // Bird AFTER pipes so it always appears in front
            // Origin set to center so rotation happens around the bird's middle
            Vector2 origin = {bird.rect.width / 2.0f, bird.rect.height / 2.0f};
            Rectangle dest = {
                bird.rect.x + bird.rect.width / 2.0f,
                bird.rect.y + bird.rect.height / 2.0f,
                bird.rect.width,
                bird.rect.height};
            DrawTexturePro(birdTex, source, dest, origin, bird.rotation, WHITE);

            // UI — always drawn last so it appears on top of everything
            DrawText(TextFormat("Score: %d", score), 20, 20, 20, BLACK);
            DrawText(TextFormat("Best: %d", bestScore), screenWidth - 110, 20, 20, BLACK);

            if (gameState == STATE_GAMEOVER)
            {
                DrawText("GAME OVER", 110, 250, 30, RED);
                DrawText("Press R to Restart", 95, 290, 20, BLACK);
            }
        }

        EndDrawing();
    } // end of while loop

    // Save best score when game exits (Esc or close button)
    std::ofstream outFile("bestscore.txt");
    if (outFile.is_open())
    {
        outFile << bestScore;
        outFile.close();
    }

    UnloadTexture(birdTex);
    UnloadTexture(pipeTex);
    UnloadTexture(pipeFlippedTex);
    for (int i = 0; i < 6; i++)
        UnloadTexture(bgLayers[i]);
    CloseWindow();
    return 0;
}