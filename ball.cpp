#include "headers/ball.h"

Ball::Ball(float x, float y, float width, float height, float velX, float velY) :
    Object(x, y, width, height), velX_(velX), velY_(velY) {
        isOnScreen_ = true;
    }

void Ball::setVelX(float velX) {
    velX_ = velX;
}

void Ball::setVelY(float velY) {
    velY_ = velY;
}

void Ball::setIsOnScreen(bool isOnScreen) {
    isOnScreen_ = isOnScreen;
}

float Ball::getVelX() {
    return velX_;
}

float Ball::getVelY() {
    return velY_;
}

bool Ball::isOnScreen() {
    return isOnScreen_;
}