#pragma once
#include "general.h"

Arduboy2 arduboy;
Sprites sprite;
Font3x5 font3x5 = Font3x5(8);
//BeepPin1 beep;
ArduboyTones sound(arduboy.audio.enabled);

uint8_t level=0;

bool facingUp;

// Declarations
void setLevel();
 
uint16_t hitCount = 0;
uint16_t enemyHit = 0;

// Life
uint8_t hearts_frame = 0;
uint8_t hide_w = 0;

// Title screen
uint8_t title_frame = 0;
uint8_t push_frame = 0;

uint8_t findUnusedBullet() {
  uint8_t bulletNum;
  for (bulletNum = 0; bulletNum < bullets; ++bulletNum) {
    if (bullet[bulletNum].x == bulletOff) {
      break; // unused bullet found
    }
  }
  return bulletNum;
}

//set sprite direction and animation
void anim() {
  if (arduboy.everyXFrames(10)) {
    if (title_frame < 2) {
      ++title_frame;
    } else {
      title_frame = 0;
    }

    if (push_frame < 1) {
      ++push_frame;
    } else {
      push_frame = 0;
    }

    if (hearts_frame < 1) {
      ++hearts_frame;
    }
    else {
      hearts_frame = 0;
    }
  }

  if (arduboy.pressed(UP_BUTTON) or arduboy.pressed(DOWN_BUTTON)
      or arduboy.pressed(LEFT_BUTTON)or arduboy.pressed(RIGHT_BUTTON)) {
          if(arduboy.everyXFrames(5)) {
              hero.frame++;
          }
          
          if (!facingUp && hero.frame >= 2) {
              hero.frame = 0;
          }

          if (facingUp && hero.frame >= 4) {
              hero.frame = 2;
          }
      }

    for (uint8_t i = 0; i < 10; i++) {
      if(arduboy.everyXFrames(8)) {
              cootie[i].frame++;
      }

      if (cootie[i].frame >=2) {
          cootie[i].frame=0;
      }
    }
}

// Controls
void input() {
  arduboy.pollButtons();
  anim();

  if (state == State::Title or state == State::GameOver) {
    if (arduboy.justPressed(A_BUTTON)) {
      //reset stats
      hero.life = 3;
      hero.x = 64;
      hero.y = 32;
      hitCount = 0;
      level = 1;

      //reset cooties
      for (uint8_t i = 0; i < targets; i++) {
        cootie[i].enable = false;
      }

      //change state
      state = State::Game;
    }
  }

  if (arduboy.pressed(UP_BUTTON)) {
      if (hero.y > 10) {
          hero.y--;
          hero.direction = Direction::Up;
          facingUp =  true;
      }
  }

  if (arduboy.pressed(DOWN_BUTTON)) {
      if (hero.y < (arduboy.height() - heroHeight)) {
          hero.y++;
          hero.direction = Direction::Down;
          facingUp = false;
      }
  }

  if (arduboy.pressed(LEFT_BUTTON)) {
      if (hero.x > 0) {
          hero.x--;
          hero.direction = Direction::Left;
      }
  }

  if (arduboy.pressed(RIGHT_BUTTON)) {
      if (hero.x < (arduboy.width() - heroWidth)) {
          hero.x++;
          hero.direction = Direction::Right;
      }
  }

  if (arduboy.pressed(A_BUTTON)) {
    if (waitCount == 0) {
      uint8_t bulletNum = findUnusedBullet();

      // If we get an unused bullet
      if (bulletNum != bullets) { 
        // Set the start position. (A positive X indicates bullet in use)
        bullet[bulletNum].x = hero.x;
        
        // Part way down the player
        bullet[bulletNum].y = hero.y + 3;

        // Uncomment this to fix the bug
        //bullet[bulletNum].direction = hero.direction; 
        sound.tone(NOTE_B5, 10);

        // Start the delay counter for the next bullet
        waitCount = bulletWait;
      }
    }
  }
}

void initBullets() {
  // Init all the bullets
  for (uint8_t bulletNum = 0; bulletNum < bullets; ++bulletNum) {
    bullet[bulletNum].x = bulletOff;
    bullet[bulletNum].width = bulletSize;
    bullet[bulletNum].height = bulletSize;
  }
}

void initEnemies() {
  // Init all the enemies
  for (uint8_t i = 0; i < targets; ++i) {
    cootie[i].width = 9;
    cootie[i].height = 9;
    cootie[i].enable = false;
  }
}

void moveBullets()
{
  for (uint8_t bulletNum = 0; bulletNum < bullets; ++bulletNum)
  {
    // This is the bug
    bullet[bulletNum].direction = hero.direction;

    // If bullet in use
    if (bullet[bulletNum].x != bulletOff)
    {
      if(bullet[bulletNum].direction == Direction::Right)
      {
        // Move bullet right
        ++bullet[bulletNum].x;
      }
      
      if(bullet[bulletNum].direction == Direction::Left)
      {
        // Move bullet left
        --bullet[bulletNum].x;
      }
      
      if(bullet[bulletNum].direction == Direction::Up)
      {
        // Move bullet up
        --bullet[bulletNum].y;
      }
      
      if(bullet[bulletNum].direction == Direction::Down)
      {
        // Move bullet down
        ++bullet[bulletNum].y;
      }
      
      // If off screen
      if (bullet[bulletNum].x >= arduboy.width() || (bullet[bulletNum].x == 0)
      || bullet[bulletNum].y >= arduboy.height() || (bullet[bulletNum].y == 0))
      { 
        // Set bullet as unused
        bullet[bulletNum].x = bulletOff;
      }
    }
  }
}

// Draw all the active bullets
void drawBullets() {
  for (uint8_t bulletNum = 0; bulletNum < bullets; ++bulletNum) {
    // If bullet in use
    if (bullet[bulletNum].x != bulletOff) {
      arduboy.fillRect(bullet[bulletNum].x, bullet[bulletNum].y, bulletSize, bulletSize);
    }
  }
}

void checkCollisions() {
  for (uint8_t i = 0; i < targets; i++) {
    if (cootie[i].life <= 0) {
          setLevel();
          cootie[i].enable = false;
          ++level;
        }

  // Check if enemy touch the hero
    if (arduboy.collide(cootie[i],hero) && hero.iframe == 0) {
      ++enemyHit;
      hero.iframe=50;

      sound.tone(NOTE_C5, 10);

      arduboy.invert(true);
      delay(20);
      arduboy.invert(false);

      if (hero.life>0) {
        --hero.life;

        if (hitCount>10) {
          hitCount = hitCount-10;
        }
      }
    }

  // Check if bullet hit the enemy
    for (uint8_t bulletNum = 0; bulletNum < bullets; ++bulletNum) {
      if (hero.iframe==0 && arduboy.collide(bullet[bulletNum], cootie[i])) {
        ++hitCount;
        if (cootie[i].life > 0) {
          --cootie[i].life;
        }
        
        // Set bullet as unused
        bullet[bulletNum].x = bulletOff;  
      }
    }
  }
}

// Check life and hide hearts
void setLife() {
  switch (hero.life)
  {
    case 3:
      hide_w = 0;
      break;

    case 2:
      hide_w = 9;
      break;

    case 1:
      hide_w = 17;
      break;

    case 0:
      state = State::GameOver;
      break;
  }
}