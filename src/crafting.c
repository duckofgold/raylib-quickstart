#include "game.h"
#include <math.h>

float GetBlockHardness(BlockType block) {
    switch (block) {
        case BLOCK_DIRT: return 0.5f;
        case BLOCK_GRASS: return 0.6f;
        case BLOCK_SAND: return 0.5f;
        case BLOCK_WOOD: return 2.0f;
        case BLOCK_LEAVES: return 0.2f;
        case BLOCK_STONE: return 1.5f;
        case BLOCK_COAL_ORE: return 3.0f;
        case BLOCK_IRON_ORE: return 3.0f;
        case BLOCK_GOLD_ORE: return 3.0f;
        case BLOCK_DIAMOND_ORE: return 15.0f;
        case BLOCK_EMERALD_ORE: return 3.0f;
        default: return 1.0f;
    }
}

float GetToolSpeed(ToolType tool) {
    switch (tool) {
        case TOOL_NONE: return 1.0f;
        case TOOL_WOODEN_PICKAXE: return 2.0f;
        case TOOL_STONE_PICKAXE: return 4.0f;
        case TOOL_IRON_PICKAXE: return 6.0f;
        case TOOL_GOLD_PICKAXE: return 12.0f;
        case TOOL_DIAMOND_PICKAXE: return 8.0f;
        default: return 1.0f;
    }
}

bool CanToolBreak(ToolType tool, BlockType block) {
    switch (block) {
        case BLOCK_STONE:
        case BLOCK_COAL_ORE:
        case BLOCK_IRON_ORE:
            return tool >= TOOL_WOODEN_PICKAXE;
        case BLOCK_GOLD_ORE:
        case BLOCK_EMERALD_ORE:
            return tool >= TOOL_IRON_PICKAXE;
        case BLOCK_DIAMOND_ORE:
            return tool >= TOOL_IRON_PICKAXE;
        default:
            return true;
    }
}

float GetBreakTime(BlockType block, ToolType tool) {
    float hardness = GetBlockHardness(block);
    float speed = GetToolSpeed(tool);
    
    if (!CanToolBreak(tool, block)) {
        return hardness * 5.0f;
    }
    
    return hardness / speed;
}

const char* GetToolName(ToolType tool) {
    switch (tool) {
        case TOOL_WOODEN_PICKAXE: return "Wooden Pickaxe";
        case TOOL_STONE_PICKAXE: return "Stone Pickaxe";
        case TOOL_IRON_PICKAXE: return "Iron Pickaxe";
        case TOOL_GOLD_PICKAXE: return "Gold Pickaxe";
        case TOOL_DIAMOND_PICKAXE: return "Diamond Pickaxe";
        default: return "No Tool";
    }
}

int GetToolDurability(ToolType tool) {
    switch (tool) {
        case TOOL_WOODEN_PICKAXE: return 60;
        case TOOL_STONE_PICKAXE: return 132;
        case TOOL_IRON_PICKAXE: return 251;
        case TOOL_GOLD_PICKAXE: return 33;
        case TOOL_DIAMOND_PICKAXE: return 1562;
        default: return 0;
    }
}

bool CanCraftTool(Player* player, ToolType tool) {
    int stickCount = 0;
    int materialCount = 0;
    BlockType requiredMaterial = BLOCK_AIR;
    
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (player->inventory[i].type == BLOCK_WOOD) {
            stickCount += player->inventory[i].count;
        }
    }
    
    for (int i = 0; i < EXTENDED_INVENTORY_SIZE; i++) {
        if (player->extendedInventory[i].type == BLOCK_WOOD) {
            stickCount += player->extendedInventory[i].count;
        }
    }
    
    switch (tool) {
        case TOOL_WOODEN_PICKAXE:
            requiredMaterial = BLOCK_WOOD;
            break;
        case TOOL_STONE_PICKAXE:
            requiredMaterial = BLOCK_STONE;
            break;
        case TOOL_IRON_PICKAXE:
            requiredMaterial = BLOCK_IRON_ORE;
            break;
        case TOOL_GOLD_PICKAXE:
            requiredMaterial = BLOCK_GOLD_ORE;
            break;
        case TOOL_DIAMOND_PICKAXE:
            requiredMaterial = BLOCK_DIAMOND_ORE;
            break;
        default:
            return false;
    }
    
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (player->inventory[i].type == requiredMaterial) {
            materialCount += player->inventory[i].count;
        }
    }
    
    for (int i = 0; i < EXTENDED_INVENTORY_SIZE; i++) {
        if (player->extendedInventory[i].type == requiredMaterial) {
            materialCount += player->extendedInventory[i].count;
        }
    }
    
    return (stickCount >= 2 && materialCount >= 3);
}

void ConsumeCraftingMaterials(Player* player, ToolType tool) {
    int sticksNeeded = 2;
    int materialsNeeded = 3;
    BlockType requiredMaterial = BLOCK_AIR;
    
    switch (tool) {
        case TOOL_WOODEN_PICKAXE:
            requiredMaterial = BLOCK_WOOD;
            sticksNeeded = 0;
            materialsNeeded = 3;
            break;
        case TOOL_STONE_PICKAXE:
            requiredMaterial = BLOCK_STONE;
            break;
        case TOOL_IRON_PICKAXE:
            requiredMaterial = BLOCK_IRON_ORE;
            break;
        case TOOL_GOLD_PICKAXE:
            requiredMaterial = BLOCK_GOLD_ORE;
            break;
        case TOOL_DIAMOND_PICKAXE:
            requiredMaterial = BLOCK_DIAMOND_ORE;
            break;
        default:
            return;
    }
    
    for (int i = 0; i < INVENTORY_SIZE && sticksNeeded > 0; i++) {
        if (player->inventory[i].type == BLOCK_WOOD) {
            int taken = fmin(sticksNeeded, player->inventory[i].count);
            player->inventory[i].count -= taken;
            sticksNeeded -= taken;
            if (player->inventory[i].count == 0) {
                player->inventory[i].type = BLOCK_AIR;
            }
        }
    }
    
    for (int i = 0; i < EXTENDED_INVENTORY_SIZE && sticksNeeded > 0; i++) {
        if (player->extendedInventory[i].type == BLOCK_WOOD) {
            int taken = fmin(sticksNeeded, player->extendedInventory[i].count);
            player->extendedInventory[i].count -= taken;
            sticksNeeded -= taken;
            if (player->extendedInventory[i].count == 0) {
                player->extendedInventory[i].type = BLOCK_AIR;
            }
        }
    }
    
    for (int i = 0; i < INVENTORY_SIZE && materialsNeeded > 0; i++) {
        if (player->inventory[i].type == requiredMaterial) {
            int taken = fmin(materialsNeeded, player->inventory[i].count);
            player->inventory[i].count -= taken;
            materialsNeeded -= taken;
            if (player->inventory[i].count == 0) {
                player->inventory[i].type = BLOCK_AIR;
            }
        }
    }
    
    for (int i = 0; i < EXTENDED_INVENTORY_SIZE && materialsNeeded > 0; i++) {
        if (player->extendedInventory[i].type == requiredMaterial) {
            int taken = fmin(materialsNeeded, player->extendedInventory[i].count);
            player->extendedInventory[i].count -= taken;
            materialsNeeded -= taken;
            if (player->extendedInventory[i].count == 0) {
                player->extendedInventory[i].type = BLOCK_AIR;
            }
        }
    }
}

void AddToolToInventory(Player* player, ToolType tool) {
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (player->inventory[i].type == BLOCK_AIR || player->inventory[i].count == 0) {
            player->inventory[i].type = BLOCK_AIR;
            player->inventory[i].tool = tool;
            player->inventory[i].count = 1;
            player->inventory[i].durability = GetToolDurability(tool);
            return;
        }
    }
    
    for (int i = 0; i < EXTENDED_INVENTORY_SIZE; i++) {
        if (player->extendedInventory[i].type == BLOCK_AIR || player->extendedInventory[i].count == 0) {
            player->extendedInventory[i].type = BLOCK_AIR;
            player->extendedInventory[i].tool = tool;
            player->extendedInventory[i].count = 1;
            player->extendedInventory[i].durability = GetToolDurability(tool);
            return;
        }
    }
}

void HandleCrafting(World* world) {
    Player* player = &world->player;
    
    if (IsKeyPressed(KEY_C)) {
        player->craftingOpen = !player->craftingOpen;
    }
    
    if (!player->craftingOpen) return;
    
    Vector2 mousePos = GetMousePosition();
    int startX = SCREEN_WIDTH / 2 - 200;
    int startY = SCREEN_HEIGHT / 2 - 150;
    
    for (int i = 1; i < TOOL_COUNT; i++) {
        Rectangle toolRect = {startX, startY + (i - 1) * 60, 400, 50};
        if (CheckCollisionPointRec(mousePos, toolRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CanCraftTool(player, (ToolType)i)) {
                ConsumeCraftingMaterials(player, (ToolType)i);
                AddToolToInventory(player, (ToolType)i);
            }
        }
    }
}