//
// Created by mixal on 12/5/2022.
//
#include "fstream"
#include "raylib.h"
#include <stack>
#include <vector>

#ifndef ENDURE_VARIABLE_DECLARATIONS_H
#define ENDURE_VARIABLE_DECLARATIONS_H

Texture2D grass;
Camera2D cam = {0};
std::fstream level;
Texture2D check_button;
Texture2D check_button_pressed;
Texture2D check_button_locked;

Texture2D back_button;
Texture2D back_button_pressed;

Texture2D exit_button;
Texture2D exit_button_pressed;

bool isAttacked{false};

bool attacked{false};

bool playerSpawned{false};

Font FatPixel;

int currentRoom = 000;

// colliders that dont allow the player to move side ways
std::vector<Rectangle> collidersX;
// colliders that don't allow the player to move up or down
std::vector<Rectangle> collidersY;
// doors to move between rooms
std::vector<std::pair<Rectangle, int>> doors;

bool isPaused = false;

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 1000;

RenderTexture screen;
Shader shader;
bool isShaderActive{false};

int enemyCount = 0;
int BLOCK_WIDTH = SCREEN_WIDTH / 20;
int BLOCK_HEIGHT = SCREEN_HEIGHT / 20;

bool shouldBePlaying = true;

std::string levelData;
std::string temp;

void DrawImage(float x, float y, Texture2D texture);
void DrawTextureWithCollider(float x, float y, Texture2D texture);
void DrawTextureWithCollider(float x, float y, Texture2D texture, float widthScale);
void DrawPauseMenu();
void loadRoom(const int id);
void ScreenShake(int intensity);


#endif //ENDURE_DECLARATIONS_H
