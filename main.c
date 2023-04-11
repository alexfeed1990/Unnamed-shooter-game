#include "raylib.h"
#include "rlgl.h"
#include "raygui.h"
#include "raymath.h"
#include <stdio.h>
#include "helper.c"

#define MAP_WIDTH 10
#define MAP_HEIGHT 10
#define CUBE_SIZE 10
void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color);
void DrawCubeTextureRec(Texture2D texture, Rectangle source, Vector3 position, float width, float height, float length, Color color); // Draw cube with a region of a texture
void makeRectsFromMap(int map[MAP_WIDTH][MAP_HEIGHT], Rectangle rects[MAP_WIDTH * MAP_HEIGHT]); 

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
        {1, 0, 1, 1, 1, 1, 1, 1, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 1, 1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

    DisableCursor();
    SetTargetFPS(60);

    Vector2 PlayerOrigin = {300, 300}; // position

    float Radius = 25; // size of circle OR can be used as player size

    // Main game loop
    while (!WindowShouldClose())
    {
        SetExitKey(KEY_DELETE);
        PlayerOrigin = (Vector2){camera.position.x, camera.position.z};

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

        // ------------------ Movement and collision -------------- //

        float charSpeed = 0.4;
        float sensitivity = 0.05;

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
        Vector2 newPosOrigin = (Vector2){destination.x, destination.z};
        Rectangle Rects[MAP_WIDTH * MAP_HEIGHT];
        makeRectsFromMap(map, Rects);
        int RectCount = MAP_WIDTH * MAP_HEIGHT;

        for (int i = 0; i < RectCount; i++)
        {
            Vector2 hitPoint = {-100, -100}; // position of where the raycast hit
            Vector2 hitNormal = {0, 0};      // normal of the face it hits
            PointNearestRectanglePoint(Rects[i], newPosOrigin, &hitPoint, &hitNormal);

            Vector2 vectorToHit = Vector2Subtract(hitPoint, newPosOrigin); // distance between new position and hitpoint

            bool inside = Vector2LengthSqr(vectorToHit) < Radius * Radius; // if distance == radisu size then kys

            if (inside) // if inside
            {
                // make the distance vector normal
                vectorToHit = Vector2Normalize(vectorToHit);

                // project that out to the radius to find the point that should be 'deepest' into the rectangle.
                // add the calculated position to the point the vector hit * the size of the player
                Vector2 projectedPoint = Vector2Add(newPosOrigin, Vector2Scale(vectorToHit, Radius));

                // compute the shift to take the deepest point out to the edge of our nearest hit, based on the vector direction
                Vector2 delta = {0, 0};

                if (hitNormal.x != 0)
                    delta.x = hitPoint.x - projectedPoint.x;
                else
                    delta.y = hitPoint.y - projectedPoint.y;

                // shift the new point by the delta to push us outside of the rectangle
                newPosOrigin = Vector2Add(newPosOrigin, delta);
            }
        }
        PlayerOrigin = newPosOrigin;

        UpdateCameraPro(&camera, velocity,
                        (Vector3){
                            GetMouseDelta().x * sensitivity, // Rotation: yaw
                            GetMouseDelta().y * sensitivity, // Rotation: pitch
                            0.0f                             // Rotation: roll
                        },
                        GetMouseWheelMove() * 0.0f); // Move to target (zoom)

        camera.position = (Vector3){PlayerOrigin.x, camera.position.y, PlayerOrigin.y};


        // ------------------ Init -------------- //

        BeginDrawing();

        ClearBackground((Color){255, 255, 255});

        BeginMode3D(camera);

        // ------------------ Rendering world  -------------- //

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
                    DrawCubeTexture(wallTexture, (Vector3){i * CUBE_SIZE, wallHeight / 2 + floorHeight, j * CUBE_SIZE}, CUBE_SIZE, wallHeight, CUBE_SIZE, WHITE);
                }
                else
                {
                    DrawCubeTexture(floorTexture, (Vector3){i * CUBE_SIZE, floorHeight, j * CUBE_SIZE}, CUBE_SIZE, 0.0f, CUBE_SIZE, WHITE);
                    // Draw ceiling & floor
                    DrawCubeTexture(ceilingTexture, (Vector3){i * CUBE_SIZE, ceilingHeight, j * CUBE_SIZE}, CUBE_SIZE, 0.0f, CUBE_SIZE, BROWN);
                }
            }
        }

        EndMode3D();

        // ------------------ HUD -------------- //

        DrawText("Stamina: 10/10", 10, 10, 20, RED);
        DrawText("Health: 10/10", 10, 30, 20, RED);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}

void makeRectsFromMap(int map[MAP_WIDTH][MAP_HEIGHT], Rectangle rects[MAP_WIDTH * MAP_HEIGHT])
{
    int count = 0;
    for (int i = 0; i < MAP_WIDTH; i++)
    {
        for (int j = 0; j < MAP_HEIGHT; j++)
        {
            if (map[i][j] == 1)
            {
                rects[count] = (Rectangle){MAP_WIDTH * CUBE_SIZE,
                                           MAP_HEIGHT * CUBE_SIZE,
                                           CUBE_SIZE,
                                           CUBE_SIZE};
                count++;
            }
        }
    }
}