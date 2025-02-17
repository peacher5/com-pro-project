#include <cmath>
#include <string>
#include "headers/global.h"
#include "headers/pause_menu.h"
#include "headers/object.h"
#include "headers/ball.h"
#include "headers/brick.h"
#include "headers/barrier_brick.h"
#include "headers/item_brick.h"
#include "headers/stone_brick.h"
#include "headers/missile.h"

// Global shared resources from main
extern Font rsu_24_font, rsu_26_font, rsu_30_font;
extern Sound hit_paddle_sound, hit_brick_sound, hit_top_sound, end_sound, missile_sound;
extern Texture black_bg_texture;
extern Texture paddle_texture, ball_texture, in_game_bg_texture, in_game_frame_texture;
extern Texture blue_brick_texture, stone_brick_texture, crack_stone_brick_texture, missile_texture;
extern Texture missiles_left_icon_texture, barrier_brick_texture;
extern Texture orange_brick_texture, red_brick_texture, yellow_brick_texture;
extern Texture blue_item_brick_texture, orange_item_brick_texture;
extern Texture red_item_brick_texture, yellow_item_brick_texture;
extern Texture level1_icon_texture, level2_icon_texture;
extern GameScene scene;
extern bool quit;

// Global vars for In-game Scene
// Max object on screen
const int MAX_BALLS = 50;
const int MAX_BRICKS = 120;
const int MAX_MISSILES = 30;

Object paddle(124, 18);
Ball balls[MAX_BALLS];
Brick **bricks;
Missile missiles[MAX_MISSILES];

// Animate dim screen at start (255 to 0)
int screen_opacity;
// Animate level badge fade out
float level_badge_opacity;
// Store current level (1-2)
int level;
// Store current game score, increase animate score & animation delay tick
int score, animate_score, animate_score_tick;
// Number of balls & missiles left
int balls_left, missiles_left;
// Start ball speed & start angle
float ball_vel, bounce_angle;
// Count breakable bricks & barrier bricks
int n_bricks, n_barrier_bricks;
// Count number of breakable bricks that's destroyed
int n_breaks;
// Number of balls on screen
int n_balls;
// is_game_start:
//   true => Ball is release from paddle
//  false => Ball stick w/ paddle
bool is_game_start;
// is_all_clear:
//   Check to show "ALL LEVEL CLEAR!" in GameOver scene
//   true when all bricks in the last level is destroy
bool is_all_clear;

void deleteBricks() {
    for (int i = 0; i < n_bricks + n_barrier_bricks; i++)
        delete bricks[i];
    delete [] bricks;
}

void initBricksLevel(int level) {
    ItemBrick *item_brick;
    StoneBrick *stone_brick;

    if (level == 1) {
        n_bricks = 96;
        n_barrier_bricks = 0;
        for (int i = 0, x = 100, y = 120; i < n_bricks; i++) {
            if (i == 7) {
                item_brick = new ItemBrick();
                item_brick->setTexture(blue_item_brick_texture);
                item_brick->setItemType(BallsSpread);
                item_brick->setScore(10);
                bricks[i] = item_brick;
            } else if (i == 36) {
                item_brick = new ItemBrick();
                item_brick->setTexture(blue_item_brick_texture);
                item_brick->setItemType(MissileAmmo);
                item_brick->setScore(10);
                bricks[i] = item_brick;
            } else if (i == 81) {
                item_brick = new ItemBrick();
                item_brick->setTexture(blue_item_brick_texture);
                item_brick->setItemType(BallSpeedIncrease);
                item_brick->setScore(10);
                bricks[i] = item_brick;
            } else if (i % 5) {
                bricks[i] = new Brick();
                bricks[i]->setTexture(blue_brick_texture);
                bricks[i]->setScore(5);
            } else {
                stone_brick = new StoneBrick();
                stone_brick->setTexture(stone_brick_texture);
                stone_brick->setCrackTexture(crack_stone_brick_texture);
                stone_brick->setScore(20);
                bricks[i] = stone_brick;
            }

            bricks[i]->setPos(x, y);
            bricks[i]->setSize(50, 25);

            if (x > WINDOW_WIDTH - 200)
                x = 100, y += 25;
            else
                x += 50;
        }
    } else if (level == 2) {
        n_bricks = 48;
        for (int i = 0, x = 100, y = 110, line = 1; i < n_bricks; i++) {
            if (i == 1) {
                item_brick = new ItemBrick();
                item_brick->setTexture(yellow_item_brick_texture);
                item_brick->setItemType(BallSpeedIncrease);
                item_brick->setScore(10);
                bricks[i] = item_brick;
            } else if (i == 24) {
                item_brick = new ItemBrick();
                item_brick->setTexture(yellow_item_brick_texture);
                item_brick->setItemType(BallsSpread);
                item_brick->setScore(10);
                bricks[i] = item_brick;
            } else if (i == 39) {
                item_brick = new ItemBrick();
                item_brick->setTexture(red_item_brick_texture);
                item_brick->setItemType(MissileAmmo);
                item_brick->setScore(10);
                bricks[i] = item_brick;
            } else {
                bricks[i] = new Brick();
                if (line == 1 || line == 5)
                    bricks[i]->setTexture(yellow_brick_texture);
                else if (line == 2 || line == 4 || line == 6 || line == 8)
                    bricks[i]->setTexture(orange_brick_texture);
                else
                    bricks[i]->setTexture(red_brick_texture);
                bricks[i]->setScore(5);
            }

            bricks[i]->setPos(x, y);
            bricks[i]->setSize(50, 25);

            if (x >= WINDOW_WIDTH - 200)
                line++, x = line % 2 == 0 ? 150 : 100, y += 25;
            else
                x += 100;
        }
        n_barrier_bricks = 8;
        for (int i = n_bricks, x = 50, y = 360; i < n_bricks + n_barrier_bricks; i++, x += 50) {
            bricks[i] = new BarrierBrick();
            bricks[i]->setPos(x, y);
            bricks[i]->setSize(50, 25);
            bricks[i]->setTexture(barrier_brick_texture);
            if (((i - n_bricks) + 1) % 2 == 0)
                x += 50;
            if ((i - n_bricks) == 3)
                x += 150;
        }
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
    for (int i = 0; i < MAX_MISSILES; i++)
        if (missiles[i].isVisible())
            missiles[i].drawTexture();

    // Missile left icon
    cpDrawTexture(28, 656, missiles_left_icon_texture->width, missiles_left_icon_texture->height,
        missiles_left_icon_texture);

    // Missile left text
    cpDrawText(255, 255, 255, 230, 76, 651, to_string(missiles_left).c_str(), rsu_26_font, false);

    // Paddle
    paddle.drawTexture();

    // Ball
    for (int i = 0; i < MAX_BALLS; i++)
        if (balls[i].isOnScreen())
            balls[i].drawTexture();

    // Breakable Bricks (if brick still have durability then draw a brick)
    for (int i = 0; i < n_bricks; i++)
        if (bricks[i]->getDurability())
            bricks[i]->drawTexture();

    // Barrier Bricks
    for (int i = n_bricks; i < n_bricks + n_barrier_bricks; i++)
        bricks[i]->drawTexture();

    // In-game Frame
    cpDrawTexture(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, in_game_frame_texture);

    // Animate increase score text
    if (animate_score < score - 100 || (animate_score_tick && animate_score < score)) {
        animate_score++;
        animate_score_tick = 0;
    } else
        animate_score_tick++;
    drawScoreText(animate_score);

    // Balls left text
    cpDrawText(255, 255, 255, 216, 758, 43, to_string(balls_left).c_str(), rsu_30_font, true);

    // Release ball hint text
    if (!is_game_start)
        cpDrawText(255, 255, 255, 230, WINDOW_WIDTH / 2, 666, "กด Spacebar เพื่อปล่อยลูก", rsu_24_font, true);

    // Level badge
    if (level_badge_opacity > 0) {
        if (level == 1)
            cpDrawTextureAlpha(315, 335, level1_icon_texture->width, level1_icon_texture->height,
                            level1_icon_texture, level_badge_opacity);
        else if (level == 2)
            cpDrawTextureAlpha(306, 335, level2_icon_texture->width, level2_icon_texture->height,
                            level2_icon_texture, level_badge_opacity);
        level_badge_opacity -= 1.5;
    }

    // Animate dim scene screen at start of the scene
    if (screen_opacity > 0) {
        cpDrawTextureAlpha(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, black_bg_texture, screen_opacity);
        screen_opacity -= 4;
    } else {
        cpDrawTextureAlpha(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, black_bg_texture, 0);
    }
}

void spreadBalls() {
    float angle = 11.25;
    int n_spread_balls = 15;
    for (int i = 0; i < MAX_BALLS && n_spread_balls; i++) {
        if (!balls[i].isOnScreen()) {
            for (int n = 0; n < MAX_BALLS; n++)
                if (balls[n].isOnScreen()) {
                    balls[i].setPos(balls[n].getX(), balls[n].getY());
                    break;
                }
            balls[i].setVelX(ball_vel * sin(angle * M_PI / 180));
            balls[i].setVelY(ball_vel * cos(angle * M_PI / 180));
            balls[i].setIsOnScreen(true);
            n_balls++, n_spread_balls--;
            angle += 22.5;
            if (angle >= 360)
                angle = 11.25;
            else if (fmod(angle, 90) == 0)
                angle += 22.5;
        }
    }
}

// Called when an object collide w/ the brick
void handleBrickEvent(Brick *brick) {
    // Play hit sound
    cpPlaySound(hit_brick_sound);

    // Decrease durability by 1
    brick->decreaseDurability();

    // + Score when brick is break
    if (!brick->getDurability()) {
        score += brick->getScore();
        n_breaks++;
        // Brick Item Effect
        ItemBrick *item_brick = dynamic_cast<ItemBrick*>(brick);
        if (item_brick) {
            ItemType type = item_brick->getItemType();
            if (type == MissileAmmo) {
                missiles_left += 15;
            } else if (type == BallsSpread) {
                spreadBalls();
            } else if (type == BallSpeedIncrease) {
                ball_vel += 3;
                for (int i = 0; i < MAX_BALLS; i++) {
                    if (balls[i].isOnScreen()) {
                        if (balls[i].getVelX() < 0)
                            balls[i].setVelX(balls[i].getVelX() - 3);
                        else
                            balls[i].setVelX(balls[i].getVelX() + 3);
                        if (balls[i].getVelY() < 0)
                            balls[i].setVelY(balls[i].getVelY() - 3);
                        else
                            balls[i].setVelY(balls[i].getVelY() + 3);
                    }
                }
            }
        }
    }
}

void showInGameScene() {
    // For store event from PollEvent function
    Event event;
    // Regulating Frame rate
    const int FPS = 60;
    // Store tick time at start of the frame
    int start_frame_tick;
    // Set max ball angle when collide w/ paddle = 70 degee angle
    const float MAX_BOUNCE_ANGLE = 7 * M_PI / 18;
    // Default paddle movement speed
    const int PADDLE_SPEED = 10;
    // Temp vars for calculate angle
    float relative_intersect, normalized_relative_intersect;
    // For spacebar key hold detect
    bool is_spacebar_hold_down = false;
    // Missile fire delay
    int missile_tick = 0;
    // Store collision side
    CollisionSide side;
    // Store time for RNG
    time_t t;

    // Init RNG for start ball angle
    srand((unsigned) time(&t));

    // Init bricks
    bricks = new Brick*[MAX_BRICKS];

    // Set paddle texture
    paddle.setTexture(paddle_texture);

    // Init paddle position
    paddle.setX(WINDOW_WIDTH / 2 - paddle.getWidth() / 2);
    paddle.setY(WINDOW_HEIGHT - 80);

    // Reset paddle movement
    paddle.setVelX(0);

    // Prevent paddle get out of window (In-game Frame side border width = 9px)
    if (paddle.getX() < 10)
        paddle.setX(10);
    else if (paddle.getX() + paddle.getWidth() > WINDOW_WIDTH - 10)
        paddle.setX(WINDOW_WIDTH - paddle.getWidth() - 10);

    // Init balls
    for (int i = 0; i < MAX_BALLS; i++) {
        balls[i].setSize(20, 20);
        balls[i].setTexture(ball_texture);
    }

    // Init number of missiles left
    missiles_left = 10;

    // Init number of balls left
    balls_left = 2;

    // Init start score & reset animate score
    score = 0;
    animate_score = 0;

    // Init alpha value to animate screen dimming at start
    screen_opacity = 255;

    // reset tick for animate score text
    animate_score_tick = 0;

    for (level = 1; level <= 2; level++) {
        // Reset level badge opacity to 100%
        level_badge_opacity = 255;

        // Reset Missiles
        for (int i = 0; i < 30; i++)
            missiles[i].setVisible(false);

        // Reset game status (ball will stick w/ paddle)
        is_game_start = false;

        // Init/Reset ball speed
        ball_vel = 8;

        // Reset balls velocity in x/y pos & random start ball angle
        bounce_angle = fmod(rand(), MAX_BOUNCE_ANGLE * 2) - MAX_BOUNCE_ANGLE;
        for (int i = 0; i < MAX_BALLS; i++) {
            balls[i].setVelX(ball_vel * sin(bounce_angle));
            balls[i].setVelY(-ball_vel * cos(bounce_angle));
            balls[i].setIsOnScreen(false);
        }

        // Reset ball
        n_balls = 1;
        balls[0].setX(paddle.getX() + paddle.getWidth() / 2 - balls[0].getWidth() / 2);
        balls[0].setY(paddle.getY() - balls[0].getHeight() - 1);
        balls[0].setIsOnScreen(true);

        // Free old bricks
        if (level > 1)
            for (int i = 0; i < n_bricks + n_barrier_bricks; i++)
                delete bricks[i];

        // Init bricks for level 1
        initBricksLevel(level);

        // Reset brick break count
        n_breaks = 0;

        while (n_breaks < n_bricks) {
            start_frame_tick = SDL_GetTicks();

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
                if (event.type == KEYUP && event.key.keysym.sym == SDLK_ESCAPE) {
                    showPauseMenuScene();
                    if (quit || scene != InGame)
                        return;
                    // Reset paddle movement
                    paddle.setVelX(0);
                    break;
                }
                // Paddle Control
                if (event.type == KEYDOWN && event.key.keysym.sym == K_LEFT) {
                    paddle.setVelX(-PADDLE_SPEED);
                    break;
                } else if (event.type == KEYDOWN && event.key.keysym.sym == K_RIGHT) {
                    paddle.setVelX(PADDLE_SPEED);
                    break;
                } else if (event.type == KEYUP &&
                          (event.key.keysym.sym == K_LEFT || event.key.keysym.sym == K_RIGHT)) {
                    paddle.setVelX(0);
                    break;
                }
                if (is_game_start) {
                    // R Key to Spread balls Cheat
                    if (is_game_start && event.type == KEYDOWN && event.key.keysym.sym == SDLK_r) {
                        spreadBalls();
                        break;
                    }
                    // Spacebar key hold down detect
                    if (event.key.keysym.sym == SDLK_SPACE) {
                        if (is_game_start && event.type == KEYDOWN)
                            is_spacebar_hold_down = true;
                        else if (event.type == KEYUP)
                            is_spacebar_hold_down = false, missile_tick = 10;
                    }
                } else {
                    // Start the game (release ball) when user press spacebar
                    if (event.type == KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                        is_game_start = true;
                        break;
                    }
                }
            }

            if (is_game_start && is_spacebar_hold_down && missiles_left) {
                // To fire missiles once per 10 ticks (~0.16 second)
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
                                missiles[i].setX(paddle.getX() + paddle.getWidth() -
                                                 (missiles[i].getWidth() + 3));
                            missiles[i].setY(paddle.getY());
                            missiles[i].setTexture(missile_texture);
                            missiles[i].setVisible(true);
                            // Play missile launch sound
                            cpPlaySound(missile_sound);
                            amount++;
                            // Fire 2 missiles per 1 time
                            if (amount == 2) {
                                missiles_left--;
                                break;
                            }
                        }
                    }
                } else
                    missile_tick++;
            }

            for (int i = 0; i < MAX_MISSILES; i++) {
                if (missiles[i].isVisible()) {
                    // Missile Speed = 7
                    missiles[i].setY(missiles[i].getY() - 7);
                    // Reset Missile when get out of window
                    if (missiles[i].getY() + missiles[i].getHeight() < 64)
                        missiles[i].setVisible(false);
                    else {
                        // When missile hit a barrier brick
                        for (int n = n_bricks; n < n_bricks + n_barrier_bricks; n++)
                            if (missiles[i].isCollide(*bricks[n]))
                                missiles[i].setVisible(false);
                        // When missile hit a breakable brick
                        for (int n = 0; n < n_bricks; n++) {
                            if (bricks[n]->getDurability() && missiles[i].isCollide(*bricks[n])) {
                                missiles[i].setVisible(false);
                                handleBrickEvent(bricks[n]);
                                // Bonus score
                                score += 3;
                            }
                        }
                    }
                }
            }

            // Set paddle position related to left/right key hold
            paddle.move();

            // Prevent paddle get out of window (In-game Frame side border width = 10px)
            if (paddle.getX() < 10)
                paddle.setX(10);
            else if (paddle.getX() + paddle.getWidth() > WINDOW_WIDTH - 10)
                paddle.setX(WINDOW_WIDTH - paddle.getWidth() - 10);

            if (!is_game_start)
                // Set ball position to center of paddle
                balls[0].setX(paddle.getX() + paddle.getWidth() / 2 - balls[0].getWidth() / 2);
            else {
                // Set ball position related to ball x/y velocity
                for (int i = 0; i < MAX_BALLS; i++)
                    if (balls[i].isOnScreen())
                        balls[i].move();
            }

            for (int i = 0; i < MAX_BALLS; i++) {
                if (balls[i].isOnScreen()) {
                    // When Ball fall down
                    if (balls[i].getY() > WINDOW_HEIGHT) {
                        balls[i].setIsOnScreen(false);
                        n_balls--;
                    }

                    // Prevent ball get out of window (In-game Frame side border width = 9px, height = 63px)
                    // Left side of In-game frame
                    if (balls[i].getX() <= 9) {
                        balls[i].setX(10);
                        balls[i].invertVelX();
                    }
                    // Right side of In-game frame
                    if (balls[i].getX() + balls[i].getWidth() > WINDOW_WIDTH - 10) {
                        balls[i].setX(WINDOW_WIDTH - balls[i].getWidth() - 10);
                        balls[i].invertVelX();
                    }
                    // Top of In-game frame
                    if (balls[i].getY() <= 63) {
                        cpPlaySound(hit_top_sound);
                        balls[i].setY(64);
                        balls[i].invertVelY();
                    }

                    // Check all bricks
                    for (int n = 0; n < n_bricks + n_barrier_bricks; n++) {
                        // When ball hit a breakable brick
                        if (bricks[n]->getDurability() && (side = bricks[n]->isCollide(balls[i]))) {

                            handleBrickEvent(bricks[n]);

                            // Prevent ball get into brick
                            switch (side) {
                                case CollideTop:
                                    balls[i].setY(bricks[n]->getY() - balls[i].getHeight() - 1);
                                    // Set ball to go up only
                                    balls[i].setVelY(-fabs(balls[i].getVelY()));
                                    break;
                                case CollideBottom:
                                    balls[i].setY(bricks[n]->getY() + bricks[n]->getHeight() + 1);
                                    // Set ball to go down only
                                    balls[i].setVelY(fabs(balls[i].getVelY()));
                                    break;
                                case CollideLeft:
                                    balls[i].setX(bricks[n]->getX() - balls[i].getWidth() - 1);
                                    // Set ball to go only left side
                                    balls[i].setVelX(-fabs(balls[i].getVelX()));
                                    break;
                                case CollideRight:
                                    balls[i].setX(bricks[n]->getX() + bricks[n]->getWidth() + 1);
                                    // Set ball to go only right side
                                    balls[i].setVelX(fabs(balls[i].getVelX()));
                                    break;
                                // suppress compiler warning
                                default:
                                    break;
                            }
                        }
                    }

                    // When ball hit the paddle
                    if ((side = paddle.isCollide(balls[i]))) {
                        cpPlaySound(hit_paddle_sound);

                        // Set new ball angle related to collide position on paddle
                        relative_intersect = (balls[i].getX() + balls[i].getWidth() / 2) -
                                            (paddle.getX() + paddle.getWidth() / 2);
                        normalized_relative_intersect = relative_intersect / (paddle.getWidth() / 2);
                        bounce_angle = normalized_relative_intersect * MAX_BOUNCE_ANGLE;

                        balls[i].setVelX(ball_vel * sin(bounce_angle));
                        balls[i].setVelY(-ball_vel * cos(bounce_angle));

                        // Prevent ball get into paddle
                        switch (side) {
                            case CollideTop:
                                balls[i].setY(paddle.getY() - balls[i].getHeight() - 1);
                                break;
                            case CollideLeft:
                                balls[i].setX(paddle.getX() - balls[i].getWidth() - 1);
                                // Set ball to go only left side
                                balls[i].setVelX(-fabs(balls[i].getVelX()));
                                break;
                            case CollideRight:
                                balls[i].setX(paddle.getX() + paddle.getWidth() + 1);
                                // Set ball to go only right side
                                balls[i].setVelX(fabs(balls[i].getVelX()));
                                break;
                            // suppress compiler warning
                            default:
                                break;
                        }
                    }
                }
            }

            // No balls left on screen
            if (!n_balls) {
                // Go to GameOver scene when no balls left to play
                if (!balls_left) {
                    scene = GameOver;
                    return;
                }
                // Stop the game (make ball stick w/ paddle)
                is_game_start = false;
                // Decrease number of balls left by 1
                balls_left--;
                n_balls = 1;
                // Reset ball position
                balls[0].setIsOnScreen(true);
                balls[0].setX(paddle.getX() + paddle.getWidth() / 2 - balls[0].getWidth() / 2);
                balls[0].setY(paddle.getY() - balls[0].getHeight() - 1);
                // Reset ball velocity in x/y pos & random start ball angle
                ball_vel = 8;
                bounce_angle = bounce_angle = fmod(rand(), MAX_BOUNCE_ANGLE * 2) - MAX_BOUNCE_ANGLE;
                balls[0].setVelX(ball_vel * sin(bounce_angle));
                balls[0].setVelY(-ball_vel * cos(bounce_angle));
            }

            // Regulating frame rate (Prevent frame run too fast on some PC)
            if (1000 / FPS > SDL_GetTicks() - start_frame_tick)
                cpDelay(1000 / FPS - (SDL_GetTicks() - start_frame_tick));
        }
    }
    // If the last level is cleared
    scene = GameOver;
    is_all_clear = true;
}