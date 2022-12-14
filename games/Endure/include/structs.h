//
// Created by mixal on 12/5/2022.
//
#include "raylib.h"
#include "declarations.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>

#ifndef ENDURE_STRUCTS_H
#define ENDURE_STRUCTS_H

/// @brief generic sprite
struct Sprite
{
    /// @brief x pissotion of the sprite
    float x{-1};
    /// @brief y possition of the sprite
    float y{-1};
    double rotation{};
    Texture2D texture{};
    bool isEnabled{true};

    /// @brief Draw the texture of the sprite
    void Draw() const
    {
        // draws the texture of the sprite at possition (x, y)
        if (isEnabled)
            DrawTexturePro(texture, Rectangle{0, 0, texture.width * 1.0f, texture.height * 1.0f}, Rectangle{x - BLOCK_WIDTH / 2, y - BLOCK_HEIGHT / 2, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 1.0f}, Vector2{BLOCK_WIDTH * 0.5f, BLOCK_HEIGHT * 0.5f}, rotation * (180.0 / PI), RAYWHITE);
    }
    /// @brief Draw the texture of the sprite with input tint/color
    /// @param color
    void Draw(Color color)
    {
        // draws the texture of the sprite at possition (x, y) with the iput color
        if (isEnabled)
            DrawTexturePro(texture, Rectangle{0, 0, texture.width * 1.0f, texture.height * 1.0f}, Rectangle{x, y, texture.width * 1.0f * (BLOCK_WIDTH / (texture.width != 0 ? texture.width : 1)), texture.height * 1.0f * (BLOCK_HEIGHT / (texture.height != 0 ? texture.height : 1))}, Vector2{BLOCK_WIDTH * 0.5f, texture.height * 0.5f}, rotation * (180.0 / PI), color);
    }

} cursor;

/// @brief base item
struct Item
{
    /// @brief the current id of the item
    int id{100};
    /// @brief the curent durability of the item
    int durability{-1};
    /// @brief the curent durability of the item
    int maxDurability{-1};
    /// @brief the number of items
    int quantity{1};
    // the items texture
    Texture2D texture{};

    bool isTextureSet{false};

    float attackSpeed{1};

    float damage{-1};

    void SetTexture()
    {
        if (isTextureSet)
            return;
        // load the sprite of correct item
        if (id == GREAT_SWORD)
        {
            texture = textures["GREAT_SWORD"];
            damage = 20.0f;
            maxDurability = 100;
            attackSpeed = 2;
        }
        else if (id == WOODEN_SWORD)
        {
            texture = textures["WOODEN_SWORD"];
            damage = 1.55f;
            maxDurability = 70;
            attackSpeed = 1;
        }
        else if (id == SILVER_SWORD)
        {
            texture = textures["SILVER_SWORD"];
            damage = 5.0f;
            maxDurability = 250;
            attackSpeed = 0.5f;
        }
        else if (id == GOLDEN_SWORD)
        {
            texture = textures["GOLDEN_SWORD"];
            damage = 12.0f;
            maxDurability = 125;
            attackSpeed = 1;
        }
        else if (id == BLOODY_SWORD)
        {
            texture = textures["BLOODY_SWORD"];
            damage = 15.0f;
            maxDurability = 200;
            attackSpeed = 1.25f;
        }
        else if (id == IRON_SWORD)
        {
            texture = textures["IRON_SWORD"];
            damage = 7.0f;
            maxDurability = 300;
            attackSpeed = 0.75;
        }
        isTextureSet = true;
    }

    // initiate default values
    Item(int inId, int inDurability, int inQuantity)
    {
        id = inId;
        durability = inDurability;
        if (durability > maxDurability)
        {
            durability = maxDurability;
        }
        quantity = inQuantity;
    }

    Item()
    {
        id = -1;
    }
} air;

struct Particle : Sprite
{
    Texture2D textures[3]{};
    float textureSwitchDelay{0.0f};
    float timeSinceSwap{0.0f};
    int index{0};
    float rotationDelay{0};
    float timeSinceLastRotaion{0};
    int alpha;
    int alphaDecrease;

    Particle(Texture2D inTextures[], float textureSwitchDelay, int x, int y, float rotationDelay, int alpha, int alphaDecrease)
    {
        this->textureSwitchDelay = textureSwitchDelay;

        for (int i = 0; i < 3; i++)
        {
            textures[i] = inTextures[i];
        }
        texture = textures[0];
        this->x = x;
        this->y = y;
        this->rotationDelay = rotationDelay;
        this->alpha = alpha;
        this->alphaDecrease = alphaDecrease;
    }

    Particle(){

    }

    void Draw()
    {
        y--;
        alpha -= alphaDecrease;
        timeSinceSwap += GetFrameTime();
        timeSinceLastRotaion += GetFrameTime();

        if (timeSinceSwap > textureSwitchDelay)
        {
            if (index > 2)
            {
                isEnabled = false;
                return;
            }
            timeSinceSwap = 0.0f;
            texture = textures[index];
            index++;
        }

        if (timeSinceLastRotaion > rotationDelay)
        {
            rotation = GetRandomValue(0, 360);
            timeSinceLastRotaion = 0.0f;
        }

        DrawTexturePro(texture, Rectangle{0, 0, texture.width * 1.0f, texture.height * 1.0f}, Rectangle{x, y, BLOCK_WIDTH * 0.4f, BLOCK_HEIGHT * 0.4f}, Vector2{BLOCK_WIDTH * 0.2f, BLOCK_HEIGHT * 0.2f}, rotation, Color{245, 245, 245, static_cast<unsigned char>(alpha)});
    }
};

/// @brief main character
struct Character : Sprite
{
    /// @brief the background hotbar slot
    Texture2D hotbarSlot{};
    /// @brief the background hotbar slot for selected item
    Texture2D hotbarSlotSelected{};
    /// @brief The slot (0-5) that the player is holding
    int selectedItemSlot{0};
    /// @brief Items in the hotbar that the player is not holding
    Item hotbarItems[6];
    /// @brief every item the player is carring
    Texture2D inventory[10]{};
    /// @brief animation in which the player cycle throught when they are idle/still
    Texture2D idleStates[4]{};
    /// @brief animation in which the player cycle throught when they are walkiing
    Texture2D walkStates[6]{};
    /// @brief full heart icon
    Texture2D fullHealth{};
    Sound attackingSoundeffect{};
    Sound missSoundeffect{};

    Texture2D dashIcon{};
    /// @brief half heart icon
    Texture2D halfHealth{};
    /// @brief the hitbox of the main item
    Rectangle itemCollider{};

    Sound dash_sound_effect{};

    float abilities{3};
    int maxAbilities{3};

    int defence{0};
    int strength{0};
    int m_defence{0};
    int agility{0};
    int luck{0};
    int stamina{0};
    int visability{0};

    int tempDefence{0};
    int tempStrength{0};
    int tempM_defence{0};
    int tempAgility{0};
    int tempLuck{0};
    int tempStamina{0};
    int tempVisability{0};

    int statPoints{3};
    int tempStatPoints{};

    /// @brief keeps track if the player is attacking or not
    bool attacking{false};
    float timeSinceLastAttack{0};
    /// @brief the size of the hitbox of the player
    float radius{};
    /// @brief keeps track of the direction the player is facing
    short facing{0};
    /// @brief the speed of the player (x)
    int speedX{};
    /// @brief the speed of the player (y)
    int speedY{};
    /// @brief the current amout of health the player has
    int health{};
    /// @brief the max amout of health a player can have
    int maxHealth{20};
    int tempMaxHealth{};
    /// @brief the current attacking state of the player
    float state{1};
    /// @brief the current animation of the player modle
    float stateTexture{0.0f};
    /// @brief the goal of the plays attacking state
    float goal{1};
    /// @brief keeps track if a player is moving or not
    bool isIdle{true};

    std::vector<Particle> footsteps;
    Texture2D footstepTextures[3]{};
    float timeSinceLastFootstep{0};

    /// @brief decreases the players health(with defence in mind)
    /// @param damage
    void Damage(int damage)
    {
        // calculate the amout of health removed
        health -= damage - (damage * (defence / 100));
        isAttacked = true;
        // check if player is dead
        if (health < 1)
        {
            // stop game
            shouldBePlaying = false;
        }
    }

    /// @brief initialize default fields
    void Init()
    {
        // load the textures for when the player is idle/still
        idleStates[0] = characterAnimationIdle["IDLE_0"];
        idleStates[1] = characterAnimationIdle["IDLE_1"];
        idleStates[2] = characterAnimationIdle["IDLE_2"];
        idleStates[3] = characterAnimationIdle["IDLE_3"];

        // load the walking animations
        walkStates[0] = characterAnimationWalk["WALK_0"];
        walkStates[1] = characterAnimationWalk["WALK_1"];
        walkStates[2] = characterAnimationWalk["WALK_2"];
        walkStates[3] = characterAnimationWalk["WALK_3"];
        walkStates[4] = characterAnimationWalk["WALK_4"];
        walkStates[5] = characterAnimationWalk["WALK_5"];

        // load hotbar slots
        hotbarSlot = playerUI["HOTBAR_SLOT"];
        hotbarSlotSelected = playerUI["HOTBAR_SELECTED"];

        dash_sound_effect = fx["PLAYER_DASH"];
        SetSoundVolume(dash_sound_effect, 1.0f);

        // load the health icons
        fullHealth = playerUI["HEALTH_FULL"];
        halfHealth = playerUI["HEALTH_HALF"];

        dashIcon = playerUI["DASH_ICON"];

        // set default values
        health = maxHealth;

        abilities = maxAbilities;

        tempStatPoints = statPoints;
        tempAgility = agility;
        tempDefence = defence;
        tempLuck = luck;
        tempM_defence = m_defence;
        tempMaxHealth = maxHealth;
        tempStamina = stamina;
        tempStrength = strength;
        tempVisability = visability;

        attackingSoundeffect = fx["PLAYER_ATTACK"];
        SetSoundVolume(attackingSoundeffect, 2.0f);
        missSoundeffect = fx["PLAYER_MISS"];
        SetSoundVolume(missSoundeffect, 1.0f);

        footstepTextures[0] = particles["FOOTSTEP_000"];
        footstepTextures[1] = particles["FOOTSTEP_001"];
        footstepTextures[2] = particles["FOOTSTEP_002"];
    }

    /// @brief draw the character modle
    void Draw()
    {

        timeSinceLastFootstep += GetFrameTime();
        // increse the state of the players texture
        stateTexture += 600 * GetFrameTime();

        // check which animation to load(idle or walking)
        if (isIdle)
        {
            // load the proper texture for idle animation
            if (stateTexture < 100)
            {
                texture = idleStates[0];
            }
            else if (stateTexture < 200)
            {
                texture = idleStates[1];
            }
            else if (stateTexture < 300)
            {
                texture = idleStates[2];
            }
            else if (stateTexture < 400)
            {
                texture = idleStates[3];
            }
            else
            {
                stateTexture = 0;
            }
        }
        else
        {
            if (timeSinceLastFootstep > 0.1f)
            {
                footsteps.emplace_back(footstepTextures, 0.06f, x, y + BLOCK_HEIGHT / 2, 0.1f, 90, 1);
                timeSinceLastFootstep = 0;
            }
            // load the proper textures for walking animation
            if (stateTexture < 75)
            {
                texture = walkStates[0];
            }
            else if (stateTexture < 150)
            {
                texture = walkStates[1];
            }
            else if (stateTexture < 225)
            {
                texture = walkStates[2];
            }
            else if (stateTexture < 300)
            {
                texture = walkStates[3];
            }
            else if (stateTexture < 375)
            {
                texture = walkStates[4];
            }
            else if (stateTexture < 450)
            {
                texture = walkStates[5];
            }
            else
            {
                stateTexture = 0;
            }
        }
        for (auto &i : footsteps)
        {
            if (!i.isEnabled)
            {
                footsteps.erase(footsteps.begin());
            }
            else
            {
                i.Draw();
            }
        }

        // check which way the player is facing
        if (facing == 1)
            // load the player texture backwords
            DrawTexturePro(texture, Rectangle{0, 0, -texture.width * 1.0f, texture.height * 1.0f}, Rectangle{x * 1.0f, y * 1.0f, BLOCK_WIDTH * 0.7f, BLOCK_HEIGHT * 1.0f}, Vector2{BLOCK_WIDTH * 0.35f, BLOCK_HEIGHT * 0.35f}, 0, RAYWHITE);
        else
            // load the normal player texture
            DrawTexturePro(texture, Rectangle{0, 0, texture.width * 1.0f, texture.height * 1.0f}, Rectangle{x * 1.0f, y * 1.0f, BLOCK_WIDTH * 0.7f, BLOCK_HEIGHT * 1.0f}, Vector2{BLOCK_WIDTH * 0.35f, BLOCK_HEIGHT * 0.35f}, 0, RAYWHITE);
    }

    /// @brief Load the players health
    void DrawPlayerUI()
    {
        // check if a half heart is needed
        if (health % 10 == 0)
        {
            // draw tha amount opf full heats that are needed
            for (int i = 0; i < health / 2; i++)
            {
                DrawTexture(fullHealth, (BLOCK_WIDTH / 2.5) * i + BLOCK_WIDTH / 4 - cam.offset.x, BLOCK_HEIGHT * 1.25, RAYWHITE);
            }
        }
        else
        {
            // draw all the fuul healths
            int i;
            for (i = 0; i < int(health / 2); i++)
            {
                DrawTexture(fullHealth, (BLOCK_WIDTH / 2.5) * i + BLOCK_WIDTH / 4 - cam.offset.x, BLOCK_HEIGHT * 1.25, RAYWHITE);
            }
            // draw the extra half
            DrawTexture(halfHealth, (BLOCK_WIDTH / 2.5) * i + BLOCK_WIDTH / 4 - cam.offset.x, BLOCK_HEIGHT * 1.25, RAYWHITE);
        }

        for (int i = 0; i < floor(abilities); i++)
        {
            DrawTexture(dashIcon, (BLOCK_WIDTH / 2.5) * i + BLOCK_WIDTH / 4 - cam.offset.x, BLOCK_HEIGHT * 2, RAYWHITE);
        }

        for (int i = 0; i < 6; i++)
        {
            if (i == selectedItemSlot)
                DrawImage((i + 1.0f) - cam.offset.x / BLOCK_WIDTH, 0.0f, hotbarSlotSelected);
            else
                DrawImage((i + 1.0f) - cam.offset.x / BLOCK_WIDTH, 0.0f, hotbarSlot);
            DrawTexturePro(hotbarItems[i].texture, Rectangle{0, 0, hotbarItems[i].texture.width * 1.0f, hotbarItems[i].texture.height * 1.0f}, Rectangle{(i + 1) * BLOCK_WIDTH - (BLOCK_WIDTH * 0.475f) - cam.offset.x, BLOCK_HEIGHT * 0.55f, BLOCK_WIDTH * 0.7f, BLOCK_HEIGHT * 0.7f}, Vector2{BLOCK_WIDTH * 0.35f, BLOCK_HEIGHT * 0.35f}, 0, RAYWHITE);
        }
    }

    /// @brief returns true if the player is colliding with a rectangle
    /// @param collider
    /// @return
    bool CheckCollision(Rectangle collider)
    {
        return CheckCollisionCircleRec(Vector2{x * 1.0f, y * 1.0f + (texture.height * BLOCK_WIDTH / (texture.height == 0 ? 1 : texture.height)) / 4.0f}, radius, collider);
    }

    /// @brief Draw the main hand item
    void DrawMainHand()
    {

        timeSinceLastAttack += GetFrameTime();

#ifdef DEBUG
        DrawText(std::to_string(timeSinceLastAttack).c_str(), x, y - 10, 20, BLUE);
#endif

        // checks if the player attacked
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && timeSinceLastAttack > (hotbarItems[selectedItemSlot].attackSpeed / 2) && !isPaused)
        {
            // flip the cuurent state
            state = -state;
            // assign the goal variable the coorect value
            if (state == 1)
            {
                goal = -1;
            }
            else if (state == -1)
            {
                goal = 1;
            }
            timeSinceLastAttack = 0;
            attacking = true;
            if(hotbarItems[selectedItemSlot].id != -1)
                PlaySound(missSoundeffect);
        }

        // checks if the player is curently attcking
        if (state != goal && goal != 0)
        {
            // checks if state should be incremented or decremented and add the correct amout
            if (state < goal)
            {
                state += 10 * GetFrameTime();
            }
            else if (goal < state)
            {
                state -= 10 * GetFrameTime();
            }
            // check if state is within bounds
            if (state > 1)
            {
                state = 1;
            }
            else if (state < -1)
            {
                state = -1;
            }
        }
        else
        {
            attacking = false;
            goal = 0;
        }

        // check if the player is attacking
        if (goal == 0)
            // draw the normal texture
            DrawTexturePro(hotbarItems[selectedItemSlot].texture, Rectangle{0, 0, hotbarItems[selectedItemSlot].texture.width * 1.0f, hotbarItems[selectedItemSlot].texture.height * 1.0f}, Rectangle{x * 1.0f, y * 1.0f, hotbarItems[selectedItemSlot].texture.width * (BLOCK_WIDTH / (hotbarItems[selectedItemSlot].texture.width != 0 ? hotbarItems[selectedItemSlot].texture.width : 1)) * 0.80f, hotbarItems[selectedItemSlot].texture.height * (BLOCK_HEIGHT / (hotbarItems[selectedItemSlot].texture.height != 0 ? hotbarItems[selectedItemSlot].texture.height : 1)) * 0.8f}, Vector2{hotbarItems[selectedItemSlot].texture.width * (BLOCK_WIDTH / (hotbarItems[selectedItemSlot].texture.width != 0 ? hotbarItems[selectedItemSlot].texture.width : 1)) * 1.2f, hotbarItems[selectedItemSlot].texture.height * (BLOCK_HEIGHT / (hotbarItems[selectedItemSlot].texture.height != 0 ? hotbarItems[selectedItemSlot].texture.height : 1)) * 1.2f}, (rotation + 3.9f) * (180.0 / PI), RAYWHITE);
        else
            // draw the item with offset-
            DrawTexturePro(hotbarItems[selectedItemSlot].texture, Rectangle{0, 0, hotbarItems[selectedItemSlot].texture.width * 1.0f, hotbarItems[selectedItemSlot].texture.height * 1.0f}, Rectangle{x * 1.0f, y * 1.0f, hotbarItems[selectedItemSlot].texture.width * (BLOCK_WIDTH / (hotbarItems[selectedItemSlot].texture.width != 0 ? hotbarItems[selectedItemSlot].texture.width : 1.0f)), hotbarItems[selectedItemSlot].texture.height * 1.0f * (BLOCK_HEIGHT / (hotbarItems[selectedItemSlot].texture.height != 0 ? hotbarItems[selectedItemSlot].texture.height : 1.0f))}, Vector2{hotbarItems[selectedItemSlot].texture.width * (BLOCK_WIDTH / (hotbarItems[selectedItemSlot].texture.width != 0 ? hotbarItems[selectedItemSlot].texture.width : 1)) * 1.2f, hotbarItems[selectedItemSlot].texture.height * (BLOCK_HEIGHT / (hotbarItems[selectedItemSlot].texture.height != 0 ? hotbarItems[selectedItemSlot].texture.height : 1)) * 1.2f}, (rotation + 3.9f + state) * (180.0 / PI), RAYWHITE);

        itemCollider = Rectangle{(x - 8 - sinf(rotation) * 50.0f) - hotbarItems[selectedItemSlot].texture.height, (y - 8 + cosf(rotation) * 50.0f) - hotbarItems[selectedItemSlot].texture.width, hotbarItems[selectedItemSlot].texture.width * 3.0f, hotbarItems[selectedItemSlot].texture.height * 3.0f};
#ifdef DEBUG
        DrawRectangleRec(itemCollider, Color{200, 0, 0, 90});
#endif
    }

    void Dash()
    {
        if (abilities >= 1)
        {
            if (facing == 0)
            {
                for (int i = 0; i < 200 + agility * 4; i++)
                {
                    int tempX = x;
                    x += 0.5f;
                    for (auto collider : collidersX)
                    {
                        if (CheckCollision(collider))
                        {
                            x = tempX;
                            i = 1000;
                        }
                    }
                }
            }
            else
            {
                for (int i = 0; i < 100 + agility; i++)
                {
                    int tempX = x;
                    x -= 0.5f;
                    for (auto collider : collidersX)
                    {
                        if (CheckCollision(collider))
                        {
                            x = tempX;
                            i = 1000;
                        }
                    }
                }
            }
            PlaySound(dash_sound_effect);
            abilities--;
        }
    }
    void EnemySwap();
    
} character;

/// @brief Default chest
struct Chest : Sprite
{
    /// @brief define that range in which a player must be to open it
    Rectangle openRange{};
    /// @brief define the item inside the chest
    Item item;
    /// @brief keep track if the chest is open
    bool isOpen{};

    int room{0};

    /// @brief load the opening animation
    /// @param state
    void SetTexture(int &state)
    {
        // check in which state of the animation the chest is in
        if (state < 100)
        {
            isOpen = false;
            texture = textures["CHEST_CLOSED"];
        }
        else if (state > 100 && state < 200)
        {
            texture = textures["CHEST_OPENING"];
        }
        else if (state > 200)
        {
            texture = textures["CHEST_OPEN"];
            isOpen = true;
        }
    }

    /// @brief Draw the item inside the chest
    void DrawItem()
    {
        DrawTexturePro(item.texture, Rectangle{0, 0, item.texture.width * 1.0f, item.texture.height * 1.0f}, Rectangle{x + (item.texture.width * (BLOCK_WIDTH / (item.texture.width != 0 ? item.texture.width : 1)) * 0.6f) / 2, y + (item.texture.height * (BLOCK_HEIGHT / (item.texture.height != 0 ? item.texture.height : 1)) * 0.6f) / 2, item.texture.width * (BLOCK_WIDTH / (item.texture.width != 0 ? item.texture.width : 1)) * 0.65f, item.texture.height * (BLOCK_HEIGHT / (item.texture.height != 0 ? item.texture.height : 1)) * 0.65f}, Vector2{item.texture.width * (BLOCK_WIDTH / (item.texture.width != 0 ? item.texture.width : 1)) * 1.2f, item.texture.height * (BLOCK_HEIGHT / (item.texture.height != 0 ? item.texture.height : 1)) * 1.2f}, 0, RAYWHITE);
    }

    /// @brief initialize the default values
    /// @param inX
    /// @param inY
    /// @param inRotation
    /// @param inOpenRange
    /// @param inItem
    Chest(int inX, int inY, double inRotation, Rectangle inOpenRange, Item inItem, int inRoom)
    {
        x = inX;
        y = inY;
        rotation = inRotation;
        openRange = inOpenRange;
        item = inItem;
        room = inRoom;
    }
};

/// @brief generic enemy
struct Enemy : Sprite
{
    /// @brief used to keep track if an enemy is awar of the player
    bool isAware{false};
    /// @brief the range in which an enemy becomes aware of the player
    float awarenessRadius{300.0f};
    /// @brief the range in which the player takes damage
    float range{30.0f};
    /// @brief the damage dealt to the player
    int damage{0};
    /// @brief the delay between attacks
    int attackSpeed{10};
    /// @brief the amout of time that passed, since the last attack
    float timeFromLastAttack{0};
    /// @brief the amout of health the enemy has
    int health{100};
    /// @brief the max amout of health the enemy can have
    int maxHealth{100};

    bool onFocus{false};

    float timeFromFocus{0.0f};

    float stateTexture{0.0f};

    float xDest{0};
    float yDest{0};

    float xFinalDest{0};
    float yFinalDest{0};

    int facing{0};

    Texture2D idleAnimations[4]{};
    Texture2D walkingAnimations[4]{};

    Rectangle collider{};

    std::string damageText{};
    int damageTextAlpha{0};
    float damageTextYPos{0.0f};

    bool attacked{false};
    float timeSinceAttacked{0.0f};
    bool attacking{false};
    float distanceTravelled{0.0f};
    float dashDistance{0.0f};

    bool isBoss{false};

    std::stack<std::pair<int,int>> path;

    Enemy(int x, int y, bool isAware, float awarenessRadius, Texture2D texture, int damage, float range, int attackSpeed, Rectangle collider){
        this->x = x;
        this->y = y;
        this->isAware = isAware;
        this->awarenessRadius = awarenessRadius;
        this->texture = texture;
        this->damage = damage;
        this->range = range;
        this->attackSpeed = attackSpeed;
        this->collider = collider;

        health = maxHealth;
    }

    Enemy(int x, int y, int id)
    {
        this->x = x;
        this->y = y;
        if (id == GOBLIN)
        {
            isAware = false;
            awarenessRadius = 300.0f;

            idleAnimations[0] = enemyTextures[GOBLIN].first[0];
            idleAnimations[1] = enemyTextures[GOBLIN].first[1];
            idleAnimations[2] = enemyTextures[GOBLIN].first[2];
            idleAnimations[3] = enemyTextures[GOBLIN].first[3];

            walkingAnimations[0] = enemyTextures[GOBLIN].second[0];
            walkingAnimations[1] = enemyTextures[GOBLIN].second[1];
            walkingAnimations[2] = enemyTextures[GOBLIN].second[2];
            walkingAnimations[3] = enemyTextures[GOBLIN].second[3];

            damage = 3;
            range = 30.0f;
            attackSpeed = 1;

            dashDistance = BLOCK_WIDTH * 1.0f;
        }
    }

    void Draw()
    {
        stateTexture += 600 * GetFrameTime();
        timeSinceAttacked += GetFrameTime();

        if(attacked){
            if (fmod(stateTexture, 355) < 50)
            {
                texture = walkingAnimations[3];
            }
            else if (fmod(stateTexture, 355) < 125)
            {
                texture = walkingAnimations[2];
            }
            else if (fmod(stateTexture, 355) < 200)
            {
                texture = walkingAnimations[1];
            }
            else if (fmod(stateTexture, 355) < 275)
            {
                texture = walkingAnimations[0];
            }else if(distanceTravelled > dashDistance){
                attacked = false;
                attacking = true;
            }
        }else if(attacking){
            if (fmod(stateTexture, 355) < 50)
            {
                texture = walkingAnimations[0];
            }
            else if (fmod(stateTexture, 355) < 125)
            {
                texture = walkingAnimations[1];
            }
            else if (fmod(stateTexture, 355) < 200)
            {
                texture = walkingAnimations[2];
            }
            else if (fmod(stateTexture, 355) < 275)
            {
                texture = walkingAnimations[3];
            }
        }
        // check which animation to load(idle or walking)
        else if (!isAware)
        {
            // load the proper texture for idle animation
            if (stateTexture < 100)
            {
                texture = idleAnimations[0];
            }
            else if (stateTexture < 200)
            {
                texture = idleAnimations[1];
            }
            else if (stateTexture < 300)
            {
                texture = idleAnimations[2];
            }
            else if (stateTexture < 400)
            {
                texture = idleAnimations[3];
            }
            else
            {
                stateTexture = 0;
            }
        }
        else
        {
            // load the proper textures for walking animation
            if (stateTexture < 75)
            {
                texture = walkingAnimations[0];
            }
            else if (stateTexture < 150)
            {
                texture = walkingAnimations[1];
            }
            else if (stateTexture < 225)
            {
                texture = walkingAnimations[2];
            }
            else if (stateTexture < 300)
            {
                texture = walkingAnimations[3];
            }
            else
            {
                stateTexture = 0;
            }
        }

        if (facing == 1)
            DrawTexturePro(texture, Rectangle{0, 0, -texture.width * 1.0f, texture.height * 1.0f}, Rectangle{x, y, BLOCK_WIDTH * 1.0f * (BLOCK_WIDTH / (texture.width == 0 ? 1 : texture.width)), BLOCK_HEIGHT * 1.0f * (BLOCK_HEIGHT / (texture.height == 0 ? 1 : texture.height))}, Vector2{texture.width * 0.5f, texture.height * 0.5f}, 0, RAYWHITE);
        else
            DrawTexturePro(texture, Rectangle{0, 0, texture.width * 1.0f, texture.height * 1.0f}, Rectangle{x, y, BLOCK_WIDTH * 1.0f * (BLOCK_WIDTH / (texture.width == 0 ? 1 : texture.width)), BLOCK_HEIGHT * 1.0f * (BLOCK_HEIGHT / (texture.height == 0 ? 1 : texture.height))}, Vector2{texture.width * 0.5f, texture.height * 0.5f}, 0, RAYWHITE);

    }

    void SetXY()
    {
        float tempX = x;
        float tempY = y;

		if(x + 1 == xDest || x - 1 == xDest)
		{
			x = xDest;
		}

		if(y + 1 == yDest || y - 1 == yDest)
		{
			y = yDest;
		}

        facing = character.x > x ?0 :1;

        if(attacked){
            distanceTravelled+=3;
            x+=facing == 1? 3:-3;
            return;
        }
        else if(attacking){
            distanceTravelled-=4;
            x-=facing == 1? 4:-4;
            if(distanceTravelled < 1){
                attacking = false;
                attacked = false;
                distanceTravelled = 0;
            }

            if (character.x > x)
            {
                if (character.y > y)
                {
                    x++;
                    y++;
                }
                else if (character.y < y)
                {
                    x++;
                    y--;
                }
                else
                {
                    x += 2;
                }
            }
            else if (character.x < x)
            {
                if (character.y > y)
                {
                    x--;
                    y++;
                }

                else if (character.y < y)
                {
                    x--;
                    y--;
                }
                else
                {
                    x -= 2;
                }
            }
            else if (character.y > y)
            {
                y += 2;
            }
            else if (character.y < y)
            {
                y -= 2;
            }

            for (auto collider : collidersX)
            {
                if (CheckCollisionRecs(collider, this->collider))
                {
                    x = tempX;
                    break;
                }
            }

            for (auto collider : collidersY)
            {
                if (CheckCollisionRecs(collider, this->collider))
                {
                    y = tempY;
                    break;
                }
            }
            return;
        }

        if(!path.empty()){
            xDest = path.top().first * BLOCK_WIDTH;
            yDest = path.top().second * BLOCK_HEIGHT;
            if(x == xDest && y == yDest){
                path.pop();
            }
        }

        if (isAware)
        {

            if (xDest > x)
            {
                if (yDest > y)
                {
                    x++;
                    y++;
                }
                else if (yDest < y)
                {
                    x++;
                    y--;
                }
                else
                {
                    x += 2;
                }
            }
            else if (xDest < x)
            {
                if (yDest > y)
                {
                    x--;
                    y++;
                }

                else if (yDest < y)
                {
                    x--;
                    y--;
                }
                else
                {
                    x -= 2;
                }
            }
            else if (yDest > y)
            {
                y += 2;
            }
            else if (yDest < y)
            {
                y -= 2;
            }
        }

        for (auto collider : collidersX)
        {
            if (CheckCollisionRecs(collider, this->collider))
            {
                x = tempX;
                break;
            }
        }

        for (auto collider : collidersY)
        {
            if (CheckCollisionRecs(collider, this->collider))
            {
                y = tempY;
                break;
            }
        }
    }

    void SetCollider()
    {
        collider = Rectangle{x - idleAnimations[0].width / 2 * 1.0f, y - idleAnimations[0].height / 2 * 1.0f, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 1.0f};
    }

    void DealDamage()
    {

        timeFromLastAttack += GetFrameTime() / attackSpeed;
#ifdef DEBUG
        DrawText(std::to_string(timeFromLastAttack).c_str(), 200, 100, 22, DARKGREEN);
#endif

        if(attacking && CheckCollisionCircles(Vector2{character.x * 1.0f, character.y * 1.0f}, character.radius, Vector2{x * 1.0f, y * 1.0f}, range) && timeFromLastAttack > attackSpeed){
            timeFromLastAttack = 0;
            attacking = false;
            character.Damage(damage);

        }
        else if (CheckCollisionCircles(Vector2{character.x * 1.0f, character.y * 1.0f}, character.radius, Vector2{x * 1.0f, y * 1.0f}, range) && timeFromLastAttack > attackSpeed)
        {
            timeSinceAttacked = 0;
            attacked = true;
        }
    }

    void Damage(int amount)
    {
        damageText = std::to_string(amount);
        damageTextAlpha = 200;

        damageTextYPos = character.y - BLOCK_HEIGHT / 1.5f;

       health -= amount;
        onFocus = true;
        timeFromFocus = 0.0f;
        if (health < 1)
        {
            isEnabled = false;
        }
    }

    void DrawDamageText(){
        for(int i = 0; i < 3; i++){
            if(damageTextAlpha > 0 && onFocus) {
                DrawTextPro(FatPixel, damageText.c_str(), Vector2{x - damageText.length() * 20 + BLOCK_WIDTH / 1.5f, damageTextYPos}, Vector2{10, 10}, 0, 20, 1,
                            Color{245, 152, 66, static_cast<unsigned char>(damageTextAlpha)});
                damageTextYPos += 0.5f;
                damageTextAlpha -= 2;
            }
        }
    }

    void SetAware()
    {
        if (!isAware)
        {
            if (CheckCollisionCircles(Vector2{character.x * 1.0f, character.y * 1.0f}, character.radius, Vector2{x * 1.0f, y * 1.0f}, awarenessRadius))
            {
                isAware = true;
            }
        }
    }

    void DrawHealth()
    {
        timeFromFocus += GetFrameTime();
        if (timeFromFocus > 14 || !isAware)
            onFocus = false;
        if (onFocus && !isBoss)
        {
            DrawRectangle(x - BLOCK_WIDTH * (BLOCK_WIDTH / (texture.width == 0 ? 1 : texture.width)) / 2.5f, y - texture.height / 3, BLOCK_WIDTH, BLOCK_HEIGHT / 8, Color{80, 80, 80, 255});
            DrawRectangle(x - BLOCK_WIDTH * (BLOCK_WIDTH / (texture.width == 0 ? 1 : texture.width)) / 2, y - texture.height / 3, (BLOCK_WIDTH * health) / maxHealth, BLOCK_HEIGHT / 8, Color{230, 41, 55, 255});
        }
    }
};

struct NHC: Sprite{
    bool isAware{false};
    float awarenessRadius{300.0f};
    float timeFromLastAttack{0};
    int health{100};
    int maxHealth{100};

    bool onFocus{false};

    float timeFromFocus{0.0f};

    float stateTexture{0.0f};

    int facing{0};

    Texture2D idleAnimations[4]{};
    Texture2D walkingAnimations[4]{};

    Rectangle collider{};

    std::string damageText{};
    int damageTextAlpha{0};
    float damageTextYPos{0.0f};

    NHC(int x, int y, int id)
    {
        this->x = x;
        this->y = y;
        if (id == PUMPKIN)
        {
            isAware = false;
            awarenessRadius = 300.0f;

            idleAnimations[0] = nhcs[PUMPKIN].first[0];
            idleAnimations[1] = nhcs[PUMPKIN].first[1];
            idleAnimations[2] = nhcs[PUMPKIN].first[2];
            idleAnimations[3] = nhcs[PUMPKIN].first[3];

            walkingAnimations[0] = nhcs[PUMPKIN].second[0];
            walkingAnimations[1] = nhcs[PUMPKIN].second[1];
            walkingAnimations[2] = nhcs[PUMPKIN].second[2];
            walkingAnimations[3] = nhcs[PUMPKIN].second[3];

        }
    }

    void Draw()
    {
        stateTexture += 600 * GetFrameTime();

        // check which animation to load(idle or walking)
        if (!isAware)
        {
            // load the proper texture for idle animation
            if (stateTexture < 100)
            {
                texture = idleAnimations[0];
            }
            else if (stateTexture < 200)
            {
                texture = idleAnimations[1];
            }
            else if (stateTexture < 300)
            {
                texture = idleAnimations[2];
            }
            else if (stateTexture < 400)
            {
                texture = idleAnimations[3];
            }
            else
            {
                stateTexture = 0;
            }
        }
        else
        {
            // load the proper textures for walking animation
            if (stateTexture < 75)
            {
                texture = walkingAnimations[0];
            }
            else if (stateTexture < 150)
            {
                texture = walkingAnimations[1];
            }
            else if (stateTexture < 225)
            {
                texture = walkingAnimations[2];
            }
            else if (stateTexture < 300)
            {
                texture = walkingAnimations[3];
            }
            else
            {
                stateTexture = 0;
            }
        }

        if (facing == 1)
            DrawTexturePro(texture, Rectangle{0, 0, -texture.width * 1.0f, texture.height * 1.0f}, Rectangle{x, y, BLOCK_WIDTH * 1.0f * (BLOCK_WIDTH / (texture.width == 0 ? 1 : texture.width)), BLOCK_HEIGHT * 1.0f * (BLOCK_HEIGHT / (texture.height == 0 ? 1 : texture.height))}, Vector2{texture.width * 0.5f, texture.height * 0.5f}, 0, RAYWHITE);
        else
            DrawTexturePro(texture, Rectangle{0, 0, texture.width * 1.0f, texture.height * 1.0f}, Rectangle{x, y, BLOCK_WIDTH * 1.0f * (BLOCK_WIDTH / (texture.width == 0 ? 1 : texture.width)), BLOCK_HEIGHT * 1.0f * (BLOCK_HEIGHT / (texture.height == 0 ? 1 : texture.height))}, Vector2{texture.width * 0.5f, texture.height * 0.5f}, 0, RAYWHITE);
    }

    void SetCollider()
    {
        collider = Rectangle{x - idleAnimations[0].width / 2 * 1.0f, y - idleAnimations[0].height / 2 * 1.0f, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 1.0f};
    }

    void SetXY() {
        float tempX = x;
        float tempY = y;
        if (isAware) {
            if (character.x > x) {
                if (character.y > y) {
                    x--;
                    y--;
                    facing = 1;
                } else if (character.y < y) {
                    x--;
                    y++;
                    facing = 1;
                } else {
                    x -= 2;
                    facing = 1;
                }
            } else if (character.x < x) {
                if (character.y > y) {
                    x++;
                    y--;
                    facing = 0;
                }
                else if (character.y < y) {
                    x++;
                    y++;
                    facing = 0;
                } else {
                    x += 2;
                    facing = 0;
                }
            } else if (character.y > y) {
                y -= 2;
            }

            else if (character.y < y) {
                y += 2;
            }else if(character.x == x){
                if (character.y > y) {
                    y+=2;
                    facing = 0;
                }
                else if (character.y < y) {
                    y-=2;
                    facing = 0;
                } else {
                    y += 2;
                    facing = 0;
                }
            }else if(character.y == y){
                if (character.x > x) {
                    x+=2;
                    facing = 0;
                }
                else if (character.x < x) {
                    x-=2;
                    facing = 0;
                } else {
                    x += 2;
                    facing = 0;
                }
            }
        }

        for (auto collider: collidersX) {
            if (CheckCollisionRecs(collider, this->collider)) {
                x = tempX;
            }
        }

        for (auto collider: collidersY) {
            if (CheckCollisionRecs(collider, this->collider)) {
                y = tempY;
            }
        }

    }

    void Damage(int amount)
    {
        damageText = std::to_string(amount);
        damageTextAlpha = 200;

        damageTextYPos = character.y - BLOCK_HEIGHT / 1.5f;

        health -= amount;
        onFocus = true;
        timeFromFocus = 0.0f;
        if (health < 1)
        {
            isEnabled = false;
        }
    }

    void DrawDamageText(){
        for(int i = 0; i < 3; i++){
            if(damageTextAlpha > 0 && onFocus) {
                DrawTextPro(FatPixel, damageText.c_str(), Vector2{x - damageText.length() * 20 + BLOCK_WIDTH / 1.5f, damageTextYPos}, Vector2{10, 10}, 0, 20, 1,
                            Color{245, 152, 66, static_cast<unsigned char>(damageTextAlpha)});
                damageTextYPos+= 0.5f;
                damageTextAlpha-=2;
            }
        }
    }

    void SetAware()
    {
        isAware = false;
        if (CheckCollisionCircles(Vector2{character.x * 1.0f, character.y * 1.0f}, character.radius, Vector2{x * 1.0f, y * 1.0f}, awarenessRadius))
        {
            isAware = true;
        }
    }

    void DrawHealth()
    {
        timeFromFocus += GetFrameTime();
        if (timeFromFocus > 14 || !isAware)
            onFocus = false;
        if (onFocus)
        {
            DrawRectangle(x - BLOCK_WIDTH * (BLOCK_WIDTH / (texture.width == 0 ? 1 : texture.width)) / 2.5f, y - texture.height / 3, BLOCK_WIDTH, BLOCK_HEIGHT / 8, Color{80, 80, 80, 255});
            DrawRectangle(x - BLOCK_WIDTH * (BLOCK_WIDTH / (texture.width == 0 ? 1 : texture.width)) / 2, y - texture.height / 3, (BLOCK_WIDTH * health) / maxHealth, BLOCK_HEIGHT / 8, Color{230, 41, 55, 255});
        }
    }
};

std::vector<Enemy> enemies;
std::vector<NHC> NHCs;

std::vector<std::pair<std::pair<int, bool>, Chest>> chests;


void Character::EnemySwap() {
    if(abilities >= 1){
        int closestIndex = -1;
        int closestDistance = 501;

        for(int i = 0; i < enemies.size(); i++){
            if(abs(cursor.x - enemies[i].x) + abs(cursor.y - enemies[i].y) < closestDistance){
                closestDistance = abs(cursor.x - enemies[i].x) + abs(cursor.y - enemies[i].y);
                closestIndex = i;
            }
        }

        if(closestIndex == -1 || closestDistance > 500 + agility * 20)
            return;

        float tempX = character.x;
        float tempY = character.y;
        float tempFacing = character.facing;

        character.x = enemies[closestIndex].x;
        character.y = enemies[closestIndex].y;
        character.facing = enemies[closestIndex].facing;

        enemies[closestIndex].x = tempX;
        enemies[closestIndex].y = tempY;
        enemies[closestIndex].facing = tempFacing;

        enemies[closestIndex].isAware = true;

        abilities -= 2;
    }
}

#endif //ENDURE_STRUCTS_H
