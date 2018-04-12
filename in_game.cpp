#include <cmath>
#include <string>
#include "headers/global.h"
#include "headers/object.h"
#include "headers/ball.h"
#include "headers/brick.h"
#include "headers/item_brick.h"
#include "headers/missile.h"

// Global shared resources from main
extern Font rsu_24_font, rsu_30_font;
extern Sound hit_paddle_sound, hit_brick_sound, hit_top_sound, end_sound, missile_sound;
extern Texture paddle_texture, ball_texture, in_game_bg_texture, in_game_frame_texture;
extern Texture blue_brick_texture, stone_brick_texture, crack_stone_brick_texture, missile_texture;
extern GameScene scene;
extern bool quit;

// Global vars for In-game Scene
Object paddle(124, 18);
Ball ball(20, 20);
Brick bricks[100];
Missile missiles[30];

typedef enum {NoCollide, CollideTop, CollideBottom, CollideLeft, CollideRight} CollisionSide;
int n_bricks, score, balls_left;
bool is_game_start;

/*  Collision Detection between two objects a and b
    Return collision side of objects a  */
CollisionSide collide(Object a, Object b) {
    float w = (a.getWidth() + b.getWidth()) / 2;
    float h = (a.getHeight() + b.getHeight()) / 2;
    float dx = (a.getX() + a.getWidth() / 2) - (b.getX() + b.getWidth() / 2);
    float dy = (a.getY() + a.getHeight() / 2) - (b.getY() + b.getHeight() / 2);

    // Collision detect
    if (fabs(dx) <= w && fabs(dy) <= h) {
        float wy = w * dy;
        float hx = h * dx;

        if (wy > hx) {
            if (wy > -hx)
                return CollideTop;
            else
                return CollideRight;
        } else {
            if (wy > -hx)
                return CollideLeft;
            else
                return CollideBottom;
        }
    }
    // No collision detect
    return NoCollide;
}

void initBricksLevel(int level) {
    if (level == 1) {
        n_bricks = 96;
        for (int i = 0, x = 100, y = 120; i < n_bricks; i++) {
                bricks[i].setX(x);
                bricks[i].setY(y);
                bricks[i].setSize(50, 25);
                if (i % 5) {
                    bricks[i].setTexture(blue_brick_texture);
                    bricks[i].setScore(5);
                } else {
                    bricks[i].setTexture(stone_brick_texture);
                    bricks[i].setCrackTexture(crack_stone_brick_texture);
                    bricks[i].setDurability(2);
                    bricks[i].setScore(20);
                }
            if (x > WINDOW_WIDTH - 200)
                x = 100, y += 25;
            else
                x += 50;
        }
    } else if (level == 2) {

    } else if (level == 3) {

    }
}

void drawScoreText(int score) {
    for (int i = 0, x = 150; i < 5; i++, x -= 27) {
        cpDrawText(255, 255, 255, 216, x, 43, to_string(score % 10).c_str(), rsu_30_font, true);
        score /= 10;
    }
}

void drawInGameTexture() {
    // In-game Background
    cpDrawTexture(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, in_game_bg_texture);
    // Missile
    for (int i = 0; i < 30; i++) {
        if (missiles[i].isVisible())
            missiles[i].drawTexture();
    }
    // Paddle
    if (scene == InGame)
        paddle.drawTexture();
    // Ball
    ball.drawTexture();
    // Bricks
    for (int i = 0; i < n_bricks; i++) {
        // if brick have durability then draw a brick
        if (bricks[i].getDurability())
            bricks[i].drawTexture();
    }
    // In-game Frame
    cpDrawTexture(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, in_game_frame_texture);
    // Score text
    drawScoreText(score);
    // Balls left text
    cpDrawText(255, 255, 255, 216, 758, 43, to_string(balls_left).c_str(), rsu_30_font, true);
}

void showInGameScene() {
    // For store event from PollEvent function
    Event event;
    // Set ball speed & start angle (0 degree = go up straight)
    float ball_vel = 9, bounce_angle = 0;
    // Set max ball angle when collide w/ paddle = 70 degee angle
    const float MAX_BOUNCE_ANGLE = 7 * M_PI / 18;
    // Temp vars for calculate angle
    float relative_intersect, normalized_relative_intersect;
    // For mouse hold detect
    bool is_mouse_down = false;
    // Missile fire delay
    int missile_tick = 0;
    // Store collision side
    CollisionSide side;

    // Hide mouse cursor
    setMouseVisible(false);

    // Set mouse x position to center
    setMouseX(WINDOW_WIDTH / 2);

    // Init paddle position
    paddle.setX(getMouseX() - paddle.getWidth() / 2);
    paddle.setY(WINDOW_HEIGHT - 80);

    // Prevent paddle get out of window (In-game Frame side border width = 9px)
    if (paddle.getX() < 10)
        paddle.setX(10);
    else if (paddle.getX() + paddle.getWidth() > WINDOW_WIDTH - 10)
        paddle.setX(WINDOW_WIDTH - paddle.getWidth() - 10);

    // Init ball position
    ball.setX(paddle.getX() + paddle.getWidth() / 2 - ball.getWidth() / 2);
    ball.setY(paddle.getY() - ball.getHeight() - 1);

    // Init ball velocity in x/y pos
    ball.setVelX(ball_vel * sin(bounce_angle));
    ball.setVelY(-ball_vel * cos(bounce_angle));

    // Set ball/paddle texture
    ball.setTexture(ball_texture);
    paddle.setTexture(paddle_texture);

    // Reset Missiles
    for (int i = 0; i < 30; i++) {
        missiles[i].setVisible(false);
    }

    // Init number of balls left
    balls_left = 2;

    // Init start score
    score = 0;

    // Init game status (ball will stick w/ paddle)
    is_game_start = false;

    // Init bricks for level 1
    initBricksLevel(1);

    while (true) {
        cpClearScreen();
        drawInGameTexture();

        // Update Screen
        cpSwapBuffers();

        // Handle events
        while (cbPollEvent(&event)) {
            if (event.type == QUIT) {
                quit = true;
                return;
            }
            // User released ESC key
            if (event.type == KEYUP && event.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
                return;
            }
            // Start the game (release ball) when user press left click
            if (!is_game_start && event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                is_game_start = true;
                break;
            }
            // Left click mouse hold down detect
            if (event.button.button == SDL_BUTTON_LEFT) {
                if (is_game_start && event.type == SDL_MOUSEBUTTONDOWN)
                    is_mouse_down = true;
                else if (event.type == SDL_MOUSEBUTTONUP)
                    is_mouse_down = false, missile_tick = 10;
            }
        }

        if (is_game_start && is_mouse_down) {
            if (missile_tick == 10) {
                missile_tick = 0;
                for (int i = 0, amount = 0; i < 30; i++) {
                    // Launch 2 new missiles
                    if (!missiles[i].isVisible()) {
                        missiles[i].setSize(8, 18);
                        if (amount == 0)
                            // Left side of paddle
                            missiles[i].setX(paddle.getX() + 3);
                        else
                            // Right side of paddle
                            missiles[i].setX(paddle.getX() + paddle.getWidth() - (missiles[i].getWidth() + 3));
                        missiles[i].setY(paddle.getY());
                        missiles[i].setTexture(missile_texture);
                        missiles[i].setVisible(true);
                        // Play missile launch sound
                        cpPlaySound(missile_sound);
                        amount++;
                        // Fire 2 missiles per 1 time
                        if (amount == 2)
                            break;
                    }
                }
            } else
                missile_tick++;
        }

        for (int i = 0; i < 30; i++) {
            if (missiles[i].isVisible()) {
                // Missile Speed = 7
                missiles[i].setY(missiles[i].getY() - 7);
                // Reset Missile when get out of window
                if (missiles[i].getY() + missiles[i].getHeight() < 64)
                    missiles[i].setVisible(false);
                else {
                    // When missile hit a brick
                    for (int j = 0; j < n_bricks; j++) {
                        if (bricks[j].getDurability() && collide(missiles[i], bricks[j])) {
                            missiles[i].setVisible(false);
                            // Play hit sound
                            cpPlaySound(hit_brick_sound);
                            // Decrease brick durability by 1
                            bricks[j].decreaseDurability();
                            // + Score when brick is break
                            if (!bricks[j].getDurability())
                                score += bricks[j].getScore();
                        }
                    }
                }
            }
        }

        // Set paddle position related to mouse position
        paddle.setX(getMouseX() - paddle.getWidth() / 2);

        // Prevent paddle get out of window (In-game Frame side border width = 10px)
        if (paddle.getX() < 10)
            paddle.setX(10);
        else if (paddle.getX() + paddle.getWidth() > WINDOW_WIDTH - 10)
            paddle.setX(WINDOW_WIDTH - paddle.getWidth() - 10);

        if (!is_game_start)
            // Set ball position to center of paddle
            ball.setX(paddle.getX() + paddle.getWidth() / 2 - ball.getWidth() / 2);
        else
            // Set ball position related to ball x/y velocity
            ball.move();

        // When Ball fall down
        if (ball.getY() > WINDOW_HEIGHT) {
            // Stop the game (make ball stick w/ paddle)
            is_game_start = false;
            // No balls left to play
            if (!balls_left) {
                scene = GameOver;
                return;
            }
            // Decrease number of balls left by 1
            balls_left--;
            // Reset ball position
            ball.setX(paddle.getX() + paddle.getWidth() / 2 - ball.getWidth() / 2);
            ball.setY(paddle.getY() - ball.getHeight() - 1);
            // Reset ball velocity in x/y pos
            bounce_angle = 0;
            ball.setVelX(ball_vel * sin(bounce_angle));
            ball.setVelY(-ball_vel * cos(bounce_angle));
        }

        // Prevent ball get out of window (In-game Frame side border width = 9px)
        if (ball.getX() < 10) {
            ball.setX(10);
            ball.invertVelX();
        }
        if (ball.getX() + ball.getWidth() > WINDOW_WIDTH - 10) {
            ball.setX(WINDOW_WIDTH - ball.getWidth() - 10);
            ball.invertVelX();
        }
        // In-game Frame header height = 63px
        if (ball.getY() < 64) {
            cpPlaySound(hit_top_sound);
            ball.setY(64);
            ball.invertVelY();
        }

        for (int i = 0; i < n_bricks; i++) {
            // When ball hit a brick
            if (bricks[i].getDurability() && (side = collide(bricks[i], ball))) {
                // Play hit sound
                cpPlaySound(hit_brick_sound);

                // Decrease durability by 1
                bricks[i].decreaseDurability();

                // + Score when brick is break
                if (!bricks[i].getDurability())
                    score += bricks[i].getScore();

                // Prevent ball get into brick
                switch (side) {
                    case CollideTop:
                        ball.setY(bricks[i].getY() - ball.getHeight() - 1);
                        // Set ball to go up only
                        ball.setVelY(-fabs(ball.getVelY()));
                        break;
                    case CollideBottom:
                        ball.setY(bricks[i].getY() + bricks[i].getHeight() + 1);
                        // Set ball to go down only
                        ball.setVelY(fabs(ball.getVelY()));
                        break;
                    case CollideLeft:
                        ball.setX(bricks[i].getX() - ball.getWidth() - 1);
                        // Set ball to go only left side
                        ball.setVelX(-fabs(ball.getVelX()));
                        break;
                    case CollideRight:
                        ball.setX(bricks[i].getX() + bricks[i].getWidth() + 1);
                        // Set ball to go only right side
                        ball.setVelX(fabs(ball.getVelX()));
                        break;
                    // suppress compiler warning
                    default:
                        break;
                }
            }
        }

        // When ball hit paddle
        if ((side = collide(paddle, ball))) {
            cpPlaySound(hit_paddle_sound);

            // Set new ball angle related to collide position on paddle
            relative_intersect = (ball.getX() + ball.getWidth() / 2) - (paddle.getX() + paddle.getWidth() / 2);
            normalized_relative_intersect = relative_intersect / (paddle.getWidth() / 2);
            bounce_angle = normalized_relative_intersect * MAX_BOUNCE_ANGLE;

            ball.setVelX(ball_vel * sin(bounce_angle));
            ball.setVelY(-ball_vel * cos(bounce_angle));

            // Prevent ball get into paddle
            switch (side) {
                case CollideTop:
                    ball.setY(paddle.getY() - ball.getHeight() - 1);
                    break;
                case CollideLeft:
                    ball.setX(paddle.getX() - ball.getWidth() - 1);
                    // Set ball to go only left side
                    ball.setVelX(-fabs(ball.getVelX()));
                    break;
                case CollideRight:
                    ball.setX(paddle.getX() + paddle.getWidth() + 1);
                    // Set ball to go only right side
                    ball.setVelX(fabs(ball.getVelX()));
                    break;
                // suppress compiler warning
                default:
                    break;
            }
        }

        cpDelay(10);
    }
}