#include <Arduino.h>
#include "heltec.h"
#include "pongManagement.h"


#define UP_BUTTON 12
#define DOWN_BUTTON 13

 SSD1306Wire  oLed = SSD1306Wire (0x3c, SDA_OLED, SCL_OLED, RST_OLED, GEOMETRY_128_64);

// SDA_OLED and other are defined in Pins_Arduino.h  (Heltec ESP32 board)
#define OLED_ARDS 0x3C
#define OLED_SDA SDA_OLED // 4
#define OLED_SCL SCL_OLED // 15
#define OLED_RESET RST_OLED  // 16
#define SMALL_FONT ArialMT_Plain_10
#define MEDIUM_FONT ArialMT_Plain_16
#define BIG_FONT ArialMT_Plain_24


  void  displayInit() {
    //Initialiser l'afficheur oled (sinon ne fonctionne pas)
    pinMode(16,OUTPUT);
    digitalWrite(OLED_RESET, LOW); // set  GPIO16 low to reset OLED
    delay(50); 
    digitalWrite(OLED_RESET, HIGH); // while OLED is running, must set GPIO16 to high

    // Initialiser interface de l'OLED
    oLed.init();

    //Inverser le sens d'affichage de l'oled. Mettre les 2 instructions sinon problème.
    oLed.flipScreenVertically();
    oLed.setFont(ArialMT_Plain_10);  // Disponibles: ArialMT_Plain_10, ArialMT_Plain_16, ArialMT_Plain_24
    //Aligner le texte
    oLed.setTextAlignment(TEXT_ALIGN_CENTER);
  }



long lastBpAction = 0;

// Return true if Up or Down button is pressed
bool isUpBp() {
  bool press = (digitalRead(UP_BUTTON) == LOW);
  if (press) lastBpAction = millis();
  return(press);
}

bool isDownBp() {
  bool press = (digitalRead(DOWN_BUTTON) == LOW);
  if (press) lastBpAction = millis();
  return(press);
}


// Return true if Up or Down are not pressed for a long time
// So, the autoMode could drive the padle at the place of the player
const long cstAutoModeTime = 15 * 1000L; // After this time, the auto mode is activated
bool isAutoMode() {
  bool autoMode = false;
  isUpBp();   // Check up and Down button
  isDownBp();
  if (millis() > lastBpAction + cstAutoModeTime ) {
    autoMode = true;
  }
  return(autoMode);
}




void setup() {
  // put your setup code here, to run once:
  displayInit();
  pinMode(UP_BUTTON, INPUT_PULLUP);
  pinMode(DOWN_BUTTON, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.printf("Starting\n");
  lastBpAction = millis();
  pongSetup();
}

void loop() {
  // put your main code here, to run repeatedly:
  pongLoop();
}