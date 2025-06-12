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

int FindSurfaceHeight(World* world, int x) {
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        if (world->blocks[y][x] != BLOCK_AIR) {
            return y - 1;
        }
    }
    return WORLD_HEIGHT - 1;
}

void GenerateTree(World* world, int x, int baseY) {
    int treeHeight = GetRandomValue(5, 12);
    
    for (int h = 0; h < treeHeight; h++) {
        int y = baseY - h;
        if (y >= 0 && y < WORLD_HEIGHT) {
            world->blocks[y][x] = BLOCK_WOOD;
        }
    }
    
    int leafCenterY = baseY - treeHeight + 2;
    for (int dy = -3; dy <= 1; dy++) {
        for (int dx = -3; dx <= 3; dx++) {
            int leafX = x + dx;
            int leafY = leafCenterY + dy;
            
            if (leafX >= 0 && leafX < WORLD_WIDTH && leafY >= 0 && leafY < WORLD_HEIGHT) {
                float distance = sqrt(dx * dx + dy * dy);
                if (distance <= 3.0f && world->blocks[leafY][leafX] == BLOCK_AIR) {
                    if (GetRandomValue(0, 100) < 80) {
                        world->blocks[leafY][leafX] = BLOCK_LEAVES;
                    }
                }
            }
        }
    }
}

void GenerateLake(World* world, int centerX, int centerY, int width, int height) {
    for (int dy = 0; dy < height; dy++) {
        for (int dx = -width/2; dx <= width/2; dx++) {
            int x = centerX + dx;
            int y = centerY + dy;
            
            if (x >= 0 && x < WORLD_WIDTH && y >= 0 && y < WORLD_HEIGHT) {
                float distFromCenter = sqrt(dx * dx + (dy * 2) * (dy * 2)) / (float)width;
                if (distFromCenter <= 1.0f) {
                    world->blocks[y][x] = BLOCK_WATER;
                }
            }
        }
    }
}

void GenerateRiver(World* world, int startX, int endX, int* surfaceHeights) {
    if (startX > endX) {
        int temp = startX;
        startX = endX;
        endX = temp;
    }
    
    int riverWidth = GetRandomValue(2, 4);
    int riverDepth = GetRandomValue(3, 6);
    
    for (int x = startX; x <= endX; x++) {
        int centerY = surfaceHeights[x];
        
        for (int dy = 0; dy < riverDepth; dy++) {
            for (int dx = -riverWidth/2; dx <= riverWidth/2; dx++) {
                int riverX = x + dx;
                int riverY = centerY + dy;
                
                if (riverX >= 0 && riverX < WORLD_WIDTH && riverY >= 0 && riverY < WORLD_HEIGHT) {
                    if (dy < riverDepth - 1) {
                        world->blocks[riverY][riverX] = BLOCK_WATER;
                    } else {
                        if (world->blocks[riverY][riverX] != BLOCK_STONE) {
                            world->blocks[riverY][riverX] = BLOCK_WATER;
                        }
                    }
                }
            }
        }
    }
}

void GenerateWorld(World* world) {
    srand(time(NULL));
    
    int surfaceHeights[WORLD_WIDTH];
    
    for (int x = 0; x < WORLD_WIDTH; x++) {
        float heightNoise = PerlinNoise(x * 0.1f, 0) * 0.5f + 0.5f;
        int surfaceHeight = (int)(heightNoise * 30) + 40;
        surfaceHeights[x] = surfaceHeight;
        
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
        }
    }
    
    for (int i = 0; i < 30; i++) {
        int x = GetRandomValue(5, WORLD_WIDTH - 5);
        int y = GetRandomValue(60, WORLD_HEIGHT - 5);
        int caveSize = GetRandomValue(2, 4);
        
        for (int dx = -caveSize; dx <= caveSize; dx++) {
            for (int dy = -caveSize; dy <= caveSize; dy++) {
                if (x + dx >= 0 && x + dx < WORLD_WIDTH && y + dy >= 0 && y + dy < WORLD_HEIGHT) {
                    if (dx * dx + dy * dy <= caveSize * caveSize) {
                        world->blocks[y + dy][x + dx] = BLOCK_AIR;
                    }
                }
            }
        }
    }
    
    for (int i = 0; i < 3; i++) {
        int x = GetRandomValue(30, WORLD_WIDTH - 30);
        int surfaceY = surfaceHeights[x];
        int lakeWidth = GetRandomValue(8, 16);
        int lakeHeight = GetRandomValue(4, 8);
        
        GenerateLake(world, x, surfaceY, lakeWidth, lakeHeight);
    }
    
    for (int i = 0; i < 2; i++) {
        int startX = GetRandomValue(10, WORLD_WIDTH / 3);
        int endX = GetRandomValue(2 * WORLD_WIDTH / 3, WORLD_WIDTH - 10);
        GenerateRiver(world, startX, endX, surfaceHeights);
    }
    
    for (int attempt = 0; attempt < 40; attempt++) {
        int x = GetRandomValue(10, WORLD_WIDTH - 10);
        int surfaceY = FindSurfaceHeight(world, x);
        
        if (surfaceY > 0 && surfaceY < 60 && 
            world->blocks[surfaceY][x] == BLOCK_AIR && 
            world->blocks[surfaceY + 1][x] == BLOCK_GRASS) {
            
            bool hasSpace = true;
            for (int checkX = x - 3; checkX <= x + 3; checkX++) {
                for (int checkY = surfaceY - 12; checkY <= surfaceY; checkY++) {
                    if (checkX >= 0 && checkX < WORLD_WIDTH && checkY >= 0 && checkY < WORLD_HEIGHT) {
                        if (world->blocks[checkY][checkX] != BLOCK_AIR) {
                            if (checkY < surfaceY && checkX >= x - 1 && checkX <= x + 1) {
                                hasSpace = false;
                                break;
                            }
                        }
                    }
                }
                if (!hasSpace) break;
            }
            
            if (hasSpace) {
                GenerateTree(world, x, surfaceY);
            }
        }
    }
    
    for (int x = 0; x < WORLD_WIDTH; x++) {
        int surfaceY = surfaceHeights[x];
        if (GetRandomValue(0, 100) < 15) {
            if (surfaceY > 0 && world->blocks[surfaceY - 1][x] == BLOCK_AIR) {
                int grassHeight = GetRandomValue(1, 3);
                for (int h = 0; h < grassHeight; h++) {
                    int y = surfaceY - 1 - h;
                    if (y >= 0 && world->blocks[y][x] == BLOCK_AIR) {
                        world->blocks[y][x] = BLOCK_LEAVES;
                    }
                }
            }
        }
    }
}