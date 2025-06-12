#include "raylib.h"
#include "resource_dir.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define WORLD_WIDTH 200
#define WORLD_HEIGHT 100 
#define BLOCK_SIZE 32
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800

typedef enum {
    BLOCK_AIR = 0,
    BLOCK_DIRT,
    BLOCK_STONE,
    BLOCK_GRASS,
    BLOCK_WATER,
    BLOCK_SAND
} BlockType;

typedef struct {
    int x, y;
    float velX, velY;
    bool onGround;
    int health;
} Player;

typedef struct {
    BlockType blocks[WORLD_HEIGHT][WORLD_WIDTH];
    Camera2D camera;
    Player player;
} World;

float SimpleNoise(int x, int y) {
    int n = x + y * 57;
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

float PerlinNoise(float x, float y) {
    int xi = (int)x;
    int yi = (int)y;
    float xf = x - xi;
    float yf = y - yi;
    
    float a = SimpleNoise(xi, yi);
    float b = SimpleNoise(xi + 1, yi);
    float c = SimpleNoise(xi, yi + 1);
    float d = SimpleNoise(xi + 1, yi + 1);
    
    float i1 = a * (1 - xf) + b * xf;
    float i2 = c * (1 - xf) + d * xf;
    
    return i1 * (1 - yf) + i2 * yf;
}

void GenerateWorld(World* world) {
    srand(time(NULL));
    
    for (int x = 0; x < WORLD_WIDTH; x++) {
        float heightNoise = PerlinNoise(x * 0.1f, 0) * 0.5f + 0.5f;
        int surfaceHeight = (int)(heightNoise * 30) + 40;
        
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            if (y > surfaceHeight + 15) {
                world->blocks[y][x] = BLOCK_STONE;
            } else if (y > surfaceHeight + 5) {
                world->blocks[y][x] = BLOCK_DIRT;
            } else if (y > surfaceHeight) {
                world->blocks[y][x] = BLOCK_DIRT;
            } else if (y == surfaceHeight) {
                world->blocks[y][x] = BLOCK_GRASS;
            } else {
                world->blocks[y][x] = BLOCK_AIR;
            }
            
            if (y > 75 && world->blocks[y][x] == BLOCK_AIR && GetRandomValue(0, 100) < 30) {
                world->blocks[y][x] = BLOCK_WATER;
            }
        }
    }
    
    for (int i = 0; i < 50; i++) {
        int x = GetRandomValue(5, WORLD_WIDTH - 5);
        int y = GetRandomValue(60, WORLD_HEIGHT - 5);
        for (int dx = -2; dx <= 2; dx++) {
            for (int dy = -2; dy <= 2; dy++) {
                if (x + dx >= 0 && x + dx < WORLD_WIDTH && y + dy >= 0 && y + dy < WORLD_HEIGHT) {
                    if (dx * dx + dy * dy <= 4) {
                        world->blocks[y + dy][x + dx] = BLOCK_AIR;
                    }
                }
            }
        }
    }
}

void InitGame(World* world) {
    world->player.x = WORLD_WIDTH * BLOCK_SIZE / 2;
    world->player.y = 20 * BLOCK_SIZE;
    world->player.velX = 0;
    world->player.velY = 0;
    world->player.onGround = false;
    world->player.health = 100;
    
    world->camera.target = (Vector2){ world->player.x, world->player.y };
    world->camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    world->camera.rotation = 0.0f;
    world->camera.zoom = 1.0f;
    
    GenerateWorld(world);
}

bool IsBlockSolid(BlockType block) {
    return block != BLOCK_AIR && block != BLOCK_WATER;
}

bool CheckCollision(World* world, int x, int y) {
    int blockX = x / BLOCK_SIZE;
    int blockY = y / BLOCK_SIZE;
    
    if (blockX < 0 || blockX >= WORLD_WIDTH || blockY < 0 || blockY >= WORLD_HEIGHT) {
        return true;
    }
    
    return IsBlockSolid(world->blocks[blockY][blockX]);
}

void UpdatePlayer(World* world, float deltaTime) {
    Player* player = &world->player;
    
    float speed = 200.0f;
    float jumpForce = 400.0f;
    float gravity = 800.0f;
    
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        player->velX = -speed;
    } else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        player->velX = speed;
    } else {
        player->velX *= 0.8f;
    }
    
    if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) && player->onGround) {
        player->velY = -jumpForce;
        player->onGround = false;
    }
    
    player->velY += gravity * deltaTime;
    
    int newX = player->x + (int)(player->velX * deltaTime);
    int newY = player->y + (int)(player->velY * deltaTime);
    
    if (!CheckCollision(world, newX, player->y) && 
        !CheckCollision(world, newX + 16, player->y) &&
        !CheckCollision(world, newX, player->y + 31) &&
        !CheckCollision(world, newX + 16, player->y + 31)) {
        player->x = newX;
    } else {
        player->velX = 0;
    }
    
    if (!CheckCollision(world, player->x, newY) && 
        !CheckCollision(world, player->x + 16, newY) &&
        !CheckCollision(world, player->x, newY + 31) &&
        !CheckCollision(world, player->x + 16, newY + 31)) {
        player->y = newY;
        player->onGround = false;
    } else {
        if (player->velY > 0) {
            player->onGround = true;
        }
        player->velY = 0;
    }
    
    world->camera.target = (Vector2){ player->x, player->y };
}

void HandleBlockInteraction(World* world) {
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), world->camera);
    int blockX = (int)(mousePos.x / BLOCK_SIZE);
    int blockY = (int)(mousePos.y / BLOCK_SIZE);
    
    if (blockX >= 0 && blockX < WORLD_WIDTH && blockY >= 0 && blockY < WORLD_HEIGHT) {
        float distX = mousePos.x - (world->player.x + 8);
        float distY = mousePos.y - (world->player.y + 16);
        float distance = sqrt(distX * distX + distY * distY);
        
        if (distance < 100) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                world->blocks[blockY][blockX] = BLOCK_AIR;
            } else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                if (world->blocks[blockY][blockX] == BLOCK_AIR) {
                    world->blocks[blockY][blockX] = BLOCK_DIRT;
                }
            }
        }
    }
}

Color GetBlockColor(BlockType block) {
    switch (block) {
        case BLOCK_DIRT: return BROWN;
        case BLOCK_STONE: return GRAY;
        case BLOCK_GRASS: return GREEN;
        case BLOCK_WATER: return BLUE;
        case BLOCK_SAND: return YELLOW;
        default: return WHITE;
    }
}

void DrawWorld(World* world) {
    int startX = (int)((world->camera.target.x - SCREEN_WIDTH / 2) / BLOCK_SIZE) - 1;
    int endX = (int)((world->camera.target.x + SCREEN_WIDTH / 2) / BLOCK_SIZE) + 1;
    int startY = (int)((world->camera.target.y - SCREEN_HEIGHT / 2) / BLOCK_SIZE) - 1;
    int endY = (int)((world->camera.target.y + SCREEN_HEIGHT / 2) / BLOCK_SIZE) + 1;
    
    startX = fmax(0, startX);
    endX = fmin(WORLD_WIDTH - 1, endX);
    startY = fmax(0, startY);
    endY = fmin(WORLD_HEIGHT - 1, endY);
    
    for (int y = startY; y <= endY; y++) {
        for (int x = startX; x <= endX; x++) {
            if (world->blocks[y][x] != BLOCK_AIR) {
                Rectangle rect = { x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE };
                DrawRectangleRec(rect, GetBlockColor(world->blocks[y][x]));
                DrawRectangleLinesEx(rect, 1, BLACK);
            }
        }
    }
}

void DrawPlayer(World* world) {
    Rectangle playerRect = { world->player.x, world->player.y, 16, 32 };
    DrawRectangleRec(playerRect, RED);
    DrawRectangleLinesEx(playerRect, 2, MAROON);
}

void DrawUI(World* world) {
    DrawText("2D Voxel World", 10, 10, 20, WHITE);
    DrawText("WASD/Arrow Keys: Move", 10, 40, 16, WHITE);
    DrawText("Left Click: Destroy Block", 10, 60, 16, WHITE);
    DrawText("Right Click: Place Block", 10, 80, 16, WHITE);
    
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), world->camera);
    int blockX = (int)(mousePos.x / BLOCK_SIZE);
    int blockY = (int)(mousePos.y / BLOCK_SIZE);
    
    if (blockX >= 0 && blockX < WORLD_WIDTH && blockY >= 0 && blockY < WORLD_HEIGHT) {
        float distX = mousePos.x - (world->player.x + 8);
        float distY = mousePos.y - (world->player.y + 16);
        float distance = sqrt(distX * distX + distY * distY);
        
        if (distance < 100) {
            Rectangle highlightRect = { blockX * BLOCK_SIZE, blockY * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE };
            DrawRectangleLinesEx(highlightRect, 3, WHITE);
        }
    }
}

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "2D Voxel World - Terraria Style");
    
    World world;
    InitGame(&world);
    
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        UpdatePlayer(&world, deltaTime);
        HandleBlockInteraction(&world);
        
        BeginDrawing();
        ClearBackground(SKYBLUE);
        
        BeginMode2D(world.camera);
        DrawWorld(&world);
        DrawPlayer(&world);
        EndMode2D();
        
        DrawUI(&world);
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
