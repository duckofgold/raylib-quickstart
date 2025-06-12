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
#define MAX_ANIMALS 20

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

typedef enum {
    ANIMAL_RABBIT = 0,
    ANIMAL_BIRD,
    ANIMAL_FISH,
    ANIMAL_PIG,
    ANIMAL_CHICKEN,
    ANIMAL_COUNT
} AnimalType;

typedef enum {
    AI_WANDER = 0,
    AI_FLEE,
    AI_FOLLOW,
    AI_SWIM
} AIState;

typedef struct {
    BlockType type;
    int count;
} InventorySlot;

typedef struct {
    int x, y;
    float velX, velY;
    bool onGround;
    bool inWater;
    int health;
    InventorySlot inventory[INVENTORY_SIZE];
    int selectedSlot;
    float lastJumpTime;
    float lastClickTime;
} Player;

typedef struct {
    AnimalType type;
    float x, y;
    float velX, velY;
    AIState state;
    float stateTimer;
    float direction;
    bool onGround;
    bool inWater;
    bool alive;
    float animTime;
} Animal;

typedef struct {
    BlockType blocks[WORLD_HEIGHT][WORLD_WIDTH];
    Camera2D camera;
    Player player;
    Animal animals[MAX_ANIMALS];
    int animalCount;
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
void DrawAnimals(World* world);

void GenerateWorld(World* world);
void InitAnimals(World* world);
void SpawnAnimal(World* world, AnimalType type, float x, float y);
void UpdateAnimals(World* world, float deltaTime);
Color GetAnimalColor(AnimalType type);
const char* GetAnimalName(AnimalType type);

#endif