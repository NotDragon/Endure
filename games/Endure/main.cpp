#include "declarations.h"
#include "json.hpp"
#include "raylib.h"
#include "raymath.h"
#include "resources.h"
#include "rlgl.h"
#include "structs.h"
#include <iostream>
#include <string>
#include "pathfinders.h"
#include "save_system.h"

// toggle between debug mode and normal mode
//#define DEBUG

using json = nlohmann::json;

#pragma region Main
int main() {
    // clear the chest vector
    chests.clear();

    if (SCREEN_HEIGHT > SCREEN_WIDTH) {
        BLOCK_WIDTH = SCREEN_WIDTH / 20;
        BLOCK_HEIGHT = SCREEN_WIDTH / 20;
    } else {
        BLOCK_WIDTH = SCREEN_HEIGHT / 20;
        BLOCK_HEIGHT = SCREEN_HEIGHT / 20;
    }
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    // initialize raylib and the screen
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Endure");
    SetWindowState(FLAG_VSYNC_HINT);
    // initialize audio
    InitAudioDevice();

    screen = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);

    // load main music
    Sound mainMusic = LoadSound("./resources/sounds/soundtrack/000.wav");

    InitResources();

    exit_button = LoadTexture("./resources/UI/buttons/exit_button.png");
    exit_button_pressed = LoadTexture("./resources/UI/buttons/exit_button_pressed.png");
    back_button = LoadTexture("./resources/UI/buttons/back_button.png");
    back_button_pressed = LoadTexture("./resources/UI/buttons/back_button_pressed.png");

    FatPixel = LoadFont("./resources/UI/fonts/FATPIXEL.TTF");

    SetExitKey(KEY_NULL);

    // initialize camera
    cam.zoom = 1;
    character.Init();

    // initialize character default fields
    character.x = BLOCK_WIDTH;
    character.y = BLOCK_HEIGHT * 2;
    character.radius = BLOCK_WIDTH / 2.7f;
    character.speedX = 300;
    character.speedY = 300;
    character.selectedItemSlot = 0;
    SetSoundVolume(character.attackingSoundeffect, 2.0f);

    // initialize hotbar
    for (int i = 0; i < 6; i++) {
        character.hotbarItems[i].id = -1;
    }

    // hide curser
    HideCursor();

    // load the custom curser texture
    cursor.texture = textures["MOUSE"];
    SetTargetFPS(60);

    // main loop
    while (!WindowShouldClose() && shouldBePlaying) {
		if(IsKeyPressed(KEY_P)){
			save();
		}else if(IsKeyPressed(KEY_L)){
			load();
		}
        // set curser possition
        cursor.x = GetMouseX() - BLOCK_WIDTH / (cursor.texture.width == 0 ? 1 : cursor.texture.width) / 2;
        cursor.y = GetMouseY() - BLOCK_HEIGHT / (cursor.texture.height == 0 ? 1 : cursor.texture.height) / 2;
        // set idle to true
        character.isIdle = true;

        character.maxAbilities = 3 + (character.stamina / 10);

        if (character.abilities < character.maxAbilities) {
            character.abilities += GetFrameTime() / (1.5f - character.stamina * 0.1f);
        } else {
            character.abilities = character.maxAbilities;
        }

        // x and y of player at the start of the frame(used for colition)
        int tempX = character.x;
        int tempY = character.y;
        // check if the hotbar changed
        if (GetMouseWheelMove() >= 1)
            // increment the slot
            character.selectedItemSlot++;
        else if (GetMouseWheelMove() <= -1)
            // decrament the slot
            character.selectedItemSlot--;
        if (character.selectedItemSlot < 0)
            // corent ite's value
            character.selectedItemSlot = 5;

        // prevent overflow
        character.selectedItemSlot %= 6;
        // check if the player should move
        bool isMovementDiagonal = false;
        if (IsKeyDown(KEY_W)) {
            // set idle to false
            character.isIdle = false;

            // move player while acounting for diagnal movement
            if (IsKeyDown(KEY_D)) {
                isMovementDiagonal = true;
                // change the direction in whichg the player is facing(to the right)
                character.facing = 0;

                // change x and y equaly
                character.x += character.speedX / 1.5f * GetFrameTime();
                character.y -= character.speedY / 1.5f * GetFrameTime();
            } else if (IsKeyDown(KEY_A)) {
                isMovementDiagonal = true;

                // change the direction in whichg the player is facing(to the left)
                character.facing = 1;

                // change x and y equaly
                character.x -= character.speedX / 1.5f * GetFrameTime();
                character.y -= character.speedY / 1.5f * GetFrameTime();
            } else {
                // change the player y possition
                character.y -= character.speedY * GetFrameTime();
            }
        } else if (IsKeyDown(KEY_S)) {
            // set idle to false
            character.isIdle = false;

            // move player while acounting for diagnal movement
            if (IsKeyDown(KEY_D)) {
                isMovementDiagonal = true;

                // change the direction in whichg the player is facing(to the right)
                character.facing = 0;

                // change x and y equaly
                character.x += character.speedX / 1.5f * GetFrameTime();
                character.y += character.speedY / 1.5f * GetFrameTime();
            } else if (IsKeyDown(KEY_A)) {
                isMovementDiagonal = true;

                // change the direction in whichg the player is facing(to the left)
                character.facing = 1;

                // change x and y equaly
                character.x -= character.speedX / 1.5f * GetFrameTime();
                character.y += character.speedY / 1.5f * GetFrameTime();
            } else {
                // change the player y possition
                character.y += character.speedY * GetFrameTime();
            }
        } else if (IsKeyDown(KEY_D)) {
            // change the direction in whichg the player is facing(to the right)
            character.facing = 0;
            // set idle to false
            character.isIdle = false;
            // change the x possition of the player
            character.x += character.speedX * GetFrameTime();
        } else if (IsKeyDown(KEY_A)) {
            // change the direction in whichg the player is facing(to the left)
            character.facing = 1;
            // set idle to false
            character.isIdle = false;
            // change the x possition of the player
            character.x -= character.speedX * GetFrameTime();
        }

        if (!isAttacked) {
            if (cam.offset.x > -character.x + SCREEN_WIDTH / 2) {
                if (cam.offset.x - 5 >= -character.x + SCREEN_WIDTH / 2)
                    if (isMovementDiagonal) {
                        cam.offset.x -= 2.5f;
                    } else {
                        cam.offset.x -= 5;
                    }

            } else if (cam.offset.x < -character.x + SCREEN_WIDTH / 2) {
                if (cam.offset.x + 5 <= -character.x + SCREEN_WIDTH / 2)
                    if (isMovementDiagonal) {
                        cam.offset.x += 2.5f;
                    } else {
                        cam.offset.x += 5;
                    }
            }
            cam.offset.y = 0;
        } else
            ScreenShake(6);
        // check for collition sideways
        for (auto collider: collidersX) {
            // check if the player is colliding the all x colliders
            if (character.CheckCollision(collider)) {
                // sets players x possition to the possition at the start of the frame
                character.x = tempX;
            }
        }

        // check for collition upwards or downwards
        for (auto collider: collidersY) {
            // check if the player is colliding the all y colliders
            if (character.CheckCollision(collider)) {
                // sets players y possition to the position at the start of the frame
                character.y = tempY;
            }
        }

        if (IsKeyPressed(KEY_SPACE)) {
            character.Dash();
        }else if(IsKeyPressed(KEY_Z)) {
            character.EnemySwap();
        }

        if (isPaused) {
            shader = shaders["BLUR"];
            isShaderActive = true;
        } else {
            isShaderActive = false;
        }

#ifdef DEBUG
        if (IsKeyPressed(KEY_KP_1)) {
            character.Damage(1);
        }
        if (IsKeyPressed(KEY_KP_2)) {
            character.Damage(2);
        }
        if (IsKeyPressed(KEY_KP_3)) {
            character.Damage(3);
        }
        if (IsKeyPressed(KEY_KP_4)) {
            character.Damage(4);
        }
        if (IsKeyPressed(KEY_KP_5)) {
            character.Damage(5);
        }
        if (IsKeyPressed(KEY_KP_6)) {
            character.Damage(6);
        }
        if (IsKeyPressed(KEY_KP_7)) {
            character.Damage(7);
        }
        if (IsKeyPressed(KEY_KP_8)) {
            character.Damage(8);
        }
        if (IsKeyPressed(KEY_KP_9)) {
            character.Damage(9);
        }
        if (IsKeyPressed(KEY_KP_0)) {
            character.Damage(10);
        }
        if (IsKeyPressed(KEY_M)) {
            StopSound(mainMusic);
            short musicId = GetRandomValue(0, 28);

            if (musicId == 0)
                mainMusic = LoadSound("./resources/sounds/soundtrack/000.wav");
            if (musicId == 1)
                mainMusic = LoadSound("./resources/sounds/soundtrack/001.wav");
            if (musicId == 2)
                mainMusic = LoadSound("./resources/sounds/soundtrack/002.wav");
            if (musicId == 3)
                mainMusic = LoadSound("./resources/sounds/soundtrack/003.wav");
            if (musicId == 4)
                mainMusic = LoadSound("./resources/sounds/soundtrack/004.wav");
            if (musicId == 5)
                mainMusic = LoadSound("./resources/sounds/soundtrack/005.wav");
            if (musicId == 6)
                mainMusic = LoadSound("./resources/sounds/soundtrack/006.wav");
            if (musicId == 7)
                mainMusic = LoadSound("./resources/sounds/soundtrack/007.wav");
            if (musicId == 8)
                mainMusic = LoadSound("./resources/sounds/soundtrack/008.wav");
            if (musicId == 9)
                mainMusic = LoadSound("./resources/sounds/soundtrack/009.wav");
            if (musicId == 10)
                mainMusic = LoadSound("./resources/sounds/soundtrack/010.wav");
            if (musicId == 11)
                mainMusic = LoadSound("./resources/sounds/soundtrack/011.wav");
            if (musicId == 12)
                mainMusic = LoadSound("./resources/sounds/soundtrack/012.wav");
            if (musicId == 13)
                mainMusic = LoadSound("./resources/sounds/soundtrack/013.wav");
            if (musicId == 14)
                mainMusic = LoadSound("./resources/sounds/soundtrack/014.wav");
            if (musicId == 15)
                mainMusic = LoadSound("./resources/sounds/soundtrack/015.wav");
            if (musicId == 16)
                mainMusic = LoadSound("./resources/sounds/soundtrack/016.wav");
            if (musicId == 17)
                mainMusic = LoadSound("./resources/sounds/soundtrack/017.wav");
            if (musicId == 18)
                mainMusic = LoadSound("./resources/sounds/soundtrack/018.wav");
            if (musicId == 19)
                mainMusic = LoadSound("./resources/sounds/soundtrack/019.wav");
            if (musicId == 20)
                mainMusic = LoadSound("./resources/sounds/soundtrack/020.wav");
            if (musicId == 21)
                mainMusic = LoadSound("./resources/sounds/soundtrack/021.wav");
            if (musicId == 22)
                mainMusic = LoadSound("./resources/sounds/soundtrack/022.wav");
            if (musicId == 23)
                mainMusic = LoadSound("./resources/sounds/soundtrack/023.wav");
            if (musicId == 25)
                mainMusic = LoadSound("./resources/sounds/soundtrack/025.wav");
            if (musicId == 26)
                mainMusic = LoadSound("./resources/sounds/soundtrack/026.wav");
            if (musicId == 27)
                mainMusic = LoadSound("./resources/sounds/soundtrack/027.wav");
            if (musicId == 28)
                mainMusic = LoadSound("./resources/sounds/soundtrack/028.wav");

            // set the main music's volume
            SetSoundVolume(mainMusic, 0.3f);
            PlaySound(mainMusic);
        }

#endif
        if (!IsSoundPlaying(mainMusic)) {
            short musicId = GetRandomValue(0, 28);

            if (musicId == 0)
                mainMusic = LoadSound("./resources/sounds/soundtrack/000.wav");
            if (musicId == 1)
                mainMusic = LoadSound("./resources/sounds/soundtrack/001.wav");
            if (musicId == 2)
                mainMusic = LoadSound("./resources/sounds/soundtrack/002.wav");
            if (musicId == 3)
                mainMusic = LoadSound("./resources/sounds/soundtrack/003.wav");
            if (musicId == 4)
                mainMusic = LoadSound("./resources/sounds/soundtrack/004.wav");
            if (musicId == 5)
                mainMusic = LoadSound("./resources/sounds/soundtrack/005.wav");
            if (musicId == 6)
                mainMusic = LoadSound("./resources/sounds/soundtrack/006.wav");
            if (musicId == 7)
                mainMusic = LoadSound("./resources/sounds/soundtrack/007.wav");
            if (musicId == 8)
                mainMusic = LoadSound("./resources/sounds/soundtrack/008.wav");
            if (musicId == 9)
                mainMusic = LoadSound("./resources/sounds/soundtrack/009.wav");
            if (musicId == 10)
                mainMusic = LoadSound("./resources/sounds/soundtrack/010.wav");
            if (musicId == 11)
                mainMusic = LoadSound("./resources/sounds/soundtrack/011.wav");
            if (musicId == 12)
                mainMusic = LoadSound("./resources/sounds/soundtrack/012.wav");
            if (musicId == 13)
                mainMusic = LoadSound("./resources/sounds/soundtrack/013.wav");
            if (musicId == 14)
                mainMusic = LoadSound("./resources/sounds/soundtrack/014.wav");
            if (musicId == 15)
                mainMusic = LoadSound("./resources/sounds/soundtrack/015.wav");
            if (musicId == 16)
                mainMusic = LoadSound("./resources/sounds/soundtrack/016.wav");
            if (musicId == 17)
                mainMusic = LoadSound("./resources/sounds/soundtrack/017.wav");
            if (musicId == 18)
                mainMusic = LoadSound("./resources/sounds/soundtrack/018.wav");
            if (musicId == 19)
                mainMusic = LoadSound("./resources/sounds/soundtrack/019.wav");
            if (musicId == 20)
                mainMusic = LoadSound("./resources/sounds/soundtrack/020.wav");
            if (musicId == 21)
                mainMusic = LoadSound("./resources/sounds/soundtrack/021.wav");
            if (musicId == 22)
                mainMusic = LoadSound("./resources/sounds/soundtrack/022.wav");
            if (musicId == 23)
                mainMusic = LoadSound("./resources/sounds/soundtrack/023.wav");
            if (musicId == 25)
                mainMusic = LoadSound("./resources/sounds/soundtrack/025.wav");
            if (musicId == 26)
                mainMusic = LoadSound("./resources/sounds/soundtrack/026.wav");
            if (musicId == 27)
                mainMusic = LoadSound("./resources/sounds/soundtrack/027.wav");
            if (musicId == 28)
                mainMusic = LoadSound("./resources/sounds/soundtrack/028.wav");

            // set the main music's volume
            SetSoundVolume(mainMusic, 0.3f);
            PlaySound(mainMusic);
        }
        // set character/mainitem rotation
        character.rotation = PI / 2 + atan2(character.y - cursor.y, character.x - (cursor.x - cam.offset.x));
        if (character.goal != 0) {
            for (auto &enemy: enemies) {
                if (CheckCollisionRecs(character.itemCollider, enemy.collider) && character.attacking) {
                    enemy.Damage((character.hotbarItems[character.selectedItemSlot].damage + character.strength * 2) * 22.0f);
                    character.hotbarItems[character.selectedItemSlot].durability--;
                    PlaySoundMulti(character.attackingSoundeffect);
                    ScreenShake(3);
                    attacked = true;
                }
            }

            for (auto &NHC: NHCs) {
                if (CheckCollisionRecs(character.itemCollider, NHC.collider) && character.attacking) {
                    NHC.Damage((character.hotbarItems[character.selectedItemSlot].damage + character.strength * 2) * 22.0f);
                    character.hotbarItems[character.selectedItemSlot].durability--;
                    PlaySoundMulti(character.attackingSoundeffect);
                    ScreenShake(3);
                    attacked = true;
                }
            }
        }

        if (attacked) {
            character.attacking = false;
        }

        if (character.attacking) {
            attacked = false;
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            if (isPaused) {
                isPaused = false;
            } else {
                isPaused = true;
            }
        }


        BeginTextureMode(screen);
        {
            ClearBackground(Color{52, 116, 235, 100});
            // begin the camera
            BeginMode2D(cam);
            // load the correct room
            loadRoom(currentRoom);
            for (auto door: doors) {
                if (character.CheckCollision(door.first)) {
                    if (IsKeyPressed(KEY_E)) {
                        levelData = "";
                        level.close();
                        currentRoom = door.second;
                        playerSpawned = false;
                    } else
                        DrawImage(door.first.x * 1.0f, door.first.y * 1.0f - BLOCK_HEIGHT, textures["E_KEY"]);
                }
#ifdef DEBUG
                //DrawRectangleRec(door.first, Color{232, 52, 235, 90});
#endif
            }
            character.DrawPlayerUI();

            character.Draw();
            character.DrawMainHand();
#ifdef DEBUG
            DrawCircle(character.x, character.y + (character.texture.height * BLOCK_WIDTH / (character.texture.height == 0 ? 1 : character.texture.height)) / 4.0f, character.radius, WHITE);
            int size = 5000;
            // draw a grid (20x20)
            for (float i = -size; i <= size; i += SCREEN_HEIGHT / 20) {
                DrawLine(i, -size, i, size, GRAY);
                DrawLine(-size, i, size, i, GRAY);
            }
            DrawFPS(10, 40);
            // display the mainitems rotation
            DrawText(std::to_string(atan2(character.y - cursor.y, character.x - (cursor.x - cam.offset.x))).c_str(), 10, 70,
                     22, DARKGREEN);

            DrawText(std::to_string(character.abilities).c_str(), 100, 100, 20, DARKGREEN);
#endif
        }
        EndTextureMode();

        BeginDrawing();
        if (isShaderActive) {
            BeginShaderMode(shader);
            DrawTextureRec(screen.texture,
                           (Rectangle){0, 0, (float) screen.texture.width, (float) -screen.texture.height},
                           (Vector2){0, 0}, WHITE);
            EndShaderMode();
        } else {
            DrawTextureRec(screen.texture,
                           (Rectangle){0, 0, (float) screen.texture.width, (float) -screen.texture.height},
                           (Vector2){0, 0}, WHITE);
        }
        if (isPaused)
            DrawPauseMenu();
        cursor.Draw(Color{184, 81, 55, 255});

        EndDrawing();
    }

    // unload textures/sounds
    StopSoundMulti();
    UnloadTexture(character.texture);
    for (auto &texture: textures) {
        UnloadTexture(texture.second);
    }
    for (auto &f: fx) {
        UnloadSound(f.second);
    }
    for (auto &statIcon: statIcons) {
        UnloadTexture(statIcon.second);
    }
    UnloadTexture(character.texture);
    for (auto &i: characterAnimationIdle) {
        UnloadTexture(i.second);
    }
    for (auto &i: characterAnimationWalk) {
        UnloadTexture(i.second);
    }
    for (auto &i: buttons) {
        UnloadTexture(i.second);
    }
    for (auto &i: playerUI) {
        UnloadTexture(i.second);
    }
    for (auto &i: particles) {
        UnloadTexture(i.second);
    }
    for (auto &i: enemyTextures) {
        for (auto &j: i.second.first) {
            UnloadTexture(j);
        }

        for (auto &j: i.second.second) {
            UnloadTexture(j);
        }
    }

    UnloadSound(mainMusic);
    // stop audio device
    CloseAudioDevice();
    // close window
    CloseWindow();

    return 0;
}
#pragma endregion Main


#pragma region Functions

float clickAnimation = 0.0f;
int clickedButton = 0;
int alpha = 230;

/// @brief Draw image in the grid
/// @param x 0-20 (in grid)
/// @param y  0-20 (in grid)
/// @param texture
void DrawImage(float x, float y, Texture2D texture) {
    DrawTexturePro(texture, Rectangle{0, 0, texture.width * 1.0f, texture.height * 1.0f}, Rectangle{(x - 0.6f) * BLOCK_WIDTH, (y + 0.4f) * BLOCK_HEIGHT, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 1.0f}, Vector2{texture.width * 0.5f, texture.height * 0.5f}, 0, RAYWHITE);
}
void DrawTextureWithCollider(float x, float y, Texture2D texture) {
    DrawTexturePro(texture, Rectangle{0, 0, texture.width * 1.0f, texture.height * 1.0f}, Rectangle{(x - 0.6f) * BLOCK_WIDTH, (y + 0.4f) * BLOCK_HEIGHT, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 1.0f}, Vector2{texture.width * 0.5f, texture.height * 0.5f}, 0, RAYWHITE);

    x -= 1;

    collidersX.push_back(Rectangle{BLOCK_WIDTH * x, BLOCK_HEIGHT * y, BLOCK_WIDTH * 0.3f, BLOCK_HEIGHT * 1.0f});
    collidersY.push_back(Rectangle{BLOCK_WIDTH * x, BLOCK_HEIGHT * y, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 0.3f});

    collidersX.push_back(Rectangle{BLOCK_WIDTH * x + 0.7f * BLOCK_WIDTH, BLOCK_HEIGHT * y, BLOCK_WIDTH * 0.3f, BLOCK_HEIGHT * 1.0f});
    collidersY.push_back(Rectangle{BLOCK_WIDTH * x, BLOCK_HEIGHT * y + 0.7f * BLOCK_HEIGHT, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 0.3f});

#ifdef DEBUG
    DrawRectangleRec(Rectangle{BLOCK_WIDTH * x + 0.7f * BLOCK_WIDTH, BLOCK_HEIGHT * y, BLOCK_WIDTH * 0.3f, BLOCK_HEIGHT * 1.0f}, YELLOW);
    DrawRectangleRec(Rectangle{BLOCK_WIDTH * x, BLOCK_HEIGHT * y + 0.7f * BLOCK_HEIGHT, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 0.3f}, GREEN);
#endif
}

void DrawTextureWithCollider(float x, float y, Texture2D texture, float widthScale) {
    DrawTexturePro(texture, Rectangle{0, 0, texture.width * 1.0f, texture.height * 1.0f}, Rectangle{(x - 0.6f) * BLOCK_WIDTH, (y + 0.4f) * BLOCK_HEIGHT, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 1.0f}, Vector2{texture.width * 0.5f, texture.height * 0.5f}, 0, RAYWHITE);
    x -= 1;
    if (widthScale > 0) {
        collidersX.push_back(Rectangle{BLOCK_WIDTH * x, BLOCK_HEIGHT * y, BLOCK_WIDTH / widthScale, BLOCK_HEIGHT * 1.0f});
        collidersY.push_back(Rectangle{BLOCK_WIDTH * x, BLOCK_HEIGHT * y, BLOCK_WIDTH / widthScale, BLOCK_HEIGHT * 1.0f});
    } else {
        widthScale = -widthScale;
        collidersX.push_back(Rectangle{BLOCK_WIDTH * x + (BLOCK_WIDTH / widthScale * widthScale - 1), BLOCK_HEIGHT * y, BLOCK_WIDTH / widthScale, BLOCK_HEIGHT * 1.0f});
        collidersY.push_back(Rectangle{BLOCK_WIDTH * x + (BLOCK_WIDTH / widthScale * widthScale - 1), BLOCK_HEIGHT * y, BLOCK_WIDTH / widthScale, BLOCK_HEIGHT * 1.0f});
    }
}


void DrawPauseMenu() {
    if (character.isIdle && alpha == 100)
        alpha = 230;
    else if (!character.isIdle && alpha == 230)
        alpha = 100;

    DrawRectangleRounded(Rectangle{BLOCK_WIDTH * 5.0f - 10, BLOCK_HEIGHT * 2.0f - 10, BLOCK_WIDTH * 10.0f + 20, BLOCK_HEIGHT * 16.0f + 20}, 0.02f, 100, Color{63, 40, 50, static_cast<unsigned char>(alpha)});
    DrawRectangleRounded(Rectangle{BLOCK_WIDTH * 5.0f - 5, BLOCK_HEIGHT * 2.0f - 5, BLOCK_WIDTH * 10.0f + 10, BLOCK_HEIGHT * 16.0f + 10}, 0.02f, 100, Color{228, 166, 114, static_cast<unsigned char>(alpha)});
    DrawRectangleRounded(Rectangle{BLOCK_WIDTH * 5.0f, BLOCK_HEIGHT * 2.0f, BLOCK_WIDTH * 10.0f, BLOCK_HEIGHT * 16.0f}, 0.02f, 100, Color{234, 212, 170, static_cast<unsigned char>(alpha)});

    if ((CheckCollisionRecs(Rectangle{cursor.x - (cursor.texture.width * BLOCK_WIDTH / (cursor.texture.width == 0 ? 1 : cursor.texture.width)) / 2, cursor.y - cursor.texture.height * BLOCK_HEIGHT / (cursor.texture.height == 0 ? 1 : cursor.texture.height) / 2, static_cast<float>(cursor.texture.width * BLOCK_HEIGHT / (cursor.texture.height == 0 ? 1 : cursor.texture.height)), static_cast<float>(cursor.texture.height * BLOCK_HEIGHT / (cursor.texture.height == 0 ? 1 : cursor.texture.height))}, Rectangle{SCREEN_WIDTH / 2 - (back_button.width * BLOCK_WIDTH / (back_button.width == 0 ? 1 : back_button.width)) / 2.0f, BLOCK_WIDTH * 3.0f, back_button.width * 1.0f, back_button.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
        DrawTexture(back_button_pressed, SCREEN_WIDTH / 2 - back_button_pressed.width * BLOCK_WIDTH * 3 / (back_button_pressed.width == 0 ? 1 : back_button_pressed.width) / 2, SCREEN_WIDTH * 0.15f + BLOCK_HEIGHT * 14 / back_button_pressed.width, Color{255, 255, 255, static_cast<unsigned char>(alpha)});

        clickAnimation = 0.25f;
        clickedButton = 1;
    } else if (clickAnimation < -0.1f && clickedButton == 1) {
        alpha -= 19;
    } else if (clickAnimation > 0 && clickedButton == 1) {
        clickAnimation -= GetFrameTime();
        DrawTexture(back_button_pressed, SCREEN_WIDTH / 2 - back_button_pressed.width * BLOCK_WIDTH * 3 / (back_button_pressed.width == 0 ? 1 : back_button_pressed.width) / 2, SCREEN_WIDTH * 0.15f + BLOCK_HEIGHT * 14 / back_button_pressed.width, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
    } else if (clickAnimation < 0) {
        clickAnimation -= GetFrameTime();
        DrawTexture(back_button, SCREEN_WIDTH / 2 - back_button.width * BLOCK_WIDTH * 3 / (back_button.width == 0 ? 1 : back_button.width) / 2, SCREEN_WIDTH * 0.15f, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
    } else
        DrawTexture(back_button, SCREEN_WIDTH / 2 - back_button.width * BLOCK_WIDTH * 3 / (back_button.width == 0 ? 1 : back_button.width) / 2, SCREEN_WIDTH * 0.15f, Color{255, 255, 255, static_cast<unsigned char>(alpha)});

    if (alpha < 10) {
        isPaused = false;
        clickedButton = 0;
        alpha = 230;
    }

    if ((CheckCollisionRecs(Rectangle{cursor.x - cursor.texture.width * BLOCK_WIDTH / (cursor.texture.width == 0 ? 1 : cursor.texture.width) / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * BLOCK_WIDTH / (cursor.texture.width == 0 ? 1 : cursor.texture.width) * 1.0f, cursor.texture.height * BLOCK_HEIGHT / (cursor.texture.height == 0 ? 1 : cursor.texture.height) * 1.0f}, Rectangle{SCREEN_WIDTH / 2 - back_button.width / 2.0f, BLOCK_WIDTH * 5.0f, back_button.width * 1.0f, back_button.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
        DrawTexture(exit_button_pressed, SCREEN_WIDTH / 2 - exit_button_pressed.width * BLOCK_WIDTH * 3 / (exit_button_pressed.width == 0 ? 1 : exit_button_pressed.width) / 2, SCREEN_HEIGHT * 0.25f + BLOCK_HEIGHT * 14 / exit_button_pressed.width, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
        clickAnimation = 0.25f;
        clickedButton = 2;
    } else if (clickAnimation < -0.15f && clickedButton == 2) {
        shouldBePlaying = false;
        clickedButton = 0;
    } else if (clickAnimation > 0 && clickedButton == 2) {
        clickAnimation -= GetFrameTime();
        DrawTexture(exit_button_pressed, SCREEN_WIDTH / 2 - exit_button_pressed.width * BLOCK_WIDTH * 3 / (exit_button_pressed.width == 0 ? 1 : exit_button_pressed.width) / 2, SCREEN_HEIGHT * 0.25f + BLOCK_HEIGHT * 14 / exit_button_pressed.width, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
    } else if (clickAnimation < 0) {
        clickAnimation -= GetFrameTime();
        DrawTexture(exit_button, SCREEN_WIDTH / 2 - exit_button.width * BLOCK_WIDTH * 3 / (exit_button.width == 0 ? 1 : exit_button.width) / 2, SCREEN_HEIGHT * 0.25f, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
    } else
        DrawTexture(exit_button, SCREEN_WIDTH / 2 - exit_button.width * BLOCK_WIDTH * 3 / (exit_button.width == 0 ? 1 : exit_button.width) / 2, SCREEN_HEIGHT * 0.25f, Color{255, 255, 255, static_cast<unsigned char>(alpha)});

    DrawTexture(statIcons["AGILITY_ICON"], BLOCK_WIDTH * 5.5f, BLOCK_HEIGHT * 8, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
    DrawTexture(statIcons["DEFENCE_ICON"], BLOCK_WIDTH * 5.5f, BLOCK_HEIGHT * 9.25f, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
    DrawTexture(statIcons["HEALTH_ICON"], BLOCK_WIDTH * 5.5f, BLOCK_HEIGHT * 10.5f, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
    DrawTexture(statIcons["LUCK_ICON"], BLOCK_WIDTH * 5.5f, BLOCK_HEIGHT * 11.75f, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
    DrawTexture(statIcons["M_DEFENCE_ICON"], BLOCK_WIDTH * 5.5f, BLOCK_HEIGHT * 13, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
    DrawTexture(statIcons["STAMINA_ICON"], BLOCK_WIDTH * 5.5f, BLOCK_HEIGHT * 14.25f, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
    DrawTexture(statIcons["STRENGTH_ICON"], BLOCK_WIDTH * 5.5f, BLOCK_HEIGHT * 15.5f, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
    DrawTexture(statIcons["VISABILITY_ICON"], BLOCK_WIDTH * 5.5f, BLOCK_HEIGHT * 16.75f, Color{255, 255, 255, static_cast<unsigned char>(alpha)});

    // DrawText(std::to_string(character.agility).c_str(), BLOCK_WIDTH * 13, BLOCK_WIDTH * 8, 25, Color{255, 255, 255, alpha});

    if (character.statPoints != 0) {
        DrawTextEx(FatPixel, std::to_string(character.tempAgility).c_str(), Vector2{BLOCK_WIDTH * 8.0f, BLOCK_HEIGHT * 8.0f}, 25, 1, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, std::to_string(character.tempDefence).c_str(), Vector2{BLOCK_WIDTH * 8.0f, BLOCK_HEIGHT * 9.25f}, 25, 1, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, std::to_string(character.tempMaxHealth).c_str(), Vector2{BLOCK_WIDTH * 8.0f, BLOCK_HEIGHT * 10.5f}, 25, 1, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, std::to_string(character.tempLuck).c_str(), Vector2{BLOCK_WIDTH * 8.0f, BLOCK_HEIGHT * 11.75f}, 25, 1, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, std::to_string(character.tempM_defence).c_str(), Vector2{BLOCK_WIDTH * 8.0f, BLOCK_HEIGHT * 13.0f}, 25, 1, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, std::to_string(character.tempStamina).c_str(), Vector2{BLOCK_WIDTH * 8.0f, BLOCK_HEIGHT * 14.25f}, 25, 1, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, std::to_string(character.tempStrength).c_str(), Vector2{BLOCK_WIDTH * 8.0f, BLOCK_HEIGHT * 15.5f}, 25, 1, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, std::to_string(character.tempVisability).c_str(), Vector2{BLOCK_WIDTH * 8.0f, BLOCK_HEIGHT * 16.75f}, 25, 1, Color{255, 255, 255, static_cast<unsigned char>(alpha)});

        if (character.tempAgility != character.agility)
            DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 8.0f}, BLOCK_HEIGHT / 2, 1, Color{3, 186, 252, static_cast<unsigned char>(alpha)});
        else
            DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 8.0f}, BLOCK_HEIGHT / 2, 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});

        if (character.tempDefence != character.defence)
            DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 9.25f}, BLOCK_HEIGHT / 2, 1, Color{3, 186, 252, static_cast<unsigned char>(alpha)});
        else
            DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 9.25f}, BLOCK_HEIGHT / 2, 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});

        if (character.tempMaxHealth != character.maxHealth)
            DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 10.5f}, BLOCK_HEIGHT / 2, 1, Color{3, 186, 252, static_cast<unsigned char>(alpha)});
        else
            DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 10.5f}, BLOCK_HEIGHT / 2, 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});

        if (character.tempLuck != character.luck)
            DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 11.75f}, BLOCK_HEIGHT / 2, 1, Color{3, 186, 252, static_cast<unsigned char>(alpha)});
        else
            DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 11.75f}, BLOCK_HEIGHT / 2, 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});

        if (character.tempM_defence != character.m_defence)
            DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 13.0f}, 25, BLOCK_HEIGHT / 2, Color{3, 186, 252, static_cast<unsigned char>(alpha)});
        else
            DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 13.0f}, 25, BLOCK_HEIGHT / 2, Color{130, 130, 130, static_cast<unsigned char>(alpha)});

        if (character.tempStamina != character.stamina)
            DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 14.25f}, BLOCK_HEIGHT / 2, 1, Color{3, 186, 252, static_cast<unsigned char>(alpha)});
        else
            DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 14.25f}, BLOCK_HEIGHT / 2, 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});

        if (character.tempStrength != character.strength)
            DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 15.5f}, BLOCK_HEIGHT / 2, 1, Color{3, 186, 252, static_cast<unsigned char>(alpha)});
        else
            DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 15.5f}, BLOCK_HEIGHT / 2, 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});

        if (character.tempVisability != character.visability)
            DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 16.75f}, BLOCK_HEIGHT / 2, 1, Color{3, 186, 252, static_cast<unsigned char>(alpha)});
        else
            DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 16.75f}, BLOCK_HEIGHT / 2, 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});

        DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.tempAgility).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 8.0f}, 25, 1, Color{3, 186, 252, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.tempDefence).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 9.25f}, 25, 1, Color{3, 186, 252, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.tempMaxHealth).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 10.5f}, 25, 1, Color{3, 186, 252, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.tempLuck).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 11.75f}, 25, 1, Color{3, 186, 252, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.tempM_defence).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 13.0f}, 25, 1, Color{3, 186, 252, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.tempStamina).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 14.25f}, 25, 1, Color{3, 186, 252, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.tempStrength).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 15.5f}, 25, 1, Color{3, 186, 252, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.tempVisability).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 16.75f}, 25, 1, Color{3, 186, 252, static_cast<unsigned char>(alpha)});

        if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 8.65f + (std::to_string(character.agility).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 8.25f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempStatPoints != 0) {
            character.tempStatPoints--;
            character.tempAgility++;
        } else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 8.65f + (std::to_string(character.tempDefence).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_WIDTH * 9.5f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempStatPoints != 0) {
            character.tempStatPoints--;
            character.tempDefence++;
        } else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 8.65f + (std::to_string(character.tempMaxHealth).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 10.75f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempStatPoints != 0) {
            character.tempStatPoints--;
            character.tempMaxHealth++;
        } else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 8.65f + (std::to_string(character.tempLuck).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 12.0f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempStatPoints != 0) {
            character.tempStatPoints--;
            character.tempLuck++;
        } else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 8.65f + (std::to_string(character.tempM_defence).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 13.25f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempStatPoints != 0) {
            character.tempStatPoints--;
            character.tempM_defence++;
        } else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 8.65f + (std::to_string(character.tempStamina).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 14.5f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempStatPoints != 0) {
            character.tempStatPoints--;
            character.tempStamina++;
        } else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 8.65f + (std::to_string(character.tempStrength).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 15.75f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempStatPoints != 0) {
            character.tempStatPoints--;
            character.tempStrength++;
        } else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 8.65f + (std::to_string(character.tempVisability).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 17.0f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempStatPoints != 0) {
            character.tempStatPoints--;
            character.tempVisability++;
        }
        // fghj
        else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 7.65f, BLOCK_HEIGHT * 8.25f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempAgility > character.agility) {
            character.tempStatPoints++;
            character.tempAgility--;
        } else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 7.65f, BLOCK_HEIGHT * 9.5f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempDefence > character.defence) {
            character.tempStatPoints++;
            character.tempDefence--;
        } else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 7.65f, BLOCK_HEIGHT * 10.75f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempMaxHealth > character.health) {
            character.tempStatPoints++;
            character.tempMaxHealth--;
        } else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 7.65f, BLOCK_HEIGHT * 12.0f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempLuck > character.luck) {
            character.tempStatPoints++;
            character.tempLuck--;
        } else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 7.65f, BLOCK_HEIGHT * 13.25f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempM_defence > character.m_defence) {
            character.tempStatPoints++;
            character.tempM_defence--;
        } else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 7.65f, BLOCK_HEIGHT * 14.5f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempStamina > character.stamina) {
            character.tempStatPoints++;
            character.tempStamina--;
        } else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 7.65f, BLOCK_HEIGHT * 15.75f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempStrength > character.strength) {
            character.tempStatPoints++;
            character.tempStrength--;
        } else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 7.65f, BLOCK_HEIGHT * 17.0f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempVisability > character.visability) {
            character.tempStatPoints++;
            character.tempVisability--;
        }

        if ((CheckCollisionRecs(Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}, Rectangle{BLOCK_WIDTH * 9.25f + (std::to_string(character.visability).length() - 1) * 25, BLOCK_HEIGHT * 7.0f, check_button.width * 1.0f, check_button.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
            DrawTexture(check_button_pressed, BLOCK_WIDTH * 9.25f + (std::to_string(character.visability).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 7 + check_button.height / 10, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
            clickAnimation = 0.25f;
            clickedButton = 3;
        } else if (clickAnimation < -0.15f && clickedButton == 3) {
            character.agility = character.tempAgility;
            character.defence = character.tempDefence;
            character.luck = character.tempLuck;
            character.m_defence = character.tempM_defence;
            character.maxHealth = character.tempMaxHealth;
            character.stamina = character.tempStamina;
            character.statPoints = character.tempStatPoints;
            character.strength = character.tempStrength;
            character.visability = character.tempVisability;

            clickedButton = 0;
        } else if (clickAnimation > 0 && clickedButton == 3) {
            clickAnimation -= GetFrameTime();
            DrawTexture(check_button_pressed, BLOCK_WIDTH * 9.25f + (std::to_string(character.visability).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 7 + check_button.height / 10, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
        } else if (clickAnimation < 0) {
            clickAnimation -= GetFrameTime();
            DrawTexture(check_button, BLOCK_WIDTH * 9.25f + (std::to_string(character.visability).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 7, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
        } else
            DrawTexture(check_button, BLOCK_WIDTH * 9.25f + (std::to_string(character.visability).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 7, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
    } else {

        DrawTextEx(FatPixel, std::to_string(character.agility).c_str(), Vector2{BLOCK_WIDTH * 8.0f, BLOCK_HEIGHT * 8.0f}, (BLOCK_HEIGHT / 2), 1, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, std::to_string(character.defence).c_str(), Vector2{BLOCK_WIDTH * 8.0f, BLOCK_HEIGHT * 9.25f}, (BLOCK_HEIGHT / 2), 1, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, std::to_string(character.maxHealth).c_str(), Vector2{BLOCK_WIDTH * 8.0f, BLOCK_HEIGHT * 10.5f}, (BLOCK_HEIGHT / 2), 1, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, std::to_string(character.luck).c_str(), Vector2{BLOCK_WIDTH * 8.0f, BLOCK_HEIGHT * 11.75f}, (BLOCK_HEIGHT / 2), 1, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, std::to_string(character.m_defence).c_str(), Vector2{BLOCK_WIDTH * 8.0f, BLOCK_HEIGHT * 13.0f}, (BLOCK_HEIGHT / 2), 1, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, std::to_string(character.stamina).c_str(), Vector2{BLOCK_WIDTH * 8.0f, BLOCK_HEIGHT * 14.25f}, (BLOCK_HEIGHT / 2), 1, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, std::to_string(character.strength).c_str(), Vector2{BLOCK_WIDTH * 8.0f, BLOCK_HEIGHT * 15.5f}, (BLOCK_HEIGHT / 2), 1, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, std::to_string(character.visability).c_str(), Vector2{BLOCK_WIDTH * 8.0f, BLOCK_HEIGHT * 16.75f}, (BLOCK_HEIGHT / 2), 1, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 8.0f}, (BLOCK_HEIGHT / 2), 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 9.25f}, (BLOCK_HEIGHT / 2), 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 10.5f}, (BLOCK_HEIGHT / 2), 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 11.75f}, (BLOCK_HEIGHT / 2), 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 13.0f}, (BLOCK_HEIGHT / 2), 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 14.25f}, (BLOCK_HEIGHT / 2), 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 15.5f}, (BLOCK_HEIGHT / 2), 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f, BLOCK_HEIGHT * 16.75f}, (BLOCK_HEIGHT / 2), 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});

        DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.agility).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 8.0f}, 25, 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.defence).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 9.25f}, 25, 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.maxHealth).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 10.5f}, 25, 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.luck).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 11.75f}, 25, 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.m_defence).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 13.0f}, 25, 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.stamina).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 14.25f}, 25, 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.strength).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 15.5f}, 25, 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});
        DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.visability).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 16.75f}, 25, 1, Color{130, 130, 130, static_cast<unsigned char>(alpha)});

        DrawTexture(check_button_locked, BLOCK_WIDTH * 9.25f + (std::to_string(character.visability).length() - 1) * (BLOCK_HEIGHT / 2), BLOCK_HEIGHT * 7, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
    }

    DrawTextEx(FatPixel, std::to_string(character.tempStatPoints).c_str(), Vector2{BLOCK_WIDTH * 5.5f, BLOCK_HEIGHT * 7.0f}, (BLOCK_HEIGHT / 2), 1, Color{0, 82, 172, static_cast<unsigned char>(alpha)});
    DrawTexture(statIcons["STAT_POINT_ICON"], BLOCK_WIDTH * 6, BLOCK_HEIGHT * 7, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
}

/// @brief load the room
/// @param id room id
void loadRoom(const int id) {
    collidersX.clear();
    collidersY.clear();
    doors.clear();

    std::string path = "./resources/maps/level";
    path.append(std::to_string(id));
    path.append(".json");
    level.open(path);
    if (!level.is_open()) {
        std::cout << "Level failed to load\n";
        shouldBePlaying = false;
        return;
    }

    while (!level.eof()) {
        level >> temp;
        levelData += temp;
    }

    if (levelData.empty()) { return; }

    json data = json::parse(levelData);

    auto map = data["map"];

    auto leveChests = data["chests"];
    auto levelDoors = data["doors"];
    auto enemiesLevel = data["emenies"];
    auto NHCsLevel = data["nhcs"];

    int enemyCounter = 0;
    int doorCounter = 0;
    int chestCounter = 0;
    int NHCsCounter = 0;

    const int width = 20;
    const int height = 20;

    int grid[width][height];

    for(auto & i : grid){
        for(int & j : i){
            j = 0;
        }
    }
    auto style = data["style"].dump();

    for (int y = 0; y < map.size(); y++) {
        auto row = map[y].dump();
        for (int x = 0; x < row.size(); x++) {
            auto tile = row[x];
            if (style == "\"human_cave\"") {
                if (tile == '#' || tile == ' ' || tile == '"') {
                    grid[x - 1][y] = 1;
                    continue;
                }
                else if (tile == '+') {
                    if (!playerSpawned) {
                        character.x = x * BLOCK_WIDTH;
                        character.y = y * BLOCK_HEIGHT;
                        playerSpawned = true;
                    }
                    DrawImage(x, y, textures["HC_FLOOR_0"]);
                    grid[x - 1][y] = 1;

                } else if (tile == 'a') {
                    DrawImage(x, y, textures["HC_FLOOR_0"]);
                    if (NHCs.size() == NHCsCounter) {
                        NHCs.emplace_back(x * BLOCK_WIDTH, y * BLOCK_WIDTH, NHCsLevel[enemyCounter]["id"]);
                        NHCsCounter++;
                    }
                    grid[x - 1][y] = 1;
                } else if (tile == '-') {
                    DrawTextureWithCollider(x, y, textures["HC_WALL"]);
                    grid[x - 1][y] = 0;
                }
                else if (tile == '.') {
                    DrawImage(x, y, textures["HC_FLOOR_0"]);
                    grid[x - 1][y] = 1;
                }
                else if (tile == ',') {
                    DrawImage(x, y, textures["HC_FLOOR_1"]);
                    grid[x - 1][y] = 1;
                }
                else if (tile == '|') {
                    DrawTextureWithCollider(x, y, textures["HC_WALL_LEFT"], 3);
                    grid[x - 1][y] = 1;
                }
                else if (tile == '/') {
                    DrawTextureWithCollider(x, y, textures["HC_WALL_RIGHT"], -3);
                    grid[x - 1][y] = 1;
                }
                else if (tile == '(') {
                    DrawTextureWithCollider(x, y, textures["HC_WALL_CORNER_LEFT"]);
                    grid[x - 1][y] = 0;
                }
                else if (tile == ')') {
                    DrawTextureWithCollider(x, y, textures["HC_WALL_CORNER_RIGHT"]);
                    grid[x - 1][y] = 0;
                }
                else if (tile == '{') {
                    DrawTextureWithCollider(x, y, textures["HC_WALL_FRONT_CORNER_LEFT"]);
                    grid[x - 1][y] = 0;
                }
                else if (tile == '}') {
                    DrawTextureWithCollider(x, y, textures["HC_WALL_FRONT_CORNER_RIGHT"]);
                    grid[x - 1][y] = 0;
                }
                else if (tile == 'c' || tile == 'C') {
                    if (chests.size() == chestCounter) {
                        chests.push_back({{1, true}, Chest{static_cast<int>(BLOCK_WIDTH * x), static_cast<int>(BLOCK_HEIGHT * (y + 1)), 0, Rectangle{BLOCK_WIDTH * (x - 2.25f), BLOCK_HEIGHT * (y - 1.68f), BLOCK_WIDTH * 3.5f, BLOCK_HEIGHT * 3.5f}, Item{leveChests[chestCounter]["itemid"], leveChests[chestCounter].value("dura", 100), leveChests[chestCounter].value("qantity", 1)}, currentRoom}});
                        chestCounter++;
                    }
                    DrawImage(x, y, textures["HC_FLOOR_0"]);
                    grid[x - 1][y] = 0;
                } else if (tile == 'd') {
                    doors.emplace_back(Rectangle{BLOCK_WIDTH * (x - 2.0f), BLOCK_HEIGHT * (y - 1.0f), BLOCK_WIDTH * 3.0f, BLOCK_HEIGHT * 3.0f}, levelDoors[doorCounter]["roomid"]);
                    DrawTextureWithCollider(x, y, textures["HC_DOOR"]);
                    doorCounter++;
                    grid[x - 1][y] = 0;
                } else if (tile == 'D') {
                    doors.emplace_back(Rectangle{BLOCK_WIDTH * x - textures["DOOR"].width * 1.0f, BLOCK_HEIGHT * (y + 0.43f) - textures["DOOR"].height * 1.0f, BLOCK_WIDTH * 3.0f, BLOCK_HEIGHT * 2.0f}, levelDoors[doorCounter]["roomid"]);
                    DrawTextureWithCollider(x, y, textures["HC_FLOOR_0"]);
                    doorCounter++;
                    grid[ - 1][y] = 0;
                } else if (tile == 'e' || tile == 'E') {
                    DrawImage(x, y, textures["HC_FLOOR_0"]);
                    if (enemies.size() == enemyCounter) {
                        enemies.emplace_back(x * BLOCK_WIDTH, y * BLOCK_WIDTH, enemiesLevel[enemyCounter]["enemyid"]);
                        enemyCounter++;
                    }
                    grid[x - 1][y] = 1;
                }
            }
        }
    }
    enemyCount = enemyCounter != 0 ? enemyCounter : enemyCount;
    for (int i = 0; i < enemyCount; i++) {
        if (enemiesLevel[i]["healthBarName"].dump() != "\"\"" && enemies[i].health > 0) {
            enemies[i].isBoss = true;
            auto name = enemiesLevel[i]["healthBarName"].dump();
            name.pop_back();
            name = name.erase(0, 1);
            DrawTextPro(FatPixel, name.c_str(), Vector2{SCREEN_WIDTH / 3.75f - cam.offset.x, SCREEN_HEIGHT - BLOCK_HEIGHT * 1.75f - (BLOCK_HEIGHT * 1.25f * i)}, Vector2{12.5f, 12.5f}, 0, 25, 1, GOLD);
            DrawRectangle(SCREEN_WIDTH / 4 - cam.offset.x, SCREEN_HEIGHT - BLOCK_HEIGHT * 1.5f - (BLOCK_HEIGHT * 1.25f * i), SCREEN_WIDTH / 2, BLOCK_HEIGHT / 2, Color{80, 80, 80, 255});
            DrawRectangle(SCREEN_WIDTH / 4 - cam.offset.x, SCREEN_HEIGHT - BLOCK_HEIGHT * 1.5f - (BLOCK_HEIGHT * 1.25f * i), (SCREEN_WIDTH / 2.0f) * (enemies[i].health *  1.0f / (enemies[i].maxHealth * 1.0f)), BLOCK_HEIGHT / 2, RED);
        }
    }
    PathFinder<width, height> pathfinder;

    for (auto &enemy: enemies) {

        // set the enemies x and y value
        if (!enemy.isEnabled)
            continue;
        if(enemy.xFinalDest != character.x || enemy.yFinalDest != character.y || enemy.path.empty()){
            enemy.path = pathfinder.aStarSearch(grid, make_pair(round(enemy.x) / BLOCK_WIDTH, round(enemy.y) / BLOCK_HEIGHT), make_pair(character.x / BLOCK_WIDTH, character.y / BLOCK_HEIGHT));
            enemy.xFinalDest = character.x;
            enemy.yFinalDest = character.y;
        }

#ifdef DEBUG
        auto tmp = enemy.path;
        while(!tmp.empty())
		{
            DrawRectangle(tmp.top().first * BLOCK_WIDTH, tmp.top().second * BLOCK_HEIGHT, BLOCK_WIDTH, BLOCK_HEIGHT, Color{50, 168, 68, 180});
            tmp.pop();
        }
#endif
        enemy.SetCollider();
        enemy.SetXY();
        enemy.SetAware();
        enemy.DealDamage();
        enemy.DrawHealth();
        enemy.DrawDamageText();
#ifdef DEBUG
        DrawCircle(enemy.x, enemy.y, enemy.awarenessRadius, Color{255, 0, 0, 70});
        DrawRectangleRec(enemy.collider, Color{0, 0, 255, 70});
#endif
        // display the enemies texture
        enemy.Draw();
    }

    for (auto &NHC: NHCs) {
        if (!NHC.isEnabled)
            continue;
        NHC.SetCollider();
        NHC.SetXY();
        NHC.SetAware();
        NHC.DrawHealth();
        NHC.DrawDamageText();

#ifdef DEBUG
        DrawCircle(NHC.x, NHC.y, NHC.awarenessRadius, Color{255, 0, 0, 70});
        DrawRectangleRec(NHC.collider, Color{0, 0, 255, 70});
#endif
        NHC.Draw();
    }
    // load chests
    for (auto &chestX: chests) {
        Chest &chest = chestX.second;
        int &state = chestX.first.first;
        bool &hasItem = chestX.first.second;
        bool isE = false;
        if (chest.room != currentRoom)
            continue;

        // check if the player
		// is within range to open the chest
        if (character.CheckCollision(chest.openRange)) {
            // check if the player opened the chest or pressed e
            if (IsKeyPressed(KEY_E) || state > 1) {
				// check if state is within range
				if (state < 300) {
					// incrament state
					state += 1000 * GetFrameTime();
				}
				else {
					// set state to max value in range
					state = 300;
				}
			}

            if (state != 300)
                isE = true;

            // check if the player picked up the chest
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionCircleRec(Vector2{cursor.x - cam.offset.x * 1.0f, cursor.y * 1.0f}, 2, Rectangle{chest.x * 1.0f - BLOCK_WIDTH, chest.y * 1.0f - BLOCK_HEIGHT, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 1.0f}) && state == 300 && hasItem) {

                // check if the player can pick up the item in his selected slot
                if (character.hotbarItems[character.selectedItemSlot].id == -1) {
                    // add character to players inventory
                    character.hotbarItems[character.selectedItemSlot] = chest.item;
                    character.hotbarItems[character.selectedItemSlot].SetTexture();
                    // remove item from chest
                    hasItem = false;
                } else {
                    for (auto & hotbarItem : character.hotbarItems) {
                        // check if their is an available slot
                        if (hotbarItem.id == -1) {
                            // add character to players inventory
                            hotbarItem = chest.item;
                            hotbarItem.SetTexture();
                            // remove item from chest
                            hasItem = false;
                            break;
                        }
                    }
                }
            }
        } else {
            // check if the chset is open
            if (state > 1)
                // slowly close the chest if player is not within range
                state -= 1000 * GetFrameTime();
        }
        chest.SetTexture(state);
        chest.Draw();

        if (isE)
            DrawTexturePro(textures["E_KEY"], Rectangle{0, 0, textures["E_KEY"].width * 1.0f, textures["E_KEY"].height * 1.0f}, Rectangle{chest.x - BLOCK_WIDTH / 2.25f, chest.y - BLOCK_HEIGHT * 1.1f, BLOCK_WIDTH / 2.5f, BLOCK_HEIGHT / 2.5f}, Vector2{textures["E_KEY"].width / 2.0f, textures["E_KEY"].height / 2.0f}, 0, RAYWHITE);

        // check if the chest is open and if the player didn't pick up the item
        if (chest.isOpen && hasItem) {
            // load the correct texture for the item
            if (!chest.item.isTextureSet) {
                chest.item.SetTexture();
            }
            // Draw the item
            chest.DrawItem();
        }

        // correct the chests values
#ifdef DEBUG
        DrawRectangleRec(chest.openRange, Color{255, 0, 0, 90});
        DrawRectangleRec(Rectangle{chest.x * 1.0f - BLOCK_WIDTH, chest.y * 1.0f - BLOCK_WIDTH, BLOCK_HEIGHT * 1.0f, BLOCK_HEIGHT * 1.0f}, Color{135, 245, 66, 90});
        DrawText(std::to_string(state).c_str(), chest.x, chest.y - 30, 20, RED);
#endif
        collidersX.push_back(Rectangle{chest.x * 1.0f - BLOCK_WIDTH, chest.y * 1.0f - BLOCK_HEIGHT, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 1.0f});
        collidersY.push_back(Rectangle{chest.x * 1.0f - BLOCK_WIDTH, chest.y * 1.0f - BLOCK_HEIGHT, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 1.0f});
    }
}

float timeSinceShake{0.0f};

void ScreenShake(int intensity) {
    timeSinceShake += GetFrameTime();

    if (timeSinceShake > 0.1f) {
        isAttacked = false;
        timeSinceShake = 0.0f;
        return;
    }

    cam.offset.x += GetRandomValue(-intensity, intensity);
    cam.offset.y += GetRandomValue(-intensity, intensity);
}

#pragma endregion