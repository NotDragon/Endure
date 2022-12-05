#include "raylib.h"
#include <map>
#include <vector>

#ifndef TEXTURES_H
#define TEXTURES_H

#define TUTORIAL 000
#define TUTORIAL_BOSS 001

#define PAUSE 0

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else
#define GLSL_VERSION 100
#endif

std::map<const char *, Texture2D> textures;
std::map<const char *, Texture2D> statIcons;
std::map<const char *, Texture2D> characterAnimationIdle;
std::map<const char *, Texture2D> characterAnimationWalk;
std::map<const char *, Texture2D> buttons;
std::map<const char *, Texture2D> playerUI;
std::map<const char *, Texture2D> particles;

std::map<const char *, int> items;

std::map<const char *, Shader> shaders;

std::map<const char *, std::vector<Texture2D>> enemyIdle;
std::map<const char *, std::vector<Texture2D>> enemyWalk;

std::map<const char *, Sound> fx;

void InitResources()
{

    textures.insert({"DOOR_FRONT", LoadTexture("./resources/textures/map/door_front.png")});
    textures.insert({"DOOR_WOODEN", LoadTexture("./resources/textures/map/door_wooden.png")});
    textures.insert({"DOOR", LoadTexture("./resources/textures/map/door.png")});
    textures.insert({"FLOOR", LoadTexture("./resources/textures/map/floor.png")});
    textures.insert({"STAIRS_DOWN", LoadTexture("./resources/textures/map/stairs_down.png")});
    textures.insert({"STAIRS_UP", LoadTexture("./resources/textures/map/stairs_up.png")});
    textures.insert({"WALL_CENTER", LoadTexture("./resources/textures/map/wall_center.png")});
    textures.insert({"WALL_CORNER_LEFT", LoadTexture("./resources/textures/map/wall_corner_left.png")});
    textures.insert({"WALL_CORNER_RIGHT", LoadTexture("./resources/textures/map/wall_corner_right.png")});
    textures.insert({"WALL_DOWN_LEFT", LoadTexture("./resources/textures/map/wall_down_left.png")});
    textures.insert({"WALL_DOWN_RIGHT", LoadTexture("./resources/textures/map/wall_down_right.png")});
    textures.insert({"WALL_FRONT_CORNER_LEFT", LoadTexture("./resources/textures/map/wall_front_corner_left.png")});
    textures.insert({"WALL_FRONT_CORNER_RIGHT", LoadTexture("./resources/textures/map/wall_front_corner_right.png")});
    textures.insert({"WALL", LoadTexture("./resources/textures/map/wall.png")});
    textures.insert({"MOUSE", LoadTexture("./resources/textures/mouse.png")});
    textures.insert({"GRASS", LoadTexture("./resources/textures/decoretions/grass.png")});
    textures.insert({"ROCKS", LoadTexture("./resources/textures/map/door_front.png")});
    textures.insert({"ROCKS", LoadTexture("./resources/textures/decoretions/rocks.png")});
    textures.insert({"TREE_BIG_ROUND", LoadTexture("./resources/textures/decoretions/tree_big_round.png")});
    textures.insert({"TREE_BIG", LoadTexture("./resources/textures/decoretions/tree_big.png")});
    textures.insert({"TREE_ROUND", LoadTexture("./resources/textures/decoretions/tree_round.png")});
    textures.insert({"TREE", LoadTexture("./resources/textures/decoretions/tree.png")});
    textures.insert({"CHEST_CLOSED", LoadTexture("./resources/textures/decoretions/chest/chest_closed.png")});
    textures.insert({"CHEST_OPENING", LoadTexture("./resources/textures/decoretions/chest/chest_opening.png")});
    textures.insert({"CHEST_OPEN", LoadTexture("./resources/textures/decoretions/chest/chest_open.png")});
    textures.insert({"E_KEY", LoadTexture("./resources/textures/keys/e_key.png")});
    textures.insert({"LIVE_CAVE_FLOOR", LoadTexture("./resources/textures/map/live_cave/floor.png")});

    statIcons.insert({"AGILITY_ICON", LoadTexture("./resources/UI/StatIcons/agility_icon.png")});
    statIcons.insert({"DEFENCE_ICON", LoadTexture("./resources/UI/StatIcons/defence_icon.png")});
    statIcons.insert({"HEALTH_ICON", LoadTexture("./resources/UI/StatIcons/health_icon.png")});
    statIcons.insert({"LUCK_ICON", LoadTexture("./resources/UI/StatIcons/luck_icon.png")});
    statIcons.insert({"M_DEFENCE_ICON", LoadTexture("./resources/UI/StatIcons/magic_defence_icon.png")});
    statIcons.insert({"STAMINA_ICON", LoadTexture("./resources/UI/StatIcons/stamina_icon.png")});
    statIcons.insert({"STRENGTH_ICON", LoadTexture("./resources/UI/StatIcons/strength_icon.png")});
    statIcons.insert({"VISABILITY_ICON", LoadTexture("./resources/UI/StatIcons/visability_icon.png")});
    statIcons.insert({"STAT_POINT_ICON", LoadTexture("./resources/UI/StatIcons/stat_point.png")});

    buttons.insert({"CHECK_BUTTON", LoadTexture("./resources/UI/buttons/check_button.png")});
    buttons.insert({"CHECK_BUTTON_PRESSED", LoadTexture("./resources/UI/buttons/check_button_pressed.png")});
    buttons.insert({"CHECK_BUTTON_LOCKED", LoadTexture("./resources/UI/buttons/check_button_locked.png")});

    fx.insert({"PLAYER_ATTACK", LoadSound("./resources/sounds/playerAttackSoundeffect/attacking_soundeffect.wav")});
    fx.insert({"PLAYER_MISS", LoadSound("./resources/sounds/playerAttackSoundeffect/miss_soundeffect.wav")});
    fx.insert({"PLAYER_DASH", LoadSound("./resources/sounds/dash_sound_effect.wav")});

    characterAnimationIdle.insert({"IDLE_0", LoadTexture("./resources/textures/character/idle/character_idle_0.png")});
    characterAnimationIdle.insert({"IDLE_1", LoadTexture("./resources/textures/character/idle/character_idle_1.png")});
    characterAnimationIdle.insert({"IDLE_2", LoadTexture("./resources/textures/character/idle/character_idle_2.png")});
    characterAnimationIdle.insert({"IDLE_3", LoadTexture("./resources/textures/character/idle/character_idle_3.png")});

    characterAnimationWalk.insert({"WALK_0", LoadTexture("./resources/textures/character/walking/character_walk_0.png")});
    characterAnimationWalk.insert({"WALK_1", LoadTexture("./resources/textures/character/walking/character_walk_1.png")});
    characterAnimationWalk.insert({"WALK_2", LoadTexture("./resources/textures/character/walking/character_walk_2.png")});
    characterAnimationWalk.insert({"WALK_3", LoadTexture("./resources/textures/character/walking/character_walk_3.png")});
    characterAnimationWalk.insert({"WALK_4", LoadTexture("./resources/textures/character/walking/character_walk_4.png")});
    characterAnimationWalk.insert({"WALK_5", LoadTexture("./resources/textures/character/walking/character_walk_5.png")});

    playerUI.insert({"HEALTH_FULL", LoadTexture("./resources/UI/Health/heart_full.png")});
    playerUI.insert({"HEALTH_HALF", LoadTexture("./resources/UI/Health/heart_half.png")});
    playerUI.insert({"DASH_ICON", LoadTexture("./resources/UI/dash_icon.png")});
    playerUI.insert({"HOTBAR_SLOT", LoadTexture("./resources/UI/hotbar/hotbar_slot.png")});
    playerUI.insert({"HOTBAR_SELECTED", LoadTexture("./resources/UI/hotbar/hotbar_selected.png")});

    enemyIdle.insert({"GOBLIN", {LoadTexture("./resources/textures/enemies/goblin/idle/goblin_idle_anim_f0.png"), LoadTexture("./resources/textures/enemies/goblin/idle/goblin_idle_anim_f1.png"), LoadTexture("./resources/textures/enemies/goblin/idle/goblin_idle_anim_f2.png"), LoadTexture("./resources/textures/enemies/goblin/idle/goblin_idle_anim_f3.png")}});
    enemyWalk.insert({"GOBLIN", {LoadTexture("./resources/textures/enemies/goblin/walking/goblin_run_anim_f0.png"), LoadTexture("./resources/textures/enemies/goblin/walking/goblin_run_anim_f1.png"), LoadTexture("./resources/textures/enemies/goblin/walking/goblin_run_anim_f2.png"), LoadTexture("./resources/textures/enemies/goblin/walking/goblin_run_anim_f3.png")}});

    particles.insert({"FOOTSTEP_000", LoadTexture("./resources/particles/footstep_000.png")});
    particles.insert({"FOOTSTEP_001", LoadTexture("./resources/particles/footstep_001.png")});
    particles.insert({"FOOTSTEP_002", LoadTexture("./resources/particles/footstep_002.png")});

    items.insert({"GREAT_SWORD", 0});
    items.insert({"WOODEN_SWORD", 1});
    items.insert({"SILVER_SWORD", 2});
    items.insert({"GOLDEN_SWORD", 3});
    items.insert({"BLOODY_SWORD", 4});
    items.insert({"IRON_SWORD", 5});

    textures.insert({"GREAT_SWORD", LoadTexture("./resources/textures/items/great_sword.png")});
    textures.insert({"WOODEN_SWORD", LoadTexture("./resources/textures/items/wooden_sword.png")});
    textures.insert({"SILVER_SWORD", LoadTexture("./resources/textures/items/silver_sword.png")});
    textures.insert({"GOLDEN_SWORD", LoadTexture("./resources/textures/items/golden_sword.png")});
    textures.insert({"BLOODY_SWORD", LoadTexture("./resources/textures/items/bloody_sword.png")});
    textures.insert({"IRON_SWORD", LoadTexture("./resources/textures/items/iron_sword.png")});

    shaders.insert({"BLUR", LoadShader(0, TextFormat("./resources/shaders/blur.fs", GLSL_VERSION))});
}

#endif