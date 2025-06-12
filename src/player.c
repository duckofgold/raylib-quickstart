#include "game.h"
#include <math.h>

bool CheckCollision(World* world, int x, int y) {
    int blockX = x / BLOCK_SIZE;
    int blockY = y / BLOCK_SIZE;
    
    if (blockX < 0 || blockX >= WORLD_WIDTH || blockY < 0 || blockY >= WORLD_HEIGHT) {
        return true;
    }
    
    return IsBlockSolid(world->blocks[blockY][blockX]);
}

void InitPlayer(Player* player) {
    player->x = WORLD_WIDTH * BLOCK_SIZE / 2;
    player->y = 20 * BLOCK_SIZE;
    player->velX = 0;
    player->velY = 0;
    player->onGround = false;
    player->health = 100;
    player->selectedSlot = 0;
    player->lastJumpTime = 0;
    player->lastClickTime = 0;
    
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        player->inventory[i].type = BLOCK_AIR;
        player->inventory[i].count = 0;
    }
    
    player->inventory[0].type = BLOCK_DIRT;
    player->inventory[0].count = 64;
    player->inventory[1].type = BLOCK_STONE;
    player->inventory[1].count = 32;
    player->inventory[2].type = BLOCK_WOOD;
    player->inventory[2].count = 16;
    player->inventory[3].type = BLOCK_SAND;
    player->inventory[3].count = 24;
}

void UpdatePlayer(World* world, float deltaTime) {
    Player* player = &world->player;
    
    float speed = 250.0f;
    float jumpForce = 450.0f;
    float gravity = 900.0f;
    float currentTime = GetTime();
    
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        player->velX = -speed;
    } else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        player->velX = speed;
    } else {
        player->velX *= 0.85f;
    }
    
    if ((IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) && 
        player->onGround && (currentTime - player->lastJumpTime) > 0.2f) {
        player->velY = -jumpForce;
        player->onGround = false;
        player->lastJumpTime = currentTime;
    }
    
    player->velY += gravity * deltaTime;
    
    if (player->velY > 600.0f) player->velY = 600.0f;
    
    int newX = player->x + (int)(player->velX * deltaTime);
    int newY = player->y + (int)(player->velY * deltaTime);
    
    if (!CheckCollision(world, newX, player->y) && 
        !CheckCollision(world, newX + 15, player->y) &&
        !CheckCollision(world, newX, player->y + 31) &&
        !CheckCollision(world, newX + 15, player->y + 31)) {
        player->x = newX;
    } else {
        player->velX = 0;
    }
    
    if (!CheckCollision(world, player->x, newY) && 
        !CheckCollision(world, player->x + 15, newY) &&
        !CheckCollision(world, player->x, newY + 31) &&
        !CheckCollision(world, player->x + 15, newY + 31)) {
        player->y = newY;
        player->onGround = false;
    } else {
        if (player->velY > 0) {
            player->onGround = true;
        }
        player->velY = 0;
    }
    
    world->camera.target = (Vector2){ player->x + 8, player->y + 16 };
}

void AddToInventory(Player* player, BlockType blockType) {
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (player->inventory[i].type == blockType && player->inventory[i].count < 64) {
            player->inventory[i].count++;
            return;
        }
    }
    
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (player->inventory[i].type == BLOCK_AIR || player->inventory[i].count == 0) {
            player->inventory[i].type = blockType;
            player->inventory[i].count = 1;
            return;
        }
    }
}

bool RemoveFromInventory(Player* player, BlockType blockType) {
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (player->inventory[i].type == blockType && player->inventory[i].count > 0) {
            player->inventory[i].count--;
            if (player->inventory[i].count == 0) {
                player->inventory[i].type = BLOCK_AIR;
            }
            return true;
        }
    }
    return false;
}

void HandleInventoryInput(World* world) {
    Player* player = &world->player;
    
    if (IsKeyPressed(KEY_ONE)) player->selectedSlot = 0;
    if (IsKeyPressed(KEY_TWO)) player->selectedSlot = 1;
    if (IsKeyPressed(KEY_THREE)) player->selectedSlot = 2;
    if (IsKeyPressed(KEY_FOUR)) player->selectedSlot = 3;
    if (IsKeyPressed(KEY_FIVE)) player->selectedSlot = 4;
    if (IsKeyPressed(KEY_SIX)) player->selectedSlot = 5;
    if (IsKeyPressed(KEY_SEVEN)) player->selectedSlot = 6;
    if (IsKeyPressed(KEY_EIGHT)) player->selectedSlot = 7;
    if (IsKeyPressed(KEY_NINE)) player->selectedSlot = 8;
    
    float mouseWheel = GetMouseWheelMove();
    if (mouseWheel != 0) {
        player->selectedSlot -= (int)mouseWheel;
        if (player->selectedSlot < 0) player->selectedSlot = INVENTORY_SIZE - 1;
        if (player->selectedSlot >= INVENTORY_SIZE) player->selectedSlot = 0;
    }
}

void HandleBlockInteraction(World* world, float deltaTime) {
    Player* player = &world->player;
    float currentTime = GetTime();
    
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), world->camera);
    int blockX = (int)(mousePos.x / BLOCK_SIZE);
    int blockY = (int)(mousePos.y / BLOCK_SIZE);
    
    if (blockX >= 0 && blockX < WORLD_WIDTH && blockY >= 0 && blockY < WORLD_HEIGHT) {
        float distX = mousePos.x - (player->x + 8);
        float distY = mousePos.y - (player->y + 16);
        float distance = sqrt(distX * distX + distY * distY);
        
        if (distance < MAX_REACH_DISTANCE) {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && (currentTime - player->lastClickTime) > 0.15f) {
                if (world->blocks[blockY][blockX] != BLOCK_AIR) {
                    AddToInventory(player, world->blocks[blockY][blockX]);
                    world->blocks[blockY][blockX] = BLOCK_AIR;
                    player->lastClickTime = currentTime;
                }
            } else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON) && (currentTime - player->lastClickTime) > 0.15f) {
                if (world->blocks[blockY][blockX] == BLOCK_AIR) {
                    BlockType selectedBlock = player->inventory[player->selectedSlot].type;
                    if (selectedBlock != BLOCK_AIR && RemoveFromInventory(player, selectedBlock)) {
                        world->blocks[blockY][blockX] = selectedBlock;
                        player->lastClickTime = currentTime;
                    }
                }
            }
        }
    }
}