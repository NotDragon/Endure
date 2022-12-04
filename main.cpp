#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "resources.h"
#include "json.hpp"
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <fstream>

// toggle between debug mode and normal mode
#define DEBUG 1

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else
#define GLSL_VERSION 100
#endif

using json = nlohmann::json;

#pragma region Declarations
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
bool attacked{false};

Font FatPixel;

int currentRoom = TUTORIAL;

// colliders that dont allow the player to move side ways
std::vector<Rectangle> collidersX;
// colliders that don't allow the player to move up or down
std::vector<Rectangle> collidersY;
// doors to move between rooms
std::vector<std::pair<Rectangle, int>> doors;

bool isPaused = false;

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 1000;

// slipt the screen into 20 parts
int BLOCK_WIDTH = SCREEN_WIDTH / 20;
int BLOCK_HEIGHT = SCREEN_HEIGHT / 20;

bool shoudlBePlaying = true;

void loadRoom(const int id);
void DrawImage(float x, float y, Texture2D texture);
void DrawPauseMenu();
void DrawTextureWithCollider(float x, float y, Texture2D texture);
void DrawTextureWithCollider(float x, float y, Texture2D texture, float widthScale);

#pragma endregion Declarations

#pragma region Structs

/// @brief generic sprite
struct Sprite
{
	/// @brief x pissotion of the sprite
	float x{-1};
	/// @brief y possition of the sprite
	float y{-1};
	double rotation{};
	Texture2D texture;
	bool isEnabled{true};

	/// @brief Draw the texture of the sprite
	void Draw()
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
	Texture2D texture;

	bool isTextureSet{false};

	float attackSpeed{1};

	float damage{0};

	void SetTexture()
	{
		if (isTextureSet)
			return;
		// load the sprite of correct item
		if (id == 0)
		{
			texture = textures["GREAT_SWORD"];
			damage = 20.0f;
			maxDurability = 100;
			attackSpeed = 2;
		}
		else if (id == 1)
		{
			texture = textures["WOODEN_SWORD"];
			damage = 1.55f;
			maxDurability = 70;
			attackSpeed = 1;
		}
		else if (id == 2)
		{
			texture = textures["SILVER_SWORD"];
			damage = 5.0f;
			maxDurability = 250;
			attackSpeed = 0.5f;
		}
		else if (id == 3)
		{
			texture = textures["GOLDEN_SWORD"];
			damage = 12.0f;
			maxDurability = 125;
			attackSpeed = 1;
		}
		else if (id == 4)
		{
			texture = textures["BLOODY_SWORD"];
			damage = 15.0f;
			maxDurability = 200;
			attackSpeed = 1.25f;
		}
		else if (id == 5)
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
	Texture2D textures[3];
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

		DrawTexturePro(texture, Rectangle{0, 0, texture.width * 1.0f, texture.height * 1.0f}, Rectangle{x, y, BLOCK_WIDTH * 0.4f, BLOCK_HEIGHT * 0.4f}, Vector2{BLOCK_WIDTH * 0.2f, BLOCK_HEIGHT * 0.2f}, rotation, Color{245, 245, 245, alpha});
	}
};

/// @brief main character
struct Character : Sprite
{
	/// @brief the background hotbar slot
	Texture2D hotbarSlot;
	/// @brief the background hotbar slot for selected item
	Texture2D hotbarSlotSelected;
	/// @brief The slot (0-5) that the player is holding
	int selectedItemSlot{0};
	/// @brief Items in the hotbar that the player is not holding
	Item hotbarItems[6];
	/// @brief every item the player is carring
	Texture2D inventory[10];
	/// @brief animation in which the player cycle throught when they are idle/still
	Texture2D idleStates[4];
	/// @brief animation in which the player cycle throught when they are walkiing
	Texture2D walkStates[6];
	/// @brief full heart icon
	Texture2D fullHealth{};
	Sound attackingSoundeffect;
	Sound missSoundeffect;

	Texture2D dashIcon{};
	/// @brief half heart icon
	Texture2D halfHealth{};
	/// @brief the hitbox of the main item
	Rectangle itemCollider{};

	Sound dash_sound_effect{};

	float dashes{3};
	int maxDashes{3};

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
	float radius;
	/// @brief keeps track of the direction the player is facing
	short facing{0};
	/// @brief the speed of the player (x)
	int speedX;
	/// @brief the speed of the player (y)
	int speedY;
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
	Texture2D footstepTextures[3];
	float timeSinceLastFootstep{0};

	/// @brief decreases the players health(with defence in mind)
	/// @param damage
	void Damage(int damage)
	{
		// calculate the amout of health removed
		health -= damage - (damage * (defence / 100));

		// check if player is dead
		if (health < 1)
		{
			// stop game
			shoudlBePlaying = false;
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

		dashes = maxDashes;

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
		SetSoundVolume(missSoundeffect, 2.0f);

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
				footsteps.push_back(Particle{footstepTextures, 0.06f, x, y + BLOCK_HEIGHT / 2, 0.1f, 90, 1});
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

		for (int i = 0; i < floor(dashes); i++)
		{
			DrawTexture(dashIcon, (BLOCK_WIDTH / 2.5) * i + BLOCK_WIDTH / 4 - cam.offset.x, BLOCK_HEIGHT * 2, RAYWHITE);
		}

		for (int i = 0; i < 6; i++)
		{
			if (i == selectedItemSlot)
				DrawImage((i + 0.5f) - cam.offset.x / BLOCK_WIDTH, 0.5f, hotbarSlotSelected);
			else
				DrawImage((i + 0.5f) - cam.offset.x / BLOCK_WIDTH, 0.5f, hotbarSlot);
			DrawTexturePro(hotbarItems[i].texture, Rectangle{0, 0, hotbarItems[i].texture.width * 1.0f, hotbarItems[i].texture.height * 1.0f}, Rectangle{(i + 1) * BLOCK_WIDTH - (BLOCK_WIDTH * 0.475f) - cam.offset.x, BLOCK_HEIGHT * 0.55f, BLOCK_WIDTH * 0.7f, BLOCK_HEIGHT * 0.7f}, Vector2{BLOCK_WIDTH * 0.35f, BLOCK_HEIGHT * 0.35f}, 0, RAYWHITE);
		}
	}

	/// @brief returns true if the player is colliding with a rectangle
	/// @param collider
	/// @return
	bool CheckCollision(Rectangle collider)
	{
		return CheckCollisionCircleRec(Vector2{x * 1.0f, y * 1.0f}, radius, collider);
	}

	/// @brief Draw the main hand item
	void DrawMainHand()
	{

		timeSinceLastAttack += GetFrameTime();

#if DEBUG == 1
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
#if DEBUG == 1
		DrawRectangleRec(itemCollider, Color{200, 0, 0, 90});
#endif
	}

	void Dash()
	{
		if (dashes > 0)
		{
			if (facing == 0)
			{
				for (int i = 0; i < 100 + agility * 2; i++)
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
			dashes--;
		}
	}

} character;

/// @brief Default chest
struct Chest : Sprite
{
	/// @brief define that range in which a player must be to open it
	Rectangle openRange;
	/// @brief define the item inside the chest
	Item item;
	/// @brief keep track if the chest is open
	bool isOpen;

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

	int facing{0};

	Texture2D idleAnimations[4];
	Texture2D walkingAnimations[4];

	Rectangle collider;

	/// @brief initailize the default values of an anemy
	/// @param x
	/// @param y
	/// @param isAware
	/// @param awarenessRadius
	/// @param texture
	/// @param damage
	/// @param range
	Enemy(int x, int y, bool isAware, float awarenessRadius, Texture2D texture, int damage, float range, int attackSpeed, Rectangle collider)
	{
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
		if (id == 0)
		{
			isAware = false;
			awarenessRadius = 300.0f;

			idleAnimations[0] = enemyIdle["GOBLIN"][0];
			idleAnimations[1] = enemyIdle["GOBLIN"][1];
			idleAnimations[2] = enemyIdle["GOBLIN"][2];
			idleAnimations[3] = enemyIdle["GOBLIN"][3];

			walkingAnimations[0] = enemyWalk["GOBLIN"][0];
			walkingAnimations[1] = enemyWalk["GOBLIN"][1];
			walkingAnimations[2] = enemyWalk["GOBLIN"][2];
			walkingAnimations[3] = enemyWalk["GOBLIN"][3];

			damage = 3;
			range = 30.0f;
			attackSpeed = 1;
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

		// check which way the player is facing
		if (facing == 1)
			// load the player texture backwords
			DrawTexturePro(texture, Rectangle{0, 0, -texture.width * 1.0f, texture.height * 1.0f}, Rectangle{x, y, BLOCK_WIDTH * 1.0f * (BLOCK_WIDTH / (texture.width == 0 ? 1 : texture.width)), BLOCK_HEIGHT * 1.0f * (BLOCK_HEIGHT / (texture.height == 0 ? 1 : texture.height))}, Vector2{texture.width * 0.5f, texture.height * 0.5f}, 0, RAYWHITE);
		else
			// load the normal player texture
			DrawTexturePro(texture, Rectangle{0, 0, texture.width * 1.0f, texture.height * 1.0f}, Rectangle{x, y, BLOCK_WIDTH * 1.0f * (BLOCK_WIDTH / (texture.width == 0 ? 1 : texture.width)), BLOCK_HEIGHT * 1.0f * (BLOCK_HEIGHT / (texture.height == 0 ? 1 : texture.height))}, Vector2{texture.width * 0.5f, texture.height * 0.5f}, 0, RAYWHITE);
	}

	/// @brief Sets the values of x and y
	void SetXY()
	{
		float tempX = x;
		float tempY = y;
		// check if an enemy is aware of the player
		if (isAware)
		{
			// try to set the x and y of the enemy to matche the players
			if (character.x > x)
			{
				// acounting for diagnal movement
				if (character.y > y)
				{
					x++;
					y++;
					facing = 0;
				}
				else if (character.y < y)
				{
					x++;
					y--;
					facing = 0;
				}
				else
				{
					x += 2;
					facing = 0;
				}
			}
			else if (character.x < x)
			{
				// acounting for diagnal movement
				if (character.y > y)
				{
					x--;
					y++;
					facing = 1;
				}

				else if (character.y < y)
				{
					x--;
					y--;
					facing = 1;
				}
				else
				{
					x -= 2;
					facing = 1;
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
		}

		for (auto collider : collidersX)
		{
			if (CheckCollisionRecs(collider, this->collider))
			{
				x = tempX;
			}
		}

		for (auto collider : collidersY)
		{
			if (CheckCollisionRecs(collider, this->collider))
			{
				y = tempY;
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
#if DEBUG == 1
		DrawText(std::to_string(timeFromLastAttack).c_str(), 200, 100, 22, DARKGREEN);
#endif
		if (CheckCollisionCircles(Vector2{character.x * 1.0f, character.y * 1.0f}, character.radius, Vector2{x * 1.0f, y * 1.0f}, range) && timeFromLastAttack > attackSpeed)
		{
			timeFromLastAttack = 0;
			character.Damage(damage);
		}
	}

	void Damage(int amount)
	{
		health -= amount;
		onFocus = true;
		timeFromFocus = 0.0f;
		if (health < 1)
		{
			isEnabled = false;
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
		if (timeFromFocus > 14 || isAware == false)
			onFocus = false;
		if (onFocus)
		{
			DrawRectangle(x - BLOCK_WIDTH * (BLOCK_WIDTH / (texture.width == 0 ? 1 : texture.width)) / 2.5f, y - texture.height / 3, BLOCK_WIDTH, BLOCK_HEIGHT / 8, Color{80, 80, 80, 255});
			DrawRectangle(x - BLOCK_WIDTH * (BLOCK_WIDTH / (texture.width == 0 ? 1 : texture.width)) / 2, y - texture.height / 3, (BLOCK_WIDTH * health) / maxHealth, BLOCK_HEIGHT / 8, Color{230, 41, 55, 255});
		}
	}
};

std::vector<Enemy> enemies;

/// @brief Vector with all chests
/// @param state the state in which the chest is in(closed, open, opening)
/// @param hasItem if the chest has and item or not
/// @param Chest the chest object
std::vector<std::pair<std::pair<int, bool>, Chest>> chests;
#pragma endregion Structs

#pragma region Main
int main(void)
{
	// clear the chest vector
	chests.clear();

	if (SCREEN_HEIGHT > SCREEN_WIDTH)
	{
		BLOCK_WIDTH = SCREEN_WIDTH / 20;
		BLOCK_HEIGHT = SCREEN_WIDTH / 20;
	}
	else
	{
		BLOCK_WIDTH = SCREEN_HEIGHT / 20;
		BLOCK_HEIGHT = SCREEN_HEIGHT / 20;
	}

	SetConfigFlags(FLAG_MSAA_4X_HINT);

	// initialize raylib and the screen
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Endure");
	SetWindowState(FLAG_VSYNC_HINT);
	// initialize audio
	InitAudioDevice();

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
	character.radius = 10.0f;
	character.speedX = 300;
	character.speedY = 300;
	character.selectedItemSlot = 0;
	SetSoundVolume(character.attackingSoundeffect, 2.0f);

	// initialize hotbar
	for (int i = 0; i < 6; i++)
	{
		character.hotbarItems[i].id = -1;
	}

	// hide curser
	HideCursor();

	// load the custom curser texture
	cursor.texture = textures["MOUSE"];
	SetTargetFPS(60);

	// main loop
	while (!WindowShouldClose() && shoudlBePlaying)
	{
		cam.offset = Vector2{-character.x + SCREEN_WIDTH / 2, 0};
		// set curser possition
		cursor.x = GetMouseX() - BLOCK_WIDTH / (cursor.texture.width == 0 ? 1 : cursor.texture.width) / 2 - cam.offset.x;
		cursor.y = GetMouseY() - BLOCK_HEIGHT / (cursor.texture.height == 0 ? 1 : cursor.texture.height) / 2;
		// set idle to true
		character.isIdle = true;

		character.maxDashes = 3 + (character.stamina / 10);

		if (character.dashes < character.maxDashes)
		{
			character.dashes += GetFrameTime() / (1.5f - character.stamina * 0.1f);
		}
		else
		{
			character.dashes = character.maxDashes;
		}

		// x and y of player at the start of the frame(used for colition)
		int tempX = character.x;
		int tempY = character.y;
		// check if the hotbar changed
		if (GetMouseWheelMove() == 1)
			// increment the slot
			character.selectedItemSlot++;
		else if (GetMouseWheelMove() == -1)
			// decrament the slot
			character.selectedItemSlot--;
		if (character.selectedItemSlot < 0)
			// corent ite's value
			character.selectedItemSlot = 5;
		// prevent overflow
		character.selectedItemSlot %= 6;
		// check if the player should move
		if (IsKeyDown(KEY_W))
		{
			// set idle to false
			character.isIdle = false;

			// move player while acounting for diagnal movement
			if (IsKeyDown(KEY_D))
			{
				// change the direction in whichg the player is facing(to the right)
				character.facing = 0;

				// change x and y equaly
				character.x += character.speedX / 1.5f * GetFrameTime();
				character.y -= character.speedY / 1.5f * GetFrameTime();
			}
			else if (IsKeyDown(KEY_A))
			{
				// change the direction in whichg the player is facing(to the left)
				character.facing = 1;

				// change x and y equaly
				character.x -= character.speedX / 1.5f * GetFrameTime();
				character.y -= character.speedY / 1.5f * GetFrameTime();
			}
			else
			{
				// change the player y possition
				character.y -= character.speedY * GetFrameTime();
			}
		}
		else if (IsKeyDown(KEY_S))
		{
			// set idle to false
			character.isIdle = false;

			// move player while acounting for diagnal movement
			if (IsKeyDown(KEY_D))
			{
				// change the direction in whichg the player is facing(to the right)
				character.facing = 0;

				// change x and y equaly
				character.x += character.speedX / 1.5f * GetFrameTime();
				character.y += character.speedY / 1.5f * GetFrameTime();
			}
			else if (IsKeyDown(KEY_A))
			{
				// change the direction in whichg the player is facing(to the left)
				character.facing = 1;

				// change x and y equaly
				character.x -= character.speedX / 1.5f * GetFrameTime();
				character.y += character.speedY / 1.5f * GetFrameTime();
			}
			else
			{
				// change the player y possition
				character.y += character.speedY * GetFrameTime();
			}
		}
		else if (IsKeyDown(KEY_D))
		{
			// change the direction in whichg the player is facing(to the right)
			character.facing = 0;
			// set idle to false
			character.isIdle = false;
			// change the x possition of the player
			character.x += character.speedX * GetFrameTime();
		}
		else if (IsKeyDown(KEY_A))
		{
			// change the direction in whichg the player is facing(to the left)
			character.facing = 1;
			// set idle to false
			character.isIdle = false;
			// change the x possition of the player
			character.x -= character.speedX * GetFrameTime();
		}

		// check for collition sideways
		for (auto collider : collidersX)
		{
			// check if the player is colliding the all x colliders
			if (character.CheckCollision(collider))
			{
				// sets players x possition to the possition at the start of the frame
				character.x = tempX;
			}
		}

		// check for collition upwards or downwards
		for (auto collider : collidersY)
		{
			// check if the player is colliding the all y colliders
			if (character.CheckCollision(collider))
			{
				// sets players y possition to the possition at the start of the frame
				character.y = tempY;
			}
		}

		if (IsKeyPressed(KEY_SPACE))
		{
			character.Dash();
		}

#if DEBUG == 1
		if (IsKeyPressed(KEY_KP_1))
		{
			character.Damage(1);
		}
		if (IsKeyPressed(KEY_KP_2))
		{
			character.Damage(2);
		}
		if (IsKeyPressed(KEY_KP_3))
		{
			character.Damage(3);
		}
		if (IsKeyPressed(KEY_KP_4))
		{
			character.Damage(4);
		}
		if (IsKeyPressed(KEY_KP_5))
		{
			character.Damage(5);
		}
		if (IsKeyPressed(KEY_KP_6))
		{
			character.Damage(6);
		}
		if (IsKeyPressed(KEY_KP_7))
		{
			character.Damage(7);
		}
		if (IsKeyPressed(KEY_KP_8))
		{
			character.Damage(8);
		}
		if (IsKeyPressed(KEY_KP_9))
		{
			character.Damage(9);
		}
		if (IsKeyPressed(KEY_KP_0))
		{
			character.Damage(10);
		}
		if (IsKeyPressed(KEY_M))
		{
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
		if (!IsSoundPlaying(mainMusic))
		{
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
		character.rotation = PI / 2 + atan2(character.y - cursor.y, character.x - cursor.x);
		if (character.goal != 0)
		{
			for (auto &enemy : enemies)
			{
				if (CheckCollisionRecs(character.itemCollider, enemy.collider) && character.attacking)
				{
					enemy.Damage((character.hotbarItems[character.selectedItemSlot].damage + character.strength * 2) * 22);
					character.hotbarItems[character.selectedItemSlot].durability--;
					PlaySoundMulti(character.attackingSoundeffect);
					attacked = true;
				}
			}
		}

		if (attacked)
		{
			character.attacking = false;
		}

		if (character.attacking)
		{
			attacked = false;
		}

		if (IsKeyPressed(KEY_ESCAPE))
		{
			if (isPaused)
			{
				isPaused = false;
			}
			else
			{
				isPaused = true;
			}
		}

		for (auto door : doors)
		{
			if (character.CheckCollision(door.first))
			{
				if (IsKeyPressed(KEY_E))
					currentRoom = door.second;
				else
					DrawImage(door.first.x * 1.0f, door.first.y * 1.0f - BLOCK_HEIGHT, textures["E_KEY"]);
			}
		}

		BeginDrawing();
		{
			// clear the background
			ClearBackground(Color{52, 116, 235, 100});
			// begin the camera
			BeginMode2D(cam);
			// load the correct room
			loadRoom(currentRoom);
			character.DrawPlayerUI();

#if DEBUG == 1
			int size = 5000;
			// draw a grid (20x20)
			for (float i = -size; i <= size; i += SCREEN_HEIGHT / 20)
			{
				DrawLine(i, -size, i, size, GRAY);
				DrawLine(-size, i, size, i, GRAY);
			}
			DrawFPS(10, 40);
			// display the mainitems rotation
			DrawText(std::to_string(atan2(character.y - GetMouseY(), character.x - GetMouseX())).c_str(), 10, 70, 22, DARKGREEN);
			DrawText(std::to_string(character.dashes).c_str(), 100, 100, 20, DARKGREEN);
#endif
			character.Draw();
			character.DrawMainHand();

			if (isPaused)
				DrawPauseMenu();

			cursor.Draw(Color{184, 81, 55, 255});
		}
		EndDrawing();
	}

	// unload textures/sounds
	StopSoundMulti();
	UnloadTexture(character.texture);
	for (auto &texture : textures)
	{
		UnloadTexture(texture.second);
	}
	for (auto &f : fx)
	{
		UnloadSound(f.second);
	}
	for (auto &statIcon : statIcons)
	{
		UnloadTexture(statIcon.second);
	}
	UnloadTexture(character.texture);
	for (auto &i : characterAnimationIdle)
	{
		UnloadTexture(i.second);
	}
	for (auto &i : characterAnimationWalk)
	{
		UnloadTexture(i.second);
	}
	for (auto &i : buttons)
	{
		UnloadTexture(i.second);
	}
	for (auto &i : playerUI)
	{
		UnloadTexture(i.second);
	}
	for (auto &i : particles)
	{
		UnloadTexture(i.second);
	}
	for (auto &i : enemyIdle)
	{
		for (auto &j : i.second)
		{
			UnloadTexture(j);
		}
	}
	for (auto &i : enemyWalk)
	{
		for (auto &j : i.second)
		{
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

#pragma region Funtions

float clickAnimation = 0.0f;
int clickedButton = 0;
int alpha = 230;

void DrawPauseMenu()
{
	if (character.isIdle && alpha == 100)
		alpha = 230;
	else if (!character.isIdle && alpha == 230)
		alpha = 100;

	DrawRectangleRounded(Rectangle{BLOCK_WIDTH * 5 - 10 - cam.offset.x, BLOCK_HEIGHT * 2.0f - 10, BLOCK_WIDTH * 10.0f + 20, BLOCK_HEIGHT * 16.0f + 20}, 0.02f, 100, Color{63, 40, 50, alpha});
	DrawRectangleRounded(Rectangle{BLOCK_WIDTH * 5 - 5 - cam.offset.x, BLOCK_HEIGHT * 2.0f - 5, BLOCK_WIDTH * 10.0f + 10, BLOCK_HEIGHT * 16.0f + 10}, 0.02f, 100, Color{228, 166, 114, alpha});
	DrawRectangleRounded(Rectangle{BLOCK_WIDTH * 5 - cam.offset.x, BLOCK_HEIGHT * 2.0f, BLOCK_WIDTH * 10.0f, BLOCK_HEIGHT * 16.0f}, 0.02f, 100, Color{234, 212, 170, alpha});

	if ((CheckCollisionRecs(Rectangle{cursor.x - (cursor.texture.width * BLOCK_WIDTH / (cursor.texture.width == 0 ? 1 : cursor.texture.width)) / 2, cursor.y - cursor.texture.height * BLOCK_HEIGHT / (cursor.texture.height == 0 ? 1 : cursor.texture.height) / 2, cursor.texture.width * BLOCK_HEIGHT / (cursor.texture.height == 0 ? 1 : cursor.texture.height), cursor.texture.height * BLOCK_HEIGHT / (cursor.texture.height == 0 ? 1 : cursor.texture.height)}, Rectangle{SCREEN_WIDTH / 2 - (back_button.width * BLOCK_WIDTH / (back_button.width == 0 ? 1 : back_button.width)) / 2 - cam.offset.x, BLOCK_WIDTH * 3.0f, back_button.width * 1.0f, back_button.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
	{
		DrawTexture(back_button_pressed, SCREEN_WIDTH / 2 - back_button_pressed.width * BLOCK_WIDTH * 3 / (back_button_pressed.width == 0 ? 1 : back_button_pressed.width) / 2 - cam.offset.x, SCREEN_WIDTH * 0.15f + BLOCK_HEIGHT * 14 / back_button_pressed.width, Color{255, 255, 255, alpha});

		clickAnimation = 0.25f;
		clickedButton = 1;
	}
	else if (clickAnimation < -0.1f && clickedButton == 1)
	{
		alpha -= 19;
	}
	else if (clickAnimation > 0 && clickedButton == 1)
	{
		clickAnimation -= GetFrameTime();
		DrawTexture(back_button_pressed, SCREEN_WIDTH / 2 - back_button_pressed.width * BLOCK_WIDTH * 3 / (back_button_pressed.width == 0 ? 1 : back_button_pressed.width) / 2 - cam.offset.x, SCREEN_WIDTH * 0.15f + BLOCK_HEIGHT * 14 / back_button_pressed.width, Color{255, 255, 255, alpha});
	}
	else if (clickAnimation < 0)
	{
		clickAnimation -= GetFrameTime();
		DrawTexture(back_button, SCREEN_WIDTH / 2 - back_button.width * BLOCK_WIDTH * 3 / (back_button.width == 0 ? 1 : back_button.width) / 2 - cam.offset.x, SCREEN_WIDTH * 0.15f, Color{255, 255, 255, alpha});
	}
	else
		DrawTexture(back_button, SCREEN_WIDTH / 2 - back_button.width * BLOCK_WIDTH * 3 / (back_button.width == 0 ? 1 : back_button.width) / 2 - cam.offset.x, SCREEN_WIDTH * 0.15f, Color{255, 255, 255, alpha});

	if (alpha < 10)
	{
		isPaused = false;
		clickedButton = 0;
		alpha = 230;
	}

	if ((CheckCollisionRecs(Rectangle{cursor.x - cursor.texture.width * BLOCK_WIDTH / (cursor.texture.width == 0 ? 1 : cursor.texture.width) / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * BLOCK_WIDTH / (cursor.texture.width == 0 ? 1 : cursor.texture.width) * 1.0f, cursor.texture.height * BLOCK_HEIGHT / (cursor.texture.height == 0 ? 1 : cursor.texture.height) * 1.0f}, Rectangle{SCREEN_WIDTH / 2 - back_button.width / 2.0f - cam.offset.x, BLOCK_WIDTH * 5.0f, back_button.width * 1.0f, back_button.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
	{
		DrawTexture(exit_button_pressed, SCREEN_WIDTH / 2 - exit_button_pressed.width * BLOCK_WIDTH * 3 / (exit_button_pressed.width == 0 ? 1 : exit_button_pressed.width) / 2 - cam.offset.x, SCREEN_HEIGHT * 0.25f + BLOCK_HEIGHT * 14 / exit_button_pressed.width, Color{255, 255, 255, alpha});
		clickAnimation = 0.25f;
		clickedButton = 2;
	}
	else if (clickAnimation < -0.15f && clickedButton == 2)
	{
		shoudlBePlaying = false;
		clickedButton = 0;
	}
	else if (clickAnimation > 0 && clickedButton == 2)
	{
		clickAnimation -= GetFrameTime();
		DrawTexture(exit_button_pressed, SCREEN_WIDTH / 2 - exit_button_pressed.width * BLOCK_WIDTH * 3 / (exit_button_pressed.width == 0 ? 1 : exit_button_pressed.width) / 2 - cam.offset.x, SCREEN_HEIGHT * 0.25f + BLOCK_HEIGHT * 14 / exit_button_pressed.width, Color{255, 255, 255, alpha});
	}
	else if (clickAnimation < 0)
	{
		clickAnimation -= GetFrameTime();
		DrawTexture(exit_button, SCREEN_WIDTH / 2 - exit_button.width * BLOCK_WIDTH * 3 / (exit_button.width == 0 ? 1 : exit_button.width) / 2 - cam.offset.x, SCREEN_HEIGHT * 0.25f, Color{255, 255, 255, alpha});
	}
	else
		DrawTexture(exit_button, SCREEN_WIDTH / 2 - exit_button.width * BLOCK_WIDTH * 3 / (exit_button.width == 0 ? 1 : exit_button.width) / 2 - cam.offset.x, SCREEN_HEIGHT * 0.25f, Color{255, 255, 255, alpha});

	DrawTexture(statIcons["AGILITY_ICON"], BLOCK_WIDTH * 5.5f - cam.offset.x, BLOCK_HEIGHT * 8, Color{255, 255, 255, alpha});
	DrawTexture(statIcons["DEFENCE_ICON"], BLOCK_WIDTH * 5.5f - cam.offset.x, BLOCK_HEIGHT * 9.25f, Color{255, 255, 255, alpha});
	DrawTexture(statIcons["HEALTH_ICON"], BLOCK_WIDTH * 5.5f - cam.offset.x, BLOCK_HEIGHT * 10.5f, Color{255, 255, 255, alpha});
	DrawTexture(statIcons["LUCK_ICON"], BLOCK_WIDTH * 5.5f - cam.offset.x, BLOCK_HEIGHT * 11.75f, Color{255, 255, 255, alpha});
	DrawTexture(statIcons["M_DEFENCE_ICON"], BLOCK_WIDTH * 5.5f - cam.offset.x, BLOCK_HEIGHT * 13, Color{255, 255, 255, alpha});
	DrawTexture(statIcons["STAMINA_ICON"], BLOCK_WIDTH * 5.5f - cam.offset.x, BLOCK_HEIGHT * 14.25f, Color{255, 255, 255, alpha});
	DrawTexture(statIcons["STRENGTH_ICON"], BLOCK_WIDTH * 5.5f - cam.offset.x, BLOCK_HEIGHT * 15.5f, Color{255, 255, 255, alpha});
	DrawTexture(statIcons["VISABILITY_ICON"], BLOCK_WIDTH * 5.5f - cam.offset.x, BLOCK_HEIGHT * 16.75f, Color{255, 255, 255, alpha});

	// DrawText(std::to_string(character.agility).c_str(), BLOCK_WIDTH * 13, BLOCK_WIDTH * 8, 25, Color{255, 255, 255, alpha});

	if (character.statPoints != 0)
	{
		DrawTextEx(FatPixel, std::to_string(character.tempAgility).c_str(), Vector2{BLOCK_WIDTH * 8 - cam.offset.x, BLOCK_HEIGHT * 8.0f}, 25, 1, Color{255, 255, 255, alpha});
		DrawTextEx(FatPixel, std::to_string(character.tempDefence).c_str(), Vector2{BLOCK_WIDTH * 8 - cam.offset.x, BLOCK_HEIGHT * 9.25f}, 25, 1, Color{255, 255, 255, alpha});
		DrawTextEx(FatPixel, std::to_string(character.tempMaxHealth).c_str(), Vector2{BLOCK_WIDTH * 8 - cam.offset.x, BLOCK_HEIGHT * 10.5f}, 25, 1, Color{255, 255, 255, alpha});
		DrawTextEx(FatPixel, std::to_string(character.tempLuck).c_str(), Vector2{BLOCK_WIDTH * 8 - cam.offset.x, BLOCK_HEIGHT * 11.75f}, 25, 1, Color{255, 255, 255, alpha});
		DrawTextEx(FatPixel, std::to_string(character.tempM_defence).c_str(), Vector2{BLOCK_WIDTH * 8 - cam.offset.x, BLOCK_HEIGHT * 13.0f}, 25, 1, Color{255, 255, 255, alpha});
		DrawTextEx(FatPixel, std::to_string(character.tempStamina).c_str(), Vector2{BLOCK_WIDTH * 8 - cam.offset.x, BLOCK_HEIGHT * 14.25f}, 25, 1, Color{255, 255, 255, alpha});
		DrawTextEx(FatPixel, std::to_string(character.tempStrength).c_str(), Vector2{BLOCK_WIDTH * 8 - cam.offset.x, BLOCK_HEIGHT * 15.5f}, 25, 1, Color{255, 255, 255, alpha});
		DrawTextEx(FatPixel, std::to_string(character.tempVisability).c_str(), Vector2{BLOCK_WIDTH * 8 - cam.offset.x, BLOCK_HEIGHT * 16.75f}, 25, 1, Color{255, 255, 255, alpha});

		if (character.tempAgility != character.agility)
			DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 8.0f}, BLOCK_HEIGHT / 2, 1, Color{3, 186, 252, alpha});
		else
			DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 8.0f}, BLOCK_HEIGHT / 2, 1, Color{130, 130, 130, alpha});

		if (character.tempDefence != character.defence)
			DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 9.25f}, BLOCK_HEIGHT / 2, 1, Color{3, 186, 252, alpha});
		else
			DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 9.25f}, BLOCK_HEIGHT / 2, 1, Color{130, 130, 130, alpha});

		if (character.tempMaxHealth != character.maxHealth)
			DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 10.5f}, BLOCK_HEIGHT / 2, 1, Color{3, 186, 252, alpha});
		else
			DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 10.5f}, BLOCK_HEIGHT / 2, 1, Color{130, 130, 130, alpha});

		if (character.tempLuck != character.luck)
			DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 11.75f}, BLOCK_HEIGHT / 2, 1, Color{3, 186, 252, alpha});
		else
			DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 11.75f}, BLOCK_HEIGHT / 2, 1, Color{130, 130, 130, alpha});

		if (character.tempM_defence != character.m_defence)
			DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 13.0f}, 25, BLOCK_HEIGHT / 2, Color{3, 186, 252, alpha});
		else
			DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 13.0f}, 25, BLOCK_HEIGHT / 2, Color{130, 130, 130, alpha});

		if (character.tempStamina != character.stamina)
			DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 14.25f}, BLOCK_HEIGHT / 2, 1, Color{3, 186, 252, alpha});
		else
			DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 14.25f}, BLOCK_HEIGHT / 2, 1, Color{130, 130, 130, alpha});

		if (character.tempStrength != character.strength)
			DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 15.5f}, BLOCK_HEIGHT / 2, 1, Color{3, 186, 252, alpha});
		else
			DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 15.5f}, BLOCK_HEIGHT / 2, 1, Color{130, 130, 130, alpha});

		if (character.tempVisability != character.visability)
			DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 16.75f}, BLOCK_HEIGHT / 2, 1, Color{3, 186, 252, alpha});
		else
			DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 16.75f}, BLOCK_HEIGHT / 2, 1, Color{130, 130, 130, alpha});

		DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.tempAgility).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 8.0f}, 25, 1, Color{3, 186, 252, alpha});
		DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.tempDefence).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 9.25f}, 25, 1, Color{3, 186, 252, alpha});
		DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.tempMaxHealth).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 10.5f}, 25, 1, Color{3, 186, 252, alpha});
		DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.tempLuck).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 11.75f}, 25, 1, Color{3, 186, 252, alpha});
		DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.tempM_defence).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 13.0f}, 25, 1, Color{3, 186, 252, alpha});
		DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.tempStamina).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 14.25f}, 25, 1, Color{3, 186, 252, alpha});
		DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.tempStrength).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 15.5f}, 25, 1, Color{3, 186, 252, alpha});
		DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.tempVisability).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 16.75f}, 25, 1, Color{3, 186, 252, alpha});

		if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 8.65f + (std::to_string(character.agility).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 8.25f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempStatPoints != 0)
		{
			character.tempStatPoints--;
			character.tempAgility++;
		}
		else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 8.65f + (std::to_string(character.tempDefence).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_WIDTH * 9.5f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempStatPoints != 0)
		{
			character.tempStatPoints--;
			character.tempDefence++;
		}
		else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 8.65f + (std::to_string(character.tempMaxHealth).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 10.75f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempStatPoints != 0)
		{
			character.tempStatPoints--;
			character.tempMaxHealth++;
		}
		else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 8.65f + (std::to_string(character.tempLuck).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 12.0f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempStatPoints != 0)
		{
			character.tempStatPoints--;
			character.tempLuck++;
		}
		else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 8.65f + (std::to_string(character.tempM_defence).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 13.25f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempStatPoints != 0)
		{
			character.tempStatPoints--;
			character.tempM_defence++;
		}
		else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 8.65f + (std::to_string(character.tempStamina).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 14.5f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempStatPoints != 0)
		{
			character.tempStatPoints--;
			character.tempStamina++;
		}
		else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 8.65f + (std::to_string(character.tempStrength).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 15.75f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempStatPoints != 0)
		{
			character.tempStatPoints--;
			character.tempStrength++;
		}
		else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 8.65f + (std::to_string(character.tempVisability).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 17.0f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempStatPoints != 0)
		{
			character.tempStatPoints--;
			character.tempVisability++;
		}
		// fghj
		else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 7.65f - cam.offset.x, BLOCK_HEIGHT * 8.25f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempAgility > character.agility)
		{
			character.tempStatPoints++;
			character.tempAgility--;
		}
		else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 7.65f - cam.offset.x, BLOCK_HEIGHT * 9.5f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempDefence > character.defence)
		{
			character.tempStatPoints++;
			character.tempDefence--;
		}
		else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 7.65f - cam.offset.x, BLOCK_HEIGHT * 10.75f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempMaxHealth > character.health)
		{
			character.tempStatPoints++;
			character.tempMaxHealth--;
		}
		else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 7.65f - cam.offset.x, BLOCK_HEIGHT * 12.0f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempLuck > character.luck)
		{
			character.tempStatPoints++;
			character.tempLuck--;
		}
		else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 7.65f - cam.offset.x, BLOCK_HEIGHT * 13.25f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempM_defence > character.m_defence)
		{
			character.tempStatPoints++;
			character.tempM_defence--;
		}
		else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 7.65f - cam.offset.x, BLOCK_HEIGHT * 14.5f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempStamina > character.stamina)
		{
			character.tempStatPoints++;
			character.tempStamina--;
		}
		else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 7.65f - cam.offset.x, BLOCK_HEIGHT * 15.75f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempStrength > character.strength)
		{
			character.tempStatPoints++;
			character.tempStrength--;
		}
		else if (CheckCollisionCircleRec(Vector2{BLOCK_WIDTH * 7.65f - cam.offset.x, BLOCK_HEIGHT * 17.0f}, 10.0f, Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && character.tempVisability > character.visability)
		{
			character.tempStatPoints++;
			character.tempVisability--;
		}

		if ((CheckCollisionRecs(Rectangle{cursor.x - cursor.texture.width / 2, cursor.y - cursor.texture.height / 2, cursor.texture.width * 1.0f, cursor.texture.height * 1.0f}, Rectangle{BLOCK_WIDTH * 9.25f + (std::to_string(character.visability).length() - 1) * 25 - cam.offset.x, BLOCK_HEIGHT * 7, check_button.width * 1.0f, check_button.height * 1.0f}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
		{
			DrawTexture(check_button_pressed, BLOCK_WIDTH * 9.25f + (std::to_string(character.visability).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 7 + check_button.height / 10, Color{255, 255, 255, alpha});
			clickAnimation = 0.25f;
			clickedButton = 3;
		}
		else if (clickAnimation < -0.15f && clickedButton == 3)
		{
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
		}
		else if (clickAnimation > 0 && clickedButton == 3)
		{
			clickAnimation -= GetFrameTime();
			DrawTexture(check_button_pressed, BLOCK_WIDTH * 9.25f + (std::to_string(character.visability).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 7 + check_button.height / 10, Color{255, 255, 255, alpha});
		}
		else if (clickAnimation < 0)
		{
			clickAnimation -= GetFrameTime();
			DrawTexture(check_button, BLOCK_WIDTH * 9.25f + (std::to_string(character.visability).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 7, Color{255, 255, 255, alpha});
		}
		else
			DrawTexture(check_button, BLOCK_WIDTH * 9.25f + (std::to_string(character.visability).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 7, Color{255, 255, 255, alpha});
	}
	else
	{

		DrawTextEx(FatPixel, std::to_string(character.agility).c_str(), Vector2{BLOCK_WIDTH * 8 - cam.offset.x, BLOCK_HEIGHT * 8.0f}, (BLOCK_HEIGHT / 2), 1, Color{255, 255, 255, alpha});
		DrawTextEx(FatPixel, std::to_string(character.defence).c_str(), Vector2{BLOCK_WIDTH * 8 - cam.offset.x, BLOCK_HEIGHT * 9.25f}, (BLOCK_HEIGHT / 2), 1, Color{255, 255, 255, alpha});
		DrawTextEx(FatPixel, std::to_string(character.maxHealth).c_str(), Vector2{BLOCK_WIDTH * 8 - cam.offset.x, BLOCK_HEIGHT * 10.5f}, (BLOCK_HEIGHT / 2), 1, Color{255, 255, 255, alpha});
		DrawTextEx(FatPixel, std::to_string(character.luck).c_str(), Vector2{BLOCK_WIDTH * 8 - cam.offset.x, BLOCK_HEIGHT * 11.75f}, (BLOCK_HEIGHT / 2), 1, Color{255, 255, 255, alpha});
		DrawTextEx(FatPixel, std::to_string(character.m_defence).c_str(), Vector2{BLOCK_WIDTH * 8 - cam.offset.x, BLOCK_HEIGHT * 13.0f}, (BLOCK_HEIGHT / 2), 1, Color{255, 255, 255, alpha});
		DrawTextEx(FatPixel, std::to_string(character.stamina).c_str(), Vector2{BLOCK_WIDTH * 8 - cam.offset.x, BLOCK_HEIGHT * 14.25f}, (BLOCK_HEIGHT / 2), 1, Color{255, 255, 255, alpha});
		DrawTextEx(FatPixel, std::to_string(character.strength).c_str(), Vector2{BLOCK_WIDTH * 8 - cam.offset.x, BLOCK_HEIGHT * 15.5f}, (BLOCK_HEIGHT / 2), 1, Color{255, 255, 255, alpha});
		DrawTextEx(FatPixel, std::to_string(character.visability).c_str(), Vector2{BLOCK_WIDTH * 8 - cam.offset.x, BLOCK_HEIGHT * 16.75f}, (BLOCK_HEIGHT / 2), 1, Color{255, 255, 255, alpha});
		DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 8.0f}, (BLOCK_HEIGHT / 2), 1, Color{130, 130, 130, alpha});
		DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 9.25f}, (BLOCK_HEIGHT / 2), 1, Color{130, 130, 130, alpha});
		DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 10.5f}, (BLOCK_HEIGHT / 2), 1, Color{130, 130, 130, alpha});
		DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 11.75f}, (BLOCK_HEIGHT / 2), 1, Color{130, 130, 130, alpha});
		DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 13.0f}, (BLOCK_HEIGHT / 2), 1, Color{130, 130, 130, alpha});
		DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 14.25f}, (BLOCK_HEIGHT / 2), 1, Color{130, 130, 130, alpha});
		DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 15.5f}, (BLOCK_HEIGHT / 2), 1, Color{130, 130, 130, alpha});
		DrawTextEx(FatPixel, "-", Vector2{BLOCK_WIDTH * 7.5f - cam.offset.x, BLOCK_HEIGHT * 16.75f}, (BLOCK_HEIGHT / 2), 1, Color{130, 130, 130, alpha});

		DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.agility).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 8.0f}, 25, 1, Color{130, 130, 130, alpha});
		DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.defence).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 9.25f}, 25, 1, Color{130, 130, 130, alpha});
		DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.maxHealth).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 10.5f}, 25, 1, Color{130, 130, 130, alpha});
		DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.luck).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 11.75f}, 25, 1, Color{130, 130, 130, alpha});
		DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.m_defence).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 13.0f}, 25, 1, Color{130, 130, 130, alpha});
		DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.stamina).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 14.25f}, 25, 1, Color{130, 130, 130, alpha});
		DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.strength).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 15.5f}, 25, 1, Color{130, 130, 130, alpha});
		DrawTextEx(FatPixel, "+", Vector2{BLOCK_WIDTH * 8.5f + (std::to_string(character.visability).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 16.75f}, 25, 1, Color{130, 130, 130, alpha});

		DrawTexture(check_button_locked, BLOCK_WIDTH * 9.25f + (std::to_string(character.visability).length() - 1) * (BLOCK_HEIGHT / 2) - cam.offset.x, BLOCK_HEIGHT * 7, Color{255, 255, 255, alpha});
	}

	DrawTextEx(FatPixel, std::to_string(character.tempStatPoints).c_str(), Vector2{BLOCK_WIDTH * 5.5f - cam.offset.x, BLOCK_HEIGHT * 7.0f}, (BLOCK_HEIGHT / 2), 1, Color{0, 82, 172, alpha});
	DrawTexture(statIcons["STAT_POINT_ICON"], BLOCK_WIDTH * 6 - cam.offset.x, BLOCK_HEIGHT * 7, Color{255, 255, 255, alpha});
}

/// @brief load the room
/// @param id room id
std::string levelData;
std::string temp;
void loadRoom(const int id)
{
	// remove all colliders
	collidersX.clear();
	collidersY.clear();

	if (id == TUTORIAL)
	{
		std::string path = "./resources/maps/level";
		path.append(std::to_string(id + 1));
		path.append(".json");
		level.open(path);
		if (!level.is_open())
		{
			std::cout << "Level failed to load\n";
			shoudlBePlaying = false;
			return;
		}
	}

	while (!level.eof())
	{
		level >> temp;
		levelData += temp;
	}

	json data = json::parse(levelData);

	auto map = data["map"];

	auto leveChests = data["chests"];
	int chestCounter = 0;

	auto levelDoors = data["doors"];
	int doorCounter = 0;

	auto enemiesLevel = data["emenies"];
	int enemyCounter = 0;

	auto style = data["style"].dump();

	for (int y = 0; y < map.size(); y++)
	{
		auto row = map[y].dump();
		for (int x = 0; x < row.size(); x++)
		{
			auto tile = row[x];
			if (style == "\"normal\"")
			{
				if (tile == '#' || tile == ' ' || tile == '"')
					continue;
				else if (tile == '-')
					DrawTextureWithCollider(x - 1, y + 0.43, textures["WALL"]);
				else if (tile == '.')
					DrawImage(x - 1, y + 0.43, textures["FLOOR"]);
				else if (tile == '|')
					DrawTextureWithCollider(x - 1, y + 0.43, textures["WALL_DOWN_LEFT"], 3);
				else if (tile == '/')
					DrawTextureWithCollider(x - 1, y + 0.43, textures["WALL_DOWN_RIGHT"], -3);
				else if (tile == '(')
					DrawTextureWithCollider(x - 1, y + 0.43, textures["WALL_CORNER_LEFT"]);
				else if (tile == ')')
					DrawTextureWithCollider(x - 1, y + 0.43, textures["WALL_CORNER_RIGHT"]);
				else if (tile == '{')
					DrawTextureWithCollider(x - 1, y + 0.43, textures["WALL_FRONT_CORNER_LEFT"]);
				else if (tile == '}')
					DrawTextureWithCollider(x - 1, y + 0.43, textures["WALL_FRONT_CORNER_RIGHT"]);
				else if (tile == 'c' || tile == 'C')
				{
					if (chests.size() == chestCounter)
					{
						chests.push_back({{1, true}, Chest{BLOCK_WIDTH * x, BLOCK_HEIGHT * (y + 1), 0, Rectangle{BLOCK_WIDTH * (x - 2.25f), BLOCK_HEIGHT * (y - 1.68f), BLOCK_WIDTH * 3.5f, BLOCK_HEIGHT * 3.5f}, Item{leveChests[chestCounter]["itemid"], leveChests[chestCounter].value("dura", 100), leveChests[chestCounter].value("qantity", 1)}, currentRoom}});
					}
					DrawImage(x - 1, (y + 0.43), textures["FLOOR"]);
					chestCounter++;
				}
				else if (tile == 'd')
				{
					if (levelDoors[doorCounter]["isWooden"] == 1)
					{
						doors.push_back({Rectangle{BLOCK_WIDTH * x - textures["DOOR_WOODEN"].width * 1.0f, BLOCK_HEIGHT * (y + 0.43f) - textures["DOOR_WOODEN"].height * 1.0f, BLOCK_WIDTH * 3.0f, BLOCK_HEIGHT * 2.0f}, levelDoors[doorCounter]["roomid"]});
						DrawTextureWithCollider(x - 1, (y + 0.43), textures["DOOR_WOODEN"]);
					}
					else if (levelDoors[doorCounter]["isOpen"] == 1)
					{
					}
					else
					{
						doors.push_back({Rectangle{BLOCK_WIDTH * x - textures["DOOR_FRONT"].width * 1.0f, BLOCK_HEIGHT * (y + 0.43f) - textures["DOOR_FRONT"].height * 1.0f, BLOCK_WIDTH * 3.0f, BLOCK_HEIGHT * 2.0f}, levelDoors[doorCounter]["roomid"]});
						DrawTextureWithCollider(x - 1, (y + 0.43), textures["DOOR_FRONT"]);
					}
					doorCounter++;
				}
				else if (tile == 'D')
				{
					doors.push_back({Rectangle{BLOCK_WIDTH * x - textures["DOOR"].width * 1.0f, BLOCK_HEIGHT * (y + 0.43f) - textures["DOOR"].height * 1.0f, BLOCK_WIDTH * 3.0f, BLOCK_HEIGHT * 2.0f}, levelDoors[doorCounter]["roomid"]});
					DrawTextureWithCollider(x - 1, (y + 0.43), textures["DOOR"]);
					doorCounter++;
				}
				else if (tile == 'e' || tile == 'E')
				{
					DrawImage(x - 1, (y + 0.43), textures["FLOOR"]);
					if (enemies.size() == enemyCounter)
					{
						enemies.push_back(Enemy{(x - 1) * BLOCK_WIDTH, (y + 0.43) * BLOCK_WIDTH, enemiesLevel[enemyCounter]["enemyid"]});
						enemyCounter++;
					}
				}
			}
			else if (style == "\"live_cave\"")
			{
				if (tile == '#' || tile == ' ' || tile == '"')
					continue;
				else if (tile == '-')
					DrawTextureWithCollider(x - 1, y, textures["WALL"]);
				else if (tile == '.')
					DrawImage(x - 1, y, textures["LIVE_CAVE_FLOOR"]);
				else if (tile == '|')
					DrawTextureWithCollider(x - 1, y, textures["WALL_DOWN_LEFT"], 3);
				else if (tile == '/')
					DrawTextureWithCollider(x - 1, y, textures["WALL_DOWN_RIGHT"], -3);
				else if (tile == '(')
					DrawTextureWithCollider(x - 1, y, textures["WALL_CORNER_LEFT"]);
				else if (tile == ')')
					DrawTextureWithCollider(x - 1, y, textures["WALL_CORNER_RIGHT"]);
				else if (tile == '{')
					DrawTextureWithCollider(x - 1, y, textures["WALL_FRONT_CORNER_LEFT"]);
				else if (tile == '}')
					DrawTextureWithCollider(x - 1, y, textures["WALL_FRONT_CORNER_RIGHT"]);
				else if (tile == 'c' || tile == 'C')
				{
					if (chests.size() == chestCounter)
					{
						chests.push_back({{1, true}, Chest{BLOCK_WIDTH * x, BLOCK_HEIGHT * (y + 1), 0, Rectangle{BLOCK_WIDTH * (x - 2.25f), BLOCK_HEIGHT * (y - 1.25f), BLOCK_WIDTH * 3.5f, BLOCK_HEIGHT * 3.5f}, Item{leveChests[chestCounter]["itemid"], leveChests[chestCounter].value("dura", 100), leveChests[chestCounter].value("qantity", 1)}, currentRoom}});
					}
					DrawTextureWithCollider(x, y, textures[leveChests[chestCounter]["backgroundid"].dump().c_str()]);
					chestCounter++;
				}
				else if (tile == 'd')
				{
					if (levelDoors[doorCounter]["isWooden"] == 1)
					{
						doors.push_back({Rectangle{BLOCK_WIDTH * x - textures["DOOR_WOODEN"].width * 1.0f, BLOCK_HEIGHT * y - textures["DOOR_WOODEN"].height * 1.0f, BLOCK_WIDTH * 3.0f, BLOCK_HEIGHT * 2.0f}, levelDoors[doorCounter]["roomid"]});
						DrawTextureWithCollider(x - 1, y, textures["DOOR_WOODEN"]);
					}
					else if (levelDoors[doorCounter]["isOpen"] == 1)
					{
					}
					else
					{
						doors.push_back({Rectangle{BLOCK_WIDTH * x - textures["DOOR_FRONT"].width * 1.0f, BLOCK_HEIGHT * y - textures["DOOR_FRONT"].height * 1.0f, BLOCK_WIDTH * 3.0f, BLOCK_HEIGHT * 2.0f}, levelDoors[doorCounter]["roomid"]});
						DrawTextureWithCollider(x - 1, y, textures["DOOR_FRONT"]);
					}
					doorCounter++;
				}
				else if (tile == 'D')
				{
					doors.push_back({Rectangle{BLOCK_WIDTH * x - textures["DOOR"].width * 1.0f, BLOCK_HEIGHT * y - textures["DOOR"].height * 1.0f, BLOCK_WIDTH * 3.0f, BLOCK_HEIGHT * 2.0f}, levelDoors[doorCounter]["roomid"]});
					DrawTextureWithCollider(x - 1, y, textures["DOOR"]);
					doorCounter++;
				}
				else if (tile == 'e' || tile == 'E')
				{
					DrawImage(x - 1, y, textures[enemiesLevel[enemyCounter]["backgroundid"].dump().c_str()]);
					if (enemies.size() == enemyCounter)
					{
						enemies.push_back(Enemy{(x - 1) * BLOCK_WIDTH, y * BLOCK_WIDTH, enemiesLevel[enemyCounter]["enemyid"]});
						enemyCounter++;
					}
				}
			}
		}
	}

	for (auto &enemy : enemies)
	{
		// set the enemies x and y value
		if (!enemy.isEnabled)
			continue;
		enemy.SetCollider();
		enemy.SetXY();
		enemy.SetAware();
		enemy.DealDamage();
		enemy.DrawHealth();
#if DEBUG == 1
		DrawCircle(enemy.x, enemy.y, enemy.awarenessRadius, Color{255, 0, 0, 70});
		DrawRectangleRec(enemy.collider, Color{0, 0, 255, 70});
#endif
		// display the enemies texture
		enemy.Draw();
	}

	// load chests
	for (auto &chestX : chests)
	{
		Chest &chest = chestX.second;
		int &state = chestX.first.first;
		bool &hasItem = chestX.first.second;
		bool isE = false;
		if (chest.room != currentRoom)
			continue;

		// check if the player is within range to open the chest
		if (character.CheckCollision(chest.openRange))
		{
			// check if the playe opened the chest or pressed e
			if (IsKeyPressed(KEY_E) || state > 1)
				// check if state is within range
				if (state < 300)
					// incrament state
					state += 1000 * GetFrameTime();
				else
					// set state to max value in range
					state = 300;

			if (state != 300)
				isE = true;

			// check if the player picked up the chest
			if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionCircleRec(Vector2{cursor.x * 1.0f, cursor.y * 1.0f}, 2, Rectangle{chest.x * 1.0f - BLOCK_WIDTH, chest.y * 1.0f - BLOCK_HEIGHT, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 1.0f}) && state == 300 && hasItem)
			{

				// check if the player can pick up the item in his selected slot
				if (character.hotbarItems[character.selectedItemSlot].id == -1)
				{
					// add character to players inventory
					character.hotbarItems[character.selectedItemSlot] = chest.item;
					character.hotbarItems[character.selectedItemSlot].SetTexture();
					// remove item from chest
					hasItem = false;
				}
				else
				{
					for (int i = 0; i < 6; i++)
					{
						// check if their is an available slot
						if (character.hotbarItems[i].id == -1)
						{
							// add character to players inventory
							character.hotbarItems[i] = chest.item;
							character.hotbarItems[i].SetTexture();
							// remove item from chest
							hasItem = false;
							break;
						}
					}
				}
			}
		}
		else
		{
			// check if the chset is open
			if (state > 1)
				// slowly close the chest if player is not within range
				state -= 1000 * GetFrameTime();
		}
		chest.SetTexture(state);
		chest.Draw();

		if (isE)
			DrawTexturePro(textures["E_KEY"], Rectangle{0, 0, textures["E_KEY"].width * 1.0f, textures["E_KEY"].height * 1.0f}, Rectangle{chest.x - BLOCK_WIDTH / 2.25f, chest.y - BLOCK_HEIGHT * 1.1f, BLOCK_WIDTH / 2.5f, BLOCK_HEIGHT / 2.5f}, Vector2{textures["E_KEY"].width / 2, textures["E_KEY"].height / 2}, 0, RAYWHITE);

		// check if the chest is open and if the player didn't pick up the item
		if (chest.isOpen && hasItem)
		{
			// load the correct texture for the item
			if (!chest.item.isTextureSet)
			{
				chest.item.SetTexture();
			}
			// Draw the item
			chest.DrawItem();
		}

		// correct the chests values
#if DEBUG == 1
		DrawRectangleRec(chest.openRange, Color{255, 0, 0, 90});
		DrawRectangleRec(Rectangle{chest.x * 1.0f - BLOCK_WIDTH, chest.y * 1.0f - BLOCK_WIDTH, BLOCK_HEIGHT * 1.0f, BLOCK_HEIGHT * 1.0f}, Color{135, 245, 66, 90});
		DrawText(std::to_string(state).c_str(), chest.x, chest.y - 30, 20, RED);
#endif
		collidersX.push_back(Rectangle{chest.x * 1.0f - BLOCK_WIDTH, chest.y * 1.0f - BLOCK_HEIGHT, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 1.0f});
		collidersY.push_back(Rectangle{chest.x * 1.0f - BLOCK_WIDTH, chest.y * 1.0f - BLOCK_HEIGHT, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 1.0f});
	}
}

/// @brief Draw image in the grid
/// @param x 0-20 (in grid)
/// @param y  0-20 (in grid)
/// @param texture
void DrawImage(float x, float y, Texture2D texture)
{
	DrawTexturePro(texture, Rectangle{0, 0, texture.width * 1.0f, texture.height * 1.0f}, Rectangle{x * BLOCK_WIDTH, y * BLOCK_HEIGHT, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 1.0f}, Vector2{texture.width * 0.5f, texture.height * 0.5f}, 0, RAYWHITE);
}
void DrawTextureWithCollider(float x, float y, Texture2D texture)
{
	DrawTexturePro(texture, Rectangle{0, 0, texture.width * 1.0f, texture.height * 1.0f}, Rectangle{x * BLOCK_WIDTH, y * BLOCK_HEIGHT, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 1.0f}, Vector2{texture.width * 0.5f, texture.height * 0.5f}, 0, RAYWHITE);

	collidersX.push_back(Rectangle{BLOCK_WIDTH * x, BLOCK_HEIGHT * y, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 1.0f});
	collidersY.push_back(Rectangle{BLOCK_WIDTH * x, BLOCK_HEIGHT * y, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 1.0f});

#if DEBUG == 1
	DrawRectangleRec(Rectangle{BLOCK_WIDTH * x, BLOCK_HEIGHT * y, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 1.0f}, Color{135, 245, 66, 90});
#endif
}

void DrawTextureWithCollider(float x, float y, Texture2D texture, float widthScale)
{
	DrawTexturePro(texture, Rectangle{0, 0, texture.width * 1.0f, texture.height * 1.0f}, Rectangle{x * BLOCK_WIDTH, y * BLOCK_HEIGHT, BLOCK_WIDTH * 1.0f, BLOCK_HEIGHT * 1.0f}, Vector2{texture.width * 0.5f, texture.height * 0.5f}, 0, RAYWHITE);

	if (widthScale > 0)
	{
		collidersX.push_back(Rectangle{BLOCK_WIDTH * x, BLOCK_HEIGHT * y, BLOCK_WIDTH / widthScale, BLOCK_HEIGHT * 1.0f});
		collidersY.push_back(Rectangle{BLOCK_WIDTH * x, BLOCK_HEIGHT * y, BLOCK_WIDTH / widthScale, BLOCK_HEIGHT * 1.0f});
	}
	else
	{
		widthScale = -widthScale;
		collidersX.push_back(Rectangle{BLOCK_WIDTH * x + (BLOCK_WIDTH / widthScale * widthScale - 1), BLOCK_HEIGHT * y, BLOCK_WIDTH / widthScale, BLOCK_HEIGHT * 1.0f});
		collidersY.push_back(Rectangle{BLOCK_WIDTH * x + (BLOCK_WIDTH / widthScale * widthScale - 1), BLOCK_HEIGHT * y, BLOCK_WIDTH / widthScale, BLOCK_HEIGHT * 1.0f});
	}
}

#pragma endregion Funtions