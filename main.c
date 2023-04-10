#include "raylib.h"
#include "rlgl.h"
#include "raygui.h"
#include "raymath.h"
#include <stdio.h>
#include "helper.c"

#define MAP_WIDTH 10
#define MAP_HEIGHT 10
void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color);
void DrawCubeTextureRec(Texture2D texture, Rectangle source, Vector3 position, float width, float height, float length, Color color); // Draw cube with a region of a texture
bool checkCollisionAABB(Rectangle rec1, Rectangle rec2);

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Unnamed shooter game");

    // Define the camera to look into our 3d world
    Camera camera = {0};
    camera.position = (Vector3){4.0f, 4.0f, 4.0f}; // Camera position
    camera.target = (Vector3){0.0f, 2.0f, 0.0f};   // Camera looking at point
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};       // Camera up vector (rotation towards target)
    camera.fovy = 110.0f;                          // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;        // Camera projection type

    // Textures
    Texture2D floorTexture = LoadTexture("assets/floor.png");
    Texture2D ceilingTexture = LoadTexture("assets/ceiling.png");
    Texture2D wallTexture = LoadTexture("assets/wall.png");

    int map[MAP_HEIGHT][MAP_WIDTH] = {
        {0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 0, 1, 1, 0, 0, 0, 1},
        {1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
        {1, 0, 1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };

    DisableCursor();
    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose())
    {
        /*
            Checklist;
                - Collision - half working.
                - Pause menu
                - Enemies (billboard sprites)
                - Guns (shooting the enemies and detection of doing that)
                - HUD (I mean player health and stamina)
                - Map file
                - Custom Textures (so you dont have to modify the code)
                - Multiplayer (please dont)
        */

        float charSpeed = 0.4;
        float sensitivity = 0.05;
        float playerThickness = 0;
        float cubeSize = 10; // basically tile size. (And wall X and Z)
        if (IsKeyDown(KEY_LEFT_SHIFT))
        {
            charSpeed = 0.6;
        }
        else if (IsKeyDown(KEY_LEFT_CONTROL))
        {
            charSpeed = 0.2; 
        }

        Vector3 velocity = (Vector3){
            (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) * charSpeed - (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) * charSpeed,
            (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) * charSpeed - (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) * charSpeed,
            0.0f};

        Vector3 destination = Vector3Add(camera.position, velocity);
        Vector2 plrMin = (Vector2){destination.x + playerThickness, destination.z + playerThickness};
        Vector2 plrMax = (Vector2){destination.x - playerThickness, destination.z - playerThickness};
        int minCubeX = floor(plrMin.x / cubeSize);
        int minCubeY = floor(plrMin.y / cubeSize);
        int maxCubeX = ceil(plrMax.x / cubeSize);
        int maxCubeY = ceil(plrMax.y / cubeSize);

        Rectangle playerRec = (Rectangle){plrMin.x, plrMin.y, plrMax.x - plrMin.x, plrMax.y - plrMin.y};

        for (int i = minCubeX; i <= maxCubeX; i++)
        {
            for (int j = minCubeY; j <= maxCubeY; j++)
            {
                if (map[i][j] == 0)
                    continue;
                Rectangle cubeRec = (Rectangle){(i * cubeSize) - cubeSize / 2, (j * cubeSize) - cubeSize / 2, cubeSize, cubeSize};
                if (checkCollisionAABB(playerRec, cubeRec))
                {
                    velocity = (Vector3){0, 0, 0};
                }
            }
        }

        UpdateCameraPro(&camera, velocity,
                        (Vector3){
                            GetMouseDelta().x * sensitivity, // Rotation: yaw
                            GetMouseDelta().y * sensitivity, // Rotation: pitch
                            0.0f                             // Rotation: roll
                        },
                        GetMouseWheelMove() * 0.0f); // Move to target (zoom)

        BeginDrawing();

        ClearBackground((Color){255, 255, 255});

        BeginMode3D(camera);
        // Imagine these as X and Y

        float floorHeight = 0;
        float ceilingHeight = 10; // You need to use this for the wall height aswell in this case. (Wall Y)
        float wallHeight = ceilingHeight - floorHeight;

        // Iterate over map[1][1-10] then map[2][1-10] etc..
        for (int i = 0; i < MAP_WIDTH; i++)
        {
            for (int j = 0; j < MAP_HEIGHT; j++)
            {
                if (map[i][j] == 1)
                {
                    DrawCubeTexture(wallTexture, (Vector3){i * cubeSize, wallHeight / 2 + floorHeight, j * cubeSize}, cubeSize, wallHeight, cubeSize, WHITE);
                }
                else
                {
                    DrawCubeTexture(floorTexture, (Vector3){i * cubeSize, floorHeight, j * cubeSize}, cubeSize, 0.0f, cubeSize, WHITE);
                    // Draw ceiling & floor
                    DrawCubeTexture(ceilingTexture, (Vector3){i * cubeSize, ceilingHeight, j * cubeSize}, cubeSize, 0.0f, cubeSize, BROWN);
                }
            }
        }

        EndMode3D();

        DrawText("Stamina: 10/10", 10, 10, 20, RED);
        DrawText("Health: 10/10", 10, 30, 20, RED);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}

bool checkCollisionAABB(Rectangle rec1, Rectangle rec2)
{
    if (rec1.x < rec2.x + rec2.width &&
        rec1.x + rec1.width > rec2.x &&
        rec1.y < rec2.y + rec2.height &&
        rec1.y + rec1.height > rec2.y)
    { return true; }
    return false;
}