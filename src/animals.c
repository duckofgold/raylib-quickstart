#include "game.h"
#include <math.h>
#include <stdlib.h>

bool CheckAnimalCollision(World* world, float x, float y, int width, int height) {
    int blockX1 = (int)x / BLOCK_SIZE;
    int blockY1 = (int)y / BLOCK_SIZE;
    int blockX2 = (int)(x + width - 1) / BLOCK_SIZE;
    int blockY2 = (int)(y + height - 1) / BLOCK_SIZE;
    
    for (int bx = blockX1; bx <= blockX2; bx++) {
        for (int by = blockY1; by <= blockY2; by++) {
            if (bx < 0 || bx >= WORLD_WIDTH || by < 0 || by >= WORLD_HEIGHT) {
                return true;
            }
            if (IsBlockSolid(world->blocks[by][bx])) {
                return true;
            }
        }
    }
    return false;
}

bool IsAnimalInWater(World* world, float x, float y, int width, int height) {
    int blockX1 = (int)x / BLOCK_SIZE;
    int blockY1 = (int)y / BLOCK_SIZE;
    int blockX2 = (int)(x + width - 1) / BLOCK_SIZE;
    int blockY2 = (int)(y + height - 1) / BLOCK_SIZE;
    
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

int FindGroundHeight(World* world, int x) {
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        if (world->blocks[y][x] != BLOCK_AIR && world->blocks[y][x] != BLOCK_WATER) {
            return y * BLOCK_SIZE - 16;
        }
    }
    return (WORLD_HEIGHT - 1) * BLOCK_SIZE;
}

void InitAnimals(World* world) {
    world->animalCount = 0;
    
    for (int i = 0; i < MAX_ANIMALS; i++) {
        world->animals[i].alive = false;
    }
    
    for (int i = 0; i < 12; i++) {
        AnimalType type = (AnimalType)GetRandomValue(0, ANIMAL_COUNT - 1);
        float x = GetRandomValue(50, (WORLD_WIDTH - 50) * BLOCK_SIZE);
        float y = FindGroundHeight(world, x / BLOCK_SIZE);
        
        if (type == ANIMAL_FISH) {
            for (int attempt = 0; attempt < 10; attempt++) {
                x = GetRandomValue(50, (WORLD_WIDTH - 50) * BLOCK_SIZE);
                y = GetRandomValue(30, 80) * BLOCK_SIZE;
                if (IsAnimalInWater(world, x, y, 12, 8)) {
                    break;
                }
            }
        }
        
        SpawnAnimal(world, type, x, y);
    }
}

void SpawnAnimal(World* world, AnimalType type, float x, float y) {
    if (world->animalCount >= MAX_ANIMALS) return;
    
    for (int i = 0; i < MAX_ANIMALS; i++) {
        if (!world->animals[i].alive) {
            Animal* animal = &world->animals[i];
            animal->type = type;
            animal->x = x;
            animal->y = y;
            animal->velX = 0;
            animal->velY = 0;
            animal->state = AI_WANDER;
            animal->stateTimer = GetRandomValue(2, 8);
            animal->direction = GetRandomValue(0, 1) ? 1.0f : -1.0f;
            animal->onGround = false;
            animal->inWater = false;
            animal->alive = true;
            animal->animTime = 0;
            world->animalCount++;
            break;
        }
    }
}

void UpdateAnimalAI(World* world, Animal* animal, float deltaTime) {
    float playerDist = sqrt((animal->x - world->player.x) * (animal->x - world->player.x) + 
                           (animal->y - world->player.y) * (animal->y - world->player.y));
    
    animal->stateTimer -= deltaTime;
    animal->animTime += deltaTime;
    
    switch (animal->state) {
        case AI_WANDER:
            if (playerDist < 80 && animal->type != ANIMAL_FISH) {
                animal->state = AI_FLEE;
                animal->stateTimer = 3.0f;
                animal->direction = (animal->x > world->player.x) ? 1.0f : -1.0f;
            } else if (animal->stateTimer <= 0) {
                animal->direction = GetRandomValue(0, 1) ? 1.0f : -1.0f;
                animal->stateTimer = GetRandomValue(2, 6);
            }
            break;
            
        case AI_FLEE:
            if (playerDist > 120) {
                animal->state = AI_WANDER;
                animal->stateTimer = GetRandomValue(2, 8);
            } else if (animal->stateTimer <= 0) {
                animal->state = AI_WANDER;
                animal->stateTimer = GetRandomValue(1, 3);
            }
            break;
            
        case AI_SWIM:
            if (!animal->inWater) {
                animal->state = AI_WANDER;
                animal->stateTimer = GetRandomValue(2, 8);
            } else if (animal->stateTimer <= 0) {
                animal->direction = GetRandomValue(0, 1) ? 1.0f : -1.0f;
                animal->stateTimer = GetRandomValue(2, 5);
            }
            break;
    }
}

void UpdateAnimalPhysics(World* world, Animal* animal, float deltaTime) {
    float speed = 50.0f;
    float jumpForce = 200.0f;
    float gravity = 400.0f;
    
    switch (animal->type) {
        case ANIMAL_RABBIT:
            speed = 80.0f;
            jumpForce = 300.0f;
            break;
        case ANIMAL_BIRD:
            speed = 60.0f;
            gravity = 100.0f;
            jumpForce = 150.0f;
            break;
        case ANIMAL_FISH:
            speed = 40.0f;
            gravity = 0.0f;
            break;
        case ANIMAL_PIG:
            speed = 30.0f;
            jumpForce = 180.0f;
            break;
        case ANIMAL_CHICKEN:
            speed = 50.0f;
            jumpForce = 250.0f;
            break;
    }
    
    int animalWidth = (animal->type == ANIMAL_BIRD) ? 8 : 12;
    int animalHeight = (animal->type == ANIMAL_FISH) ? 6 : 12;
    
    animal->inWater = IsAnimalInWater(world, animal->x, animal->y, animalWidth, animalHeight);
    
    if (animal->type == ANIMAL_FISH && !animal->inWater) {
        animal->alive = false;
        world->animalCount--;
        return;
    }
    
    if (animal->type == ANIMAL_FISH) {
        animal->state = AI_SWIM;
        animal->velX = animal->direction * speed;
        if (GetRandomValue(0, 100) < 5) {
            animal->velY = GetRandomValue(-50, 50);
        }
    } else {
        if (animal->state == AI_FLEE || (animal->state == AI_WANDER && GetRandomValue(0, 100) < 50)) {
            animal->velX = animal->direction * speed;
        } else {
            animal->velX *= 0.9f;
        }
        
        if (animal->type == ANIMAL_BIRD && GetRandomValue(0, 100) < 10) {
            animal->velY = -jumpForce;
        } else if (animal->onGround && GetRandomValue(0, 100) < 5 && animal->type == ANIMAL_RABBIT) {
            animal->velY = -jumpForce;
        }
        
        if (!animal->inWater) {
            animal->velY += gravity * deltaTime;
        } else {
            animal->velY *= 0.8f;
            animal->velY += gravity * deltaTime * 0.3f;
        }
    }
    
    if (animal->velY > 300.0f) animal->velY = 300.0f;
    if (animal->velY < -300.0f) animal->velY = -300.0f;
    
    float newX = animal->x + animal->velX * deltaTime;
    float newY = animal->y + animal->velY * deltaTime;
    
    if (!CheckAnimalCollision(world, newX, animal->y, animalWidth, animalHeight)) {
        animal->x = newX;
    } else {
        animal->velX = 0;
        animal->direction *= -1;
    }
    
    if (!CheckAnimalCollision(world, animal->x, newY, animalWidth, animalHeight)) {
        animal->y = newY;
        animal->onGround = false;
    } else {
        if (animal->velY > 0) {
            animal->onGround = true;
        }
        animal->velY = 0;
    }
    
    if (animal->x < 0 || animal->x > WORLD_WIDTH * BLOCK_SIZE || 
        animal->y > WORLD_HEIGHT * BLOCK_SIZE) {
        animal->alive = false;
        world->animalCount--;
    }
}

void UpdateAnimals(World* world, float deltaTime) {
    for (int i = 0; i < MAX_ANIMALS; i++) {
        if (world->animals[i].alive) {
            UpdateAnimalAI(world, &world->animals[i], deltaTime);
            UpdateAnimalPhysics(world, &world->animals[i], deltaTime);
        }
    }
    
    if (world->animalCount < 8 && GetRandomValue(0, 1000) < 2) {
        AnimalType type = (AnimalType)GetRandomValue(0, ANIMAL_COUNT - 1);
        float x = GetRandomValue(50, (WORLD_WIDTH - 50) * BLOCK_SIZE);
        float y = FindGroundHeight(world, x / BLOCK_SIZE);
        
        if (type == ANIMAL_FISH) {
            for (int attempt = 0; attempt < 5; attempt++) {
                x = GetRandomValue(50, (WORLD_WIDTH - 50) * BLOCK_SIZE);
                y = GetRandomValue(40, 80) * BLOCK_SIZE;
                if (IsAnimalInWater(world, x, y, 12, 8)) {
                    break;
                }
            }
        }
        
        SpawnAnimal(world, type, x, y);
    }
}

Color GetAnimalColor(AnimalType type) {
    switch (type) {
        case ANIMAL_RABBIT: return (Color){150, 111, 51, 255};
        case ANIMAL_BIRD: return (Color){70, 130, 180, 255};
        case ANIMAL_FISH: return (Color){255, 140, 0, 255};
        case ANIMAL_PIG: return (Color){255, 192, 203, 255};
        case ANIMAL_CHICKEN: return WHITE;
        default: return GRAY;
    }
}

const char* GetAnimalName(AnimalType type) {
    switch (type) {
        case ANIMAL_RABBIT: return "Rabbit";
        case ANIMAL_BIRD: return "Bird";
        case ANIMAL_FISH: return "Fish";
        case ANIMAL_PIG: return "Pig";
        case ANIMAL_CHICKEN: return "Chicken";
        default: return "Unknown";
    }
}

void DrawAnimals(World* world) {
    for (int i = 0; i < MAX_ANIMALS; i++) {
        if (world->animals[i].alive) {
            Animal* animal = &world->animals[i];
            Color color = GetAnimalColor(animal->type);
            
            int width = (animal->type == ANIMAL_BIRD) ? 8 : 12;
            int height = (animal->type == ANIMAL_FISH) ? 6 : 12;
            
            Rectangle animalRect = { animal->x, animal->y, width, height };
            DrawRectangleRec(animalRect, color);
            DrawRectangleLinesEx(animalRect, 1, BLACK);
            
            int eyeOffset = (int)(animal->animTime * 10) % 2;
            DrawCircle(animal->x + width - 3, animal->y + 2 + eyeOffset, 1, BLACK);
            
            if (animal->type == ANIMAL_RABBIT) {
                DrawRectangle(animal->x + 2, animal->y - 3, 2, 4, color);
                DrawRectangle(animal->x + 6, animal->y - 3, 2, 4, color);
            } else if (animal->type == ANIMAL_BIRD) {
                int wingFlap = (int)(animal->animTime * 15) % 3;
                DrawRectangle(animal->x - 2, animal->y + 2 - wingFlap, 4, 2, color);
                DrawRectangle(animal->x + width, animal->y + 2 - wingFlap, 4, 2, color);
            } else if (animal->type == ANIMAL_CHICKEN) {
                DrawRectangle(animal->x + width/2 - 1, animal->y - 2, 2, 3, RED);
            }
        }
    }
}