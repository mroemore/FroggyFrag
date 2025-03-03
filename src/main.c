#include <stdlib.h>
#include <math.h>

#include "raylib.h"

// #if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
// #else   // PLATFORM_ANDROID, PLATFORM_WEB
//     #define GLSL_VERSION            100
// #endif

typedef struct {
    Font* font;
    int glyph_widths[255];

} FontInfo;

void AnimateText(Font f, FontInfo fi, const char* text, Vector2 pos, float size, int spacing, Color c, float time);
void initFontInfo(FontInfo *fi, Font f);
int main(void){
    const int screenWidth = 640;
    const int screenHeight = 480;

    const int virtualScreenWidth = 320;
    const int virtualScreenHeight = 240;
    const float virtualRatio = (float)screenWidth/(float)virtualScreenWidth;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - smooth pixel-perfect camera");

    const char msg[256] = "It finally works!";

    Font fontTtf = LoadFont("resources/alagard.png");// LoadFontEx("resources/Daydream.ttf", 16, 0, 252);
    FontInfo fi;
    initFontInfo(&fi, fontTtf);
    SetTextLineSpacing(8);

    Camera2D worldSpaceCamera = { 0 };  // Game world camera
    worldSpaceCamera.zoom = 1.0f;

    Camera2D screenSpaceCamera = { 0 }; // Smoothing camera
    screenSpaceCamera.zoom = 1.0f;

    RenderTexture2D target = LoadRenderTexture(virtualScreenWidth, virtualScreenHeight); // This is where we'll draw all our objects.

    // The target's height is flipped (in the source Rectangle), due to OpenGL reasons
    Rectangle sourceRec = { 0.0f, 0.0f, (float)target.texture.width, -(float)target.texture.height };
    Rectangle destRec = { -virtualRatio, -virtualRatio, screenWidth + (virtualRatio*2), screenHeight + (virtualRatio*2) };

    Vector2 origin = { 0.0f, 0.0f };

    float rotation = 0.0f;

    float cameraX = 0.0f;
    float cameraY = 0.0f;

    Shader shader = LoadShader(0, "resources/shaders/crtshader.frag.glsl");
    Image smiley = LoadImage("resources/smiley.png");
    Texture2D smileTex = LoadTextureFromImage(smiley);      // Image converted to texture, uploaded to GPU memory (VRAM)
    UnloadImage(smiley);

    SetTargetFPS(60);

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        float time = GetTime();
        BeginTextureMode(target);
                BeginMode2D(worldSpaceCamera);
                ClearBackground(RAYWHITE);
                AnimateText(fontTtf, fi, msg, (Vector2){ 8.0f, 100.0f }, (float)fontTtf.baseSize, 4, LIME, GetTime());
                DrawTexture(smileTex, 10,10,LIME);
//                DrawText("Hi there, world.", 10,110, 16, LIME);
                EndMode2D();
        EndTextureMode();
        
        // NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
            //DrawTextureRec(target.texture, (Rectangle){ 0, 0, (float)target.texture.width, (float)-target.texture.height }, origin, RED);

        BeginDrawing();
            ClearBackground(WHITE);
            

            //BeginMode2D(screenSpaceCamera);
            BeginShaderMode(shader);
            SetShaderValue(shader, GetShaderLocation(shader, "iTime"), &time, SHADER_UNIFORM_FLOAT);
                //DrawTextureRec(target.texture, (Rectangle){ 0, 0, (float)target.texture.width, (float)-target.texture.height }, origin, RED);
                DrawTexturePro(target.texture, sourceRec, destRec, origin, 0.0f, WHITE);
            EndShaderMode();

            //EndMode2D();

            DrawText(TextFormat("Screen resolution: %ix%i", screenWidth, screenHeight), 10, 10, 20, DARKBLUE);
            DrawText(TextFormat("World resolution: %ix%i", virtualScreenWidth, virtualScreenHeight), 10, 40, 20, DARKGREEN);
            DrawFPS(GetScreenWidth() - 95, 10);
        EndDrawing();
    }
}

void AnimateText(Font f, FontInfo fi, const char* text, Vector2 pos, float size, int spacing, Color c, float time){
    const int anim_range = 3;
    const float speed = 1.5f;
    
    int p_i = 0;
    char letter[2] = {text[p_i], '\0'};
    int total_spacing = 0;
    time *= speed;

    while(letter[0] != '\0'){
        float sinmod_a = sinf(p_i + time);
        int new_x = pos.x + (p_i * spacing) + total_spacing;
        int new_y = pos.y + sinmod_a * anim_range;
        Color cn = (Color){c.r, (int)(c.g * (sinmod_a)), c.b, c.a};
        DrawTextEx(f, letter, (Vector2){new_x, new_y}, size, 2, cn);
        total_spacing += (fi.glyph_widths[(int)letter[0]] + spacing)/2;
        p_i++;
        letter[0] = text[p_i];
    }
}

void initFontInfo(FontInfo *fi, Font f){
    char letter[2] = {(char)0, '\0'};

    for(int i = 0; i < 255; i++){
        letter[0] = (char)i;
        fi->glyph_widths[i] = MeasureText(letter, f.baseSize);
    }
}