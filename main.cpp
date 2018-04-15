#include <iostream>
#include <cstdlib>
#include "headers/global.h"
#include "headers/in_game.h"
#include "headers/game_over.h"

// Global shared resources
Font rsu_20_font, rsu_24_font, rsu_26_font, rsu_30_font;
Music background_music;
Sound hit_paddle_sound, hit_brick_sound, hit_top_sound, end_sound, missile_sound;
Texture paddle_texture, ball_texture, in_game_bg_texture, in_game_frame_texture;
Texture blue_brick_texture, stone_brick_texture, crack_stone_brick_texture, item_brick_texture;
Texture orange_brick_texture, red_brick_texture, yellow_brick_texture, barrier_brick_texture;
Texture game_over_overlay_texture, all_lv_clear_overlay_texture;
Texture missile_texture, missiles_left_icon_texture;
GameScene scene;
bool quit;

Font loadFont(const char *font_file, int font_size) {
    Font font = cpLoadFont(font_file, font_size);
    if (!font) {
        cout << "Font init failed (" << font_file << "): " << TTF_GetError() << endl;
        exit(1);
    }
    return font;
}

Music loadMusic(const char *music_file) {
    Music music = cpLoadMusic(music_file);
    if (!music) {
        cout << "Music init failed (" << music_file << "): " << Mix_GetError() << endl;
        exit(1);
    }
    return music;
}

Sound loadSound(const char *sound_file) {
    Sound sound = cpLoadSound(sound_file);
    if (!sound) {
        cout << "Sound init failed (" << sound_file << "): " << Mix_GetError() << endl;
        exit(1);
    }
    return sound;
}

Texture loadTexture(const char *filename) {
    Texture texture = cpLoadTexture(filename);
    if (!texture) {
        cout << "Texture init failed (" << filename << "): " << IMG_GetError() << endl;
        exit(1);
    }
    return texture;
}

void loadResources() {
    // Font
    rsu_20_font = loadFont("fonts/RSU_BOLD.ttf", 20);
    rsu_24_font = loadFont("fonts/RSU_BOLD.ttf", 24);
    rsu_26_font = loadFont("fonts/RSU_BOLD.ttf", 26);
    rsu_30_font = loadFont("fonts/RSU_BOLD.ttf", 30);

    // Sound & Music
    background_music = loadMusic("sounds/bgm.ogg");
    hit_paddle_sound = loadSound("sounds/hitDown.wav");
    hit_brick_sound = loadSound("sounds/hitUp.wav");
    hit_top_sound = loadSound("sounds/hitTop.wav");
    end_sound = loadSound("sounds/theEnd.wav");
    missile_sound = loadSound("sounds/missile.wav");

    // In-game Texture
    in_game_bg_texture = loadTexture("textures/in_game_bg.png");
    in_game_frame_texture = loadTexture("textures/in_game_frame.png");
    // Main Object
    paddle_texture = loadTexture("textures/paddle.png");
    ball_texture = loadTexture("textures/ball.png");
    // Bricks
    blue_brick_texture = loadTexture("textures/blue_brick.png");
    orange_brick_texture = loadTexture("textures/orange_brick.png");
    red_brick_texture = loadTexture("textures/red_brick.png");
    yellow_brick_texture = loadTexture("textures/yellow_brick.png");
    stone_brick_texture = loadTexture("textures/stone_brick.png");
    crack_stone_brick_texture = loadTexture("textures/crack_stone_brick.png");
    item_brick_texture = loadTexture("textures/missile_brick.png");
    barrier_brick_texture = loadTexture("textures/barrier_brick.png");
    // Missile
    missile_texture = loadTexture("textures/missile.png");
    missiles_left_icon_texture = loadTexture("textures/missiles_left_icon.png");

    // GameOver scene Texture
    game_over_overlay_texture = loadTexture("textures/game_over_overlay.png");
    all_lv_clear_overlay_texture = loadTexture("textures/all_lv_clear_overlay.png");
}

int main(int argc, char *args[]) {

    cout << "Starting..." << endl;

    if (!cpInit(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT)) {
        cout << "Window Initialization Failed!" << endl;
        exit(1);
    }

    loadResources();

    // TODO: Create MainMenu scene then remove this line
    scene = InGame;

    // Reduce volume of missile fire sound
    Mix_VolumeChunk(missile_sound, MIX_MAX_VOLUME / 6);

    // Loop play background music
    cpPlayMusic(background_music);

    while (!quit) {
        switch (scene) {
            case MainMenu:
                break;
            case InGame:
                showInGameScene();
                break;
            case GameOver:
                showGameOverScene();
                break;
            case HighScore:
                break;
        }
    }

    cpCleanUp();
    return 0;
}
