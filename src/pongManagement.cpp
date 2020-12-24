#include <Arduino.h>
#include "heltec.h"
#include "pongManagement.h"


// Bp status defined in main
bool isUpBp(void);
bool isDownBp(void);
bool isAutoMode(void);
extern SSD1306Wire  oLed;   // Defined in main


const unsigned long PADDLE_RATE = 33;
const unsigned long BALL_RATE = 16;
const uint8_t PADDLE_HEIGHT = 15; //24;
const int PLAYER_MOVE = 2;              // The player is a cheater: he's move 2x faster than the Cpu

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define HALF_SCREEN_WITH (SCREEN_WIDTH>>1)
#define COURT_HEIGHT SCREEN_HEIGHT-1  // Max y coordonate


uint8_t ball_x = 64, ball_y = 32;
uint8_t ball_dir_x = 1, ball_dir_y = 1;
unsigned long ball_update;

unsigned long paddle_update;
const uint8_t CPU_X = 12;
int8_t cpu_y = 16;

const uint8_t PLAYER_X = 115;
int8_t player_y = 16;

int playerScore = 0;
int cpuScore  = 0;

void drawCourt();


// Initialise timers and display for the game
void pongSetup(void)
{
    unsigned long start = millis();
    oLed.clear();
    drawCourt();
    oLed.display();
    // Show the court 2 seconds before starting
    while(millis() - start < 2000); 
    ball_update = millis();
    paddle_update = ball_update;
}

// Update the score on the oLed, with the values of cpuScore and playerScore
void displayScore(void) {
    static char strScore[8];    // Keep the old string in memory to rewrite it in black (to erase the old score)
    oLed.setColor(BLACK);       // Erase old score
    oLed.drawString( HALF_SCREEN_WITH, 5, String(strScore));
    sprintf(strScore,"%02d-%02d", cpuScore, playerScore);
    oLed.setColor(WHITE);       // Display new score
    oLed.drawString( HALF_SCREEN_WITH, 5, String(strScore));
}

// Calculate the next position of the ball. Check if it touch another thing 
// and update the score if it hurt the vertical wall
bool ballUpdate(void) {
    bool update = false;
    if(millis() > ball_update) {
        uint8_t new_x = ball_x + ball_dir_x;
        uint8_t new_y = ball_y + ball_dir_y;

        // Check if we hit the vertical walls
        if(new_x == 0 || new_x == 127) {
            if (new_x == 127) {
                cpuScore += 1;
            }
            else {
                playerScore += 1;
            }
            if (cpuScore > 99 || playerScore > 99) {
                cpuScore = playerScore = 0;
            }
            displayScore();

            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;
        }

        // Check if we hit the horizontal walls.
        if(new_y == 0 || new_y == 63) {
            ball_dir_y = -ball_dir_y;
            new_y += ball_dir_y + ball_dir_y;
        }

        // Check if we hit the CPU paddle
        if(new_x == CPU_X && new_y >= cpu_y && new_y <= cpu_y + PADDLE_HEIGHT) {
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;
        }

        // Check if we hit the player paddle
        if(new_x == PLAYER_X
           && new_y >= player_y
           && new_y <= player_y + PADDLE_HEIGHT)
        {
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;
        }

        oLed.setColor(INVERSE);
        oLed.setPixel(ball_x, ball_y);
        oLed.setColor(INVERSE);
        oLed.setPixel(new_x, new_y);
        ball_x = new_x;
        ball_y = new_y;

        ball_update += BALL_RATE;

        update = true;
    }
    return(update);
}

// Move and display padle for padMove pixel. Up is a negative value, Down is a positive value
// pPadY is updated with the new Y value.
void movePadle(int padX, int8_t &pPadY, int padHeight, int padMove) {
    //if (padMove == 0) return; // Nothing to do
    // Player paddle
    oLed.setColor(BLACK);
    oLed.drawVerticalLine(padX, pPadY, padHeight);
    // Calculate next position 
    if(padMove > 0 ) {  // Go Down to COURT_HEIGHT
        if( (pPadY + padHeight + padMove) < COURT_HEIGHT ) {
            pPadY += padMove;
        }
    }
    else if(padMove < 0) {          // Go up
        if ( pPadY >  -padMove ) {
            pPadY += padMove;
        }
    }
    oLed.setColor(WHITE);
    oLed.drawVerticalLine(padX, pPadY, padHeight);
}

// Return the desired moving to make padle follow the Y position of the ball
int8_t moveYToBall(int8_t yPadle) {
    const uint8_t half_paddle = PADDLE_HEIGHT >> 1;
    int8_t move = 0;
    if(yPadle + half_paddle > ball_y) {
        move = -1;
    }
    if(yPadle + half_paddle < ball_y) {
        move = 1;
    }
    return(move);
}

// Main loop of pong: update ball and padles positions
void pongLoop(void)
{

    bool update = false;
    unsigned long time = millis();
    int padleMove = 0;
    static bool up_state = false;
    static bool down_state = false;
    

    update |= ballUpdate();     // Move the ball if it is time

    if(time > paddle_update) {      // if it is time to move the padles
        paddle_update += PADDLE_RATE;
        update = true;

        // CPU paddle
        padleMove = moveYToBall(cpu_y);
        movePadle(CPU_X, cpu_y, PADDLE_HEIGHT, padleMove);

        // Player move 
        padleMove =0;
        up_state = isUpBp();
        down_state = isDownBp();
        if ( ! isAutoMode()) {
            if (up_state) padleMove = 1;
            if (down_state) padleMove = -1;
        }
        else {
            padleMove = moveYToBall(player_y);
        }
        padleMove *= PLAYER_MOVE; // This player could move faster 
        movePadle(PLAYER_X, player_y, PADDLE_HEIGHT, padleMove);
    }

    if(update)
        oLed.display();


}

// Initial display of the game. 
void drawCourt() {
    // The court
    oLed.setColor(WHITE);
    oLed.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    oLed.setPixel(ball_x, ball_y);  // The ball at the initial position 
    movePadle(CPU_X, cpu_y, PADDLE_HEIGHT, 0);  // Display the padle without move
    movePadle(PLAYER_X, player_y, PADDLE_HEIGHT, 0);  // same thing
    displayScore(); 
}
