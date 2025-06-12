#include "game.h"
#include "resource_dir.h"

void InitGame(World* world) {
    InitPlayer(&world->player);
    
    world->camera.target = (Vector2){ world->player.x, world->player.y };
    world->camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    world->camera.rotation = 0.0f;
    world->camera.zoom = 1.0f;
    
    GenerateWorld(world);
    InitAnimals(world);
}

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "2D Voxel World - Enhanced");
    SetTargetFPS(60);
    
    World world;
    InitGame(&world);
    
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        HandleInventoryInput(&world);
        UpdatePlayer(&world, deltaTime);
        UpdateAnimals(&world, deltaTime);
        HandleBlockInteraction(&world, deltaTime);
        
        BeginDrawing();
        ClearBackground(SKYBLUE);
        
        BeginMode2D(world.camera);
        DrawWorld(&world);
        DrawAnimals(&world);
        DrawPlayer(&world);
        EndMode2D();
        
        DrawUI(&world);
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
