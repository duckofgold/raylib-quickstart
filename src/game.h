#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>

#define WORLD_WIDTH 200
#define WORLD_HEIGHT 100 
#define BLOCK_SIZE 32
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800
#define INVENTORY_SIZE 9
#define MAX_REACH_DISTANCE 100.0f

typedef enum {
    BLOCK_AIR = 0,
    BLOCK_DIRT,
    BLOCK_STONE,
    BLOCK_GRASS,
    BLOCK_WATER,
    BLOCK_SAND,
    BLOCK_WOOD,
    BLOCK_LEAVES,
    BLOCK_COUNT
} BlockType;

typedef struct {
    BlockType type;
    int count;
} InventorySlot;

typedef struct {
    int x, y;
    float velX, velY;
    bool onGround;
    int health;
    InventorySlot inventory[INVENTORY_SIZE];
    int selectedSlot;
    float lastJumpTime;
    float lastClickTime;
} Player;

typedef struct {
    BlockType blocks[WORLD_HEIGHT][WORLD_WIDTH];
    Camera2D camera;
    Player player;
} World;

bool IsBlockSolid(BlockType block);
Color GetBlockColor(BlockType block);
const char* GetBlockName(BlockType block);

void InitGame(World* world);
void InitPlayer(Player* player);
void UpdatePlayer(World* world, float deltaTime);
void HandleBlockInteraction(World* world, float deltaTime);
void HandleInventoryInput(World* world);

void DrawWorld(World* world);
void DrawPlayer(World* world);
void DrawUI(World* world);
void DrawInventory(World* world);

void GenerateWorld(World* world);

#endif