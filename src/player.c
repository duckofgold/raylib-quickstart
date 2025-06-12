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

bool IsInWater(World* world, int x, int y, int width, int height) {
    int blockX1 = x / BLOCK_SIZE;
    int blockY1 = y / BLOCK_SIZE;
    int blockX2 = (x + width - 1) / BLOCK_SIZE;
    int blockY2 = (y + height - 1) / BLOCK_SIZE;
    
    for (int bx = blockX1; bx <= blockX2; bx++) {
        for (int by = blockY1; by <= blockY2; by++) {
            if (bx >= 0 && bx < WORLD_WIDTH && by >= 0 && by < WORLD_HEIGHT) {
                if (world->blocks[by][bx] == BLOCK_WATER) {
                    return true;
                }
            }
        }
    }
    return false;
}

void InitPlayer(Player* player) {
    player->x = WORLD_WIDTH * BLOCK_SIZE / 2;
    player->y = 20 * BLOCK_SIZE;
    player->velX = 0;
    player->velY = 0;
    player->onGround = false;
    player->inWater = false;
    player->health = 100;
    player->selectedSlot = 0;
    player->lastJumpTime = 0;
    player->lastClickTime = 0;
    player->inventoryOpen = false;
    player->isDragging = false;
    player->draggedSlot = -1;
    player->dragFromExtended = false;
    player->isBreaking = false;
    player->breakStartTime = 0;
    player->breakProgress = 0;
    player->breakingBlockX = -1;
    player->breakingBlockY = -1;
    player->craftingOpen = false;
    
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        player->inventory[i].type = BLOCK_AIR;
        player->inventory[i].tool = TOOL_NONE;
        player->inventory[i].count = 0;
        player->inventory[i].durability = 0;
    }
    
    for (int i = 0; i < EXTENDED_INVENTORY_SIZE; i++) {
        player->extendedInventory[i].type = BLOCK_AIR;
        player->extendedInventory[i].tool = TOOL_NONE;
        player->extendedInventory[i].count = 0;
        player->extendedInventory[i].durability = 0;
    }
    
    player->inventory[0].type = BLOCK_DIRT;
    player->inventory[0].count = 64;
    player->inventory[1].type = BLOCK_STONE;
    player->inventory[1].count = 32;
    player->inventory[2].type = BLOCK_WOOD;
    player->inventory[2].count = 16;
    player->inventory[3].type = BLOCK_SAND;
    player->inventory[3].count = 24;
    
    player->extendedInventory[0].type = BLOCK_COAL_ORE;
    player->extendedInventory[0].count = 5;
    player->extendedInventory[1].type = BLOCK_IRON_ORE;
    player->extendedInventory[1].count = 3;
    player->extendedInventory[2].type = BLOCK_GOLD_ORE;
    player->extendedInventory[2].count = 2;
    
    player->inventory[4].type = BLOCK_AIR;
    player->inventory[4].tool = TOOL_WOODEN_PICKAXE;
    player->inventory[4].count = 1;
    player->inventory[4].durability = GetToolDurability(TOOL_WOODEN_PICKAXE);
}

void UpdatePlayer(World* world, float deltaTime) {
    Player* player = &world->player;
    
    player->inWater = IsInWater(world, player->x, player->y, 16, 32);
    
    float speed = player->inWater ? 150.0f : 250.0f;
    float jumpForce = player->inWater ? 200.0f : 450.0f;
    float gravity = player->inWater ? 200.0f : 900.0f;
    float currentTime = GetTime();
    
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        player->velX = -speed;
    } else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        player->velX = speed;
    } else {
        float friction = player->inWater ? 0.7f : 0.85f;
        player->velX *= friction;
    }
    
    if (player->inWater) {
        if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
            player->velY = -jumpForce;
        } else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
            player->velY = jumpForce;
        } else {
            player->velY *= 0.8f;
        }
    } else {
        if ((IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) && 
            player->onGround && (currentTime - player->lastJumpTime) > 0.2f) {
            player->velY = -jumpForce;
            player->onGround = false;
            player->lastJumpTime = currentTime;
        }
    }
    
    if (!player->inWater) {
        player->velY += gravity * deltaTime;
        if (player->velY > 600.0f) player->velY = 600.0f;
    } else {
        player->velY += gravity * deltaTime * 0.3f;
        if (player->velY > 200.0f) player->velY = 200.0f;
        if (player->velY < -200.0f) player->velY = -200.0f;
    }
    
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
        if (!player->inWater) {
            player->onGround = false;
        }
    } else {
        if (player->velY > 0 && !player->inWater) {
            player->onGround = true;
        }
        player->velY = 0;
    }
    
    world->camera.target = (Vector2){ player->x + 8, player->y + 16 };
}

void AddToInventory(Player* player, BlockType blockType) {
    // Try to add to existing stack in hotbar first
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (player->inventory[i].type == blockType && player->inventory[i].tool == TOOL_NONE && 
            player->inventory[i].count > 0 && player->inventory[i].count < 64) {
            player->inventory[i].count++;
            return;
        }
    }
    
    // Try to find empty slot in hotbar
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (player->inventory[i].type == BLOCK_AIR || player->inventory[i].count == 0) {
            // Clear the slot completely to ensure no old tool data remains
            player->inventory[i].type = blockType;
            player->inventory[i].tool = TOOL_NONE;
            player->inventory[i].count = 1;
            player->inventory[i].durability = 0;
            return;
        }
    }
    
    // Try to add to existing stack in extended inventory
    for (int i = 0; i < EXTENDED_INVENTORY_SIZE; i++) {
        if (player->extendedInventory[i].type == blockType && player->extendedInventory[i].tool == TOOL_NONE && 
            player->extendedInventory[i].count > 0 && player->extendedInventory[i].count < 64) {
            player->extendedInventory[i].count++;
            return;
        }
    }
    
    // Try to find empty slot in extended inventory
    for (int i = 0; i < EXTENDED_INVENTORY_SIZE; i++) {
        if (player->extendedInventory[i].type == BLOCK_AIR || player->extendedInventory[i].count == 0) {
            // Clear the slot completely to ensure no old tool data remains
            player->extendedInventory[i].type = blockType;
            player->extendedInventory[i].tool = TOOL_NONE;
            player->extendedInventory[i].count = 1;
            player->extendedInventory[i].durability = 0;
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
    
    if (IsKeyPressed(KEY_E)) {
        player->inventoryOpen = !player->inventoryOpen;
    }
    
    if (!player->inventoryOpen) {
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
}

void SwapInventorySlots(InventorySlot* slot1, InventorySlot* slot2) {
    InventorySlot temp = *slot1;
    *slot1 = *slot2;
    *slot2 = temp;
}

void HandleExtendedInventory(World* world) {
    Player* player = &world->player;
    
    if (!player->inventoryOpen) return;
    
    Vector2 mousePos = GetMousePosition();
    
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        int slotSize = 50;
        int startX = SCREEN_WIDTH / 2 - 225;
        int startY = SCREEN_HEIGHT / 2 - 135;
        
        for (int i = 0; i < INVENTORY_SIZE; i++) {
            Rectangle slotRect = {startX + (i % 9) * slotSize, startY + 180, slotSize, slotSize};
            if (CheckCollisionPointRec(mousePos, slotRect)) {
                if (player->isDragging) {
                    if (player->dragFromExtended) {
                        SwapInventorySlots(&player->extendedInventory[player->draggedSlot], &player->inventory[i]);
                    } else {
                        SwapInventorySlots(&player->inventory[player->draggedSlot], &player->inventory[i]);
                    }
                    player->isDragging = false;
                    player->draggedSlot = -1;
                } else {
                    player->isDragging = true;
                    player->draggedSlot = i;
                    player->dragFromExtended = false;
                }
                return;
            }
        }
        
        for (int i = 0; i < EXTENDED_INVENTORY_SIZE; i++) {
            int row = i / 9;
            int col = i % 9;
            Rectangle slotRect = {startX + col * slotSize, startY + row * slotSize, slotSize, slotSize};
            if (CheckCollisionPointRec(mousePos, slotRect)) {
                if (player->isDragging) {
                    if (player->dragFromExtended) {
                        SwapInventorySlots(&player->extendedInventory[player->draggedSlot], &player->extendedInventory[i]);
                    } else {
                        SwapInventorySlots(&player->inventory[player->draggedSlot], &player->extendedInventory[i]);
                    }
                    player->isDragging = false;
                    player->draggedSlot = -1;
                } else {
                    player->isDragging = true;
                    player->draggedSlot = i;
                    player->dragFromExtended = true;
                }
                return;
            }
        }
        
        player->isDragging = false;
        player->draggedSlot = -1;
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
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                if (world->blocks[blockY][blockX] != BLOCK_AIR) {
                    ToolType currentTool = player->inventory[player->selectedSlot].tool;
                    float breakTime = GetBreakTime(world->blocks[blockY][blockX], currentTool);
                    
                    if (!player->isBreaking || player->breakingBlockX != blockX || player->breakingBlockY != blockY) {
                        player->isBreaking = true;
                        player->breakStartTime = currentTime;
                        player->breakProgress = 0;
                        player->breakingBlockX = blockX;
                        player->breakingBlockY = blockY;
                    }
                    
                    player->breakProgress = (currentTime - player->breakStartTime) / breakTime;
                    
                    if (player->breakProgress >= 1.0f) {
                        AddToInventory(player, world->blocks[blockY][blockX]);
                        world->blocks[blockY][blockX] = BLOCK_AIR;
                        
                        if (currentTool != TOOL_NONE) {
                            player->inventory[player->selectedSlot].durability--;
                            if (player->inventory[player->selectedSlot].durability <= 0) {
                                player->inventory[player->selectedSlot].tool = TOOL_NONE;
                                player->inventory[player->selectedSlot].count = 0;
                                player->inventory[player->selectedSlot].type = BLOCK_AIR;
                            }
                        }
                        
                        player->isBreaking = false;
                        player->breakProgress = 0;
                    }
                }
            } else {
                player->isBreaking = false;
                player->breakProgress = 0;
            }
            
            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                if (world->blocks[blockY][blockX] == BLOCK_AIR) {
                    InventorySlot* selectedSlot = &player->inventory[player->selectedSlot];
                    if (selectedSlot->type != BLOCK_AIR && selectedSlot->tool == TOOL_NONE && selectedSlot->count > 0) {
                        world->blocks[blockY][blockX] = selectedSlot->type;
                        selectedSlot->count--;
                        if (selectedSlot->count == 0) {
                            selectedSlot->type = BLOCK_AIR;
                        }
                    }
                }
            }
        } else {
            player->isBreaking = false;
            player->breakProgress = 0;
        }
    } else {
        player->isBreaking = false;
        player->breakProgress = 0;
    }
}