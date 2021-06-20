#include <Adafruit_CircuitPlayground.h>
#include <Keyboard.h>
#include "secrets.h"

#define STATE_INIT 0
#define STATE_LEFT_BUTTON_HELD 1
#define STATE_LEFT_BUTTON_RELEASED 2
#define STATE_RIGHT_BUTTON_HELD 3

int state = STATE_INIT;
int lastEventTime = 0;
int currentMacro = 0;
#define TIMEOUT 2000
#define ENTER_TIMEOUT 500


void setup() {
  //Serial.begin(9600);
  //Serial.println("started!");
  CircuitPlayground.begin();
  Keyboard.begin();
  startupAnimation();
}

void startupAnimation() {
    for (int i = 0; i < 10; i++) {
      CircuitPlayground.clearPixels();
      CircuitPlayground.setPixelColor(10-i, CircuitPlayground.colorWheel(i*25));
      delay(50);
  }
}

void leftButtonPressed(int n) {
  //Serial.println("left button pressed");
  CircuitPlayground.setPixelColor(10-n, CircuitPlayground.colorWheel(25*n));
  CircuitPlayground.speaker.enable(true);
  CircuitPlayground.playTone(500 + n * 500, 100);
  state = STATE_LEFT_BUTTON_HELD;
  lastEventTime = millis();
}

void playMacro(int n, bool withReturn) {
  Keyboard.print(macros[n-1]);
  if (withReturn)
    Keyboard.print("\n");
  //Serial.println("play macro");
  CircuitPlayground.speaker.enable(true);
  CircuitPlayground.playTone(1760, 100);
  CircuitPlayground.playTone(1760*2, 100);
  
  for (int i = 0; i < 5; i++) {
    CircuitPlayground.setPixelColor(10-n, 25*n);
    delay(100);
    CircuitPlayground.clearPixels();
    delay(100);
  }

  state = STATE_INIT;
}

void loop() {
  CircuitPlayground.speaker.enable(false);

  if (state == STATE_INIT) {
    currentMacro = 0;
    CircuitPlayground.clearPixels();
    if (CircuitPlayground.leftButton()) {
      leftButtonPressed(++currentMacro);
    }
  } else if (state == STATE_LEFT_BUTTON_HELD) {
    if (!CircuitPlayground.leftButton()) {
      //Serial.println("left button released");
      lastEventTime = millis();
      state = STATE_LEFT_BUTTON_RELEASED;
    }
  } else if (state == STATE_LEFT_BUTTON_RELEASED) {
    if (CircuitPlayground.rightButton()) {
      state = STATE_RIGHT_BUTTON_HELD;
      lastEventTime = millis();
    } else if (CircuitPlayground.leftButton()) {
      leftButtonPressed(++currentMacro);
    } else if (lastEventTime + TIMEOUT < millis()) {
      // timed out, go back to init
      state = STATE_INIT;
      //Serial.println("timeout");
    } 
  } else if (state == STATE_RIGHT_BUTTON_HELD) {
    if (!CircuitPlayground.rightButton()) {
      playMacro(currentMacro, false);
    } else if (lastEventTime + ENTER_TIMEOUT < millis()) {
      playMacro(currentMacro, true);
    }
  }
}
