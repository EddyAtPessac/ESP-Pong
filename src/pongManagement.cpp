#include <Arduino.h>
#include "heltec.h"
#include "pongManagement.h"


// Bp status defined in main
bool isUpBp(void);
bool isDownBp(void);
bool isAutoMode(void);
extern SSD1306Wire  oLed;   // Defined in main


void drawCourt();

const unsigned long PADDLE_RATE = 33;
const unsigned long BALL_RATE = 16;
const uint8_t PADDLE_HEIGHT = 15; //24;
const int PLAYER_MOVE = 2;      

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

void pongSetup(void)
{
    unsigned long start = millis();
    oLed.clear();
    drawCourt();
    oLed.display();
    while(millis() - start < 2000); // Wait 2s before starting
    ball_update = millis();
    paddle_update = ball_update;
}

void displayScore(void) {
    static char strScore[8];
    oLed.setColor(BLACK);       // Erase old score
    oLed.drawString( HALF_SCREEN_WITH, 5, String(strScore));
    sprintf(strScore,"%02d-%02d", cpuScore, playerScore);
    oLed.setColor(WHITE);
    oLed.drawString( HALF_SCREEN_WITH, 5, String(strScore));
}

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

// Move padle for padMove pixel. Up is a negative value, Down is a positive value
void movePadle(int padX, int8_t &pPadY, int padHeight, int padMove) {
    //if (padMove == 0) return; // Nothing to do
    // Player paddle
    oLed.setColor(BLACK);
    oLed.drawVerticalLine(padX, pPadY, padHeight);
    // Calculate next position 
    if(padMove > 0 ) {  // Go Down to COURT_HEIGHT
        //Serial.printf("Down y=%d\n", pPadY);
        if( (pPadY + padHeight + padMove) < COURT_HEIGHT ) {
            pPadY += padMove;
        }
    }
    else if(padMove < 0) {          // Go up
        //Serial.printf("Up   y=%d\n", pPadY);
        if ( pPadY >  -padMove ) {
            pPadY += padMove;
        }
    }
    oLed.setColor(WHITE);
    oLed.drawVerticalLine(padX, pPadY, padHeight);
}

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

void pongLoop(void)
{

    bool update = false;
    unsigned long time = millis();
    int padleMove = 0;
    static bool up_state = false;
    static bool down_state = false;
    

    update |= ballUpdate();

    if(time > paddle_update) {
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


void drawCourt() {
    // The court
    oLed.setColor(WHITE);
    oLed.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    oLed.setPixel(ball_x, ball_y);  // The ball at the initial position 
    movePadle(CPU_X, cpu_y, PADDLE_HEIGHT, 0);
    movePadle(PLAYER_X, player_y, PADDLE_HEIGHT, 0);
    displayScore();
}
