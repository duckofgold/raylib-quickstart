#include "game.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

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
    
    for (int i = 0; i < 20; i++) {
        int x = GetRandomValue(10, WORLD_WIDTH - 10);
        int y = GetRandomValue(20, 50);
        
        if (world->blocks[y][x] == BLOCK_AIR) {
            for (int h = 0; h < GetRandomValue(5, 12); h++) {
                if (y + h < WORLD_HEIGHT) {
                    world->blocks[y + h][x] = BLOCK_WOOD;
                }
            }
            
            for (int dy = -2; dy <= 2; dy++) {
                for (int dx = -2; dx <= 2; dx++) {
                    if (x + dx >= 0 && x + dx < WORLD_WIDTH && y + dy >= 0 && y + dy < WORLD_HEIGHT) {
                        if (dx * dx + dy * dy <= 4 && world->blocks[y + dy][x + dx] == BLOCK_AIR) {
                            world->blocks[y + dy][x + dx] = BLOCK_LEAVES;
                        }
                    }
                }
            }
        }
    }
}