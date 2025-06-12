#include "game.h"
#include <math.h>
#include <stdio.h>

bool IsBlockSolid(BlockType block) {
    return block != BLOCK_AIR && block != BLOCK_WATER;
}

Color GetBlockColor(BlockType block) {
    switch (block) {
        case BLOCK_DIRT: return BROWN;
        case BLOCK_STONE: return GRAY;
        case BLOCK_GRASS: return GREEN;
        case BLOCK_WATER: return (Color){100, 150, 255, 180};
        case BLOCK_SAND: return YELLOW;
        case BLOCK_WOOD: return (Color){139, 69, 19, 255};
        case BLOCK_LEAVES: return (Color){50, 170, 50, 255};
        default: return WHITE;
    }
}

const char* GetBlockName(BlockType block) {
    switch (block) {
        case BLOCK_DIRT: return "Dirt";
        case BLOCK_STONE: return "Stone";
        case BLOCK_GRASS: return "Grass";
        case BLOCK_WATER: return "Water";
        case BLOCK_SAND: return "Sand";
        case BLOCK_WOOD: return "Wood";
        case BLOCK_LEAVES: return "Leaves";
        default: return "Air";
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
                Color blockColor = GetBlockColor(world->blocks[y][x]);
                
                if (world->blocks[y][x] == BLOCK_WATER) {
                    DrawRectangleRec(rect, blockColor);
                } else if (world->blocks[y][x] == BLOCK_LEAVES && y < WORLD_HEIGHT - 1 && 
                          (world->blocks[y + 1][x] == BLOCK_GRASS || world->blocks[y + 1][x] == BLOCK_DIRT)) {
                    DrawRectangle(rect.x + 4, rect.y + 8, 8, 16, (Color){60, 180, 60, 255});
                    DrawRectangle(rect.x + 12, rect.y + 4, 6, 20, (Color){40, 160, 40, 255});
                    DrawRectangle(rect.x + 20, rect.y + 12, 8, 12, (Color){80, 200, 80, 255});
                } else {
                    DrawRectangleRec(rect, blockColor);
                    DrawRectangleLinesEx(rect, 1, BLACK);
                }
            }
        }
    }
}

void DrawPlayer(World* world) {
    Rectangle playerRect = { world->player.x, world->player.y, 16, 32 };
    Color playerColor = world->player.inWater ? BLUE : RED;
    Color outlineColor = world->player.inWater ? DARKBLUE : MAROON;
    
    DrawRectangleRec(playerRect, playerColor);
    DrawRectangleLinesEx(playerRect, 2, outlineColor);
    
    DrawCircle(world->player.x + 8, world->player.y + 8, 3, WHITE);
    
    if (world->player.inWater) {
        for (int i = 0; i < 3; i++) {
            int bubbleX = world->player.x + GetRandomValue(-5, 20);
            int bubbleY = world->player.y + GetRandomValue(0, 32);
            int animOffset = ((int)(GetTime() * 20) + i * 10) % 40;
            DrawCircle(bubbleX, bubbleY - animOffset, 2, (Color){200, 230, 255, 150});
        }
    }
}

void DrawInventory(World* world) {
    Player* player = &world->player;
    int slotSize = 60;
    int startX = SCREEN_WIDTH / 2 - (INVENTORY_SIZE * slotSize) / 2;
    int startY = SCREEN_HEIGHT - slotSize - 10;
    
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        Rectangle slotRect = { startX + i * slotSize, startY, slotSize, slotSize };
        
        Color slotColor = (i == player->selectedSlot) ? YELLOW : LIGHTGRAY;
        DrawRectangleRec(slotRect, slotColor);
        DrawRectangleLinesEx(slotRect, 2, BLACK);
        
        if (player->inventory[i].type != BLOCK_AIR && player->inventory[i].count > 0) {
            Rectangle blockRect = { startX + i * slotSize + 5, startY + 5, slotSize - 10, slotSize - 30 };
            DrawRectangleRec(blockRect, GetBlockColor(player->inventory[i].type));
            DrawRectangleLinesEx(blockRect, 1, BLACK);
            
            char countText[8];
            sprintf(countText, "%d", player->inventory[i].count);
            DrawText(countText, startX + i * slotSize + 5, startY + slotSize - 20, 16, WHITE);
        }
        
        char slotNum[2];
        sprintf(slotNum, "%d", i + 1);
        DrawText(slotNum, startX + i * slotSize + 2, startY + 2, 12, BLACK);
    }
}

void DrawUI(World* world) {
    DrawText("2D Voxel World", 10, 10, 20, WHITE);
    DrawText("WASD/Arrow Keys: Move", 10, 40, 16, WHITE);
    DrawText("Left Click: Destroy Block", 10, 60, 16, WHITE);
    DrawText("Right Click: Place Block", 10, 80, 16, WHITE);
    DrawText("1-9 Keys: Select Inventory", 10, 100, 16, WHITE);
    DrawText("Mouse Wheel: Scroll Inventory", 10, 120, 16, WHITE);
    if (world->player.inWater) {
        DrawText("Swimming: S to dive, W/Space to swim up", 10, 140, 16, BLUE);
    }
    
    char animalCountText[64];
    sprintf(animalCountText, "Animals: %d/%d", world->animalCount, MAX_ANIMALS);
    DrawText(animalCountText, SCREEN_WIDTH - 200, 10, 16, WHITE);
    
    Player* player = &world->player;
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), world->camera);
    int blockX = (int)(mousePos.x / BLOCK_SIZE);
    int blockY = (int)(mousePos.y / BLOCK_SIZE);
    
    for (int i = 0; i < MAX_ANIMALS; i++) {
        if (world->animals[i].alive) {
            Animal* animal = &world->animals[i];
            float distX = mousePos.x - (animal->x + 6);
            float distY = mousePos.y - (animal->y + 6);
            float distance = sqrt(distX * distX + distY * distY);
            
            if (distance < 20) {
                Vector2 worldPos = {animal->x + 6, animal->y - 10};
                Vector2 screenPos = GetWorldToScreen2D(worldPos, world->camera);
                const char* animalName = GetAnimalName(animal->type);
                DrawText(animalName, screenPos.x - MeasureText(animalName, 12)/2, screenPos.y, 12, YELLOW);
                
                Rectangle highlightRect = {animal->x - 2, animal->y - 2, 16, 16};
                DrawRectangleLinesEx(highlightRect, 2, YELLOW);
            }
        }
    }
    
    if (blockX >= 0 && blockX < WORLD_WIDTH && blockY >= 0 && blockY < WORLD_HEIGHT) {
        float distX = mousePos.x - (player->x + 8);
        float distY = mousePos.y - (player->y + 16);
        float distance = sqrt(distX * distX + distY * distY);
        
        if (distance < MAX_REACH_DISTANCE) {
            Rectangle highlightRect = { blockX * BLOCK_SIZE, blockY * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE };
            DrawRectangleLinesEx(highlightRect, 3, WHITE);
            
            if (world->blocks[blockY][blockX] != BLOCK_AIR) {
                const char* blockName;
                if (world->blocks[blockY][blockX] == BLOCK_LEAVES && blockY < WORLD_HEIGHT - 1 && 
                   (world->blocks[blockY + 1][blockX] == BLOCK_GRASS || world->blocks[blockY + 1][blockX] == BLOCK_DIRT)) {
                    blockName = "Grass Patch";
                } else {
                    blockName = GetBlockName(world->blocks[blockY][blockX]);
                }
                Vector2 worldPos = {highlightRect.x + BLOCK_SIZE/2, highlightRect.y - 10};
                Vector2 screenPos = GetWorldToScreen2D(worldPos, world->camera);
                DrawText(blockName, screenPos.x - MeasureText(blockName, 12)/2, screenPos.y, 12, WHITE);
            }
        }
    }
    
    if (player->inventory[player->selectedSlot].type != BLOCK_AIR) {
        const char* selectedBlockName = GetBlockName(player->inventory[player->selectedSlot].type);
        char selectedText[64];
        sprintf(selectedText, "Selected: %s (%d)", selectedBlockName, player->inventory[player->selectedSlot].count);
        DrawText(selectedText, SCREEN_WIDTH - 300, 10, 16, WHITE);
    }
    
    DrawInventory(world);
}