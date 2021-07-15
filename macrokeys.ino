#include <Fsm.h>
#include <Adafruit_CircuitPlayground.h>
#include <Keyboard.h>
#include "secrets.h"

#define LEFT_BUTTON 1
#define RIGHT_BUTTON 2
#define LONG_LEFT_BUTTON 3
#define LONG_RIGHT_BUTTON 4
#define RELEASE_LEFT_BUTTON 5
#define RELEASE_RIGHT_BUTTON 6

#define LONG_PRESS_TIME 500
#define TIMEOUT 5000

#define MUTE_BUTTON RIGHT_BUTTON
#define RELEASE_MUTE_BUTTON RELEASE_RIGHT_BUTTON
#define BANK_BUTTON LEFT_BUTTON
#define RELEASE_BANK_BUTTON RELEASE_LEFT_BUTTON
#define MACRO_BUTTON RIGHT_BUTTON
#define RELEASE_MACRO_BUTTON RELEASE_RIGHT_BUTTON
#define ISSUE_BUTTON LEFT_BUTTON
#define RELEASE_ISSUE_BUTTON RELEASE_LEFT_BUTTON
#define LONG_ISSUE_BUTTON LONG_LEFT_BUTTON

State state_idle(&on_init, &check_buttons, NULL);

State state_toggle_beep(NULL, &check_buttons, NULL);

State state_bank_button_down(&play_beep, &check_buttons, NULL);
State state_bank_button_up(NULL, &check_buttons, NULL);

State state_macro_button_down(&play_beep, &check_buttons, NULL);
State state_macro_button_up(NULL, &check_buttons, NULL);

State state_issue_button_down(&play_beep, &check_buttons, NULL);

Fsm fsm(&state_idle);

int currentMacro = 0;
int currentBank = 0;
bool leftButtonHeld = true;
bool rightButtonHeld = true;
bool mute = false;

void on_init() {
    CircuitPlayground.speaker.enable(false);
    startupAnimation();
}

void check_buttons() {
    if (CircuitPlayground.leftButton()) {
        if (!leftButtonHeld) {
            fsm.trigger(LEFT_BUTTON);
            leftButtonHeld = true;
            return;
        }
    } else {
        if (leftButtonHeld) {
            fsm.trigger(RELEASE_LEFT_BUTTON);
        }
        leftButtonHeld = false;
    }

    if (CircuitPlayground.rightButton()) {
        if (!rightButtonHeld) {
            fsm.trigger(RIGHT_BUTTON);
            rightButtonHeld = true;
            return;
        }
    } else {
        if (rightButtonHeld) {
            fsm.trigger(RELEASE_RIGHT_BUTTON);
        }
        rightButtonHeld = false;
    }
}

void first_bank() {
    currentBank = 0;
    single_light(0);
}

void next_bank() {
    currentBank++;
    currentBank %= NUM_BANKS;
    single_light(currentBank);
}

void first_macro() {
    currentMacro = 0;
    single_light(9);
}

void next_macro() {
    currentMacro++;
    currentMacro %= NUM_MACROS;
    single_light(9-currentMacro);
}

void clear_lights() {
    CircuitPlayground.clearPixels();
}

void single_light(int n) {
    clear_lights();
    CircuitPlayground.setPixelColor(n, CircuitPlayground.colorWheel(n * 25));
}

void toggle_beep() {
    mute ^= true;
    play_beep();
}

void setup() {
//    Serial.begin(9600);
//    Serial.println("started!");
    CircuitPlayground.begin();
    Keyboard.begin();
    startupAnimation();

    // from idle state
    fsm.add_transition(&state_idle, &state_bank_button_down, BANK_BUTTON, &first_bank);
    fsm.add_transition(&state_idle, &state_toggle_beep, MUTE_BUTTON, &toggle_beep);

    fsm.add_transition(&state_toggle_beep, &state_idle, RELEASE_MUTE_BUTTON, NULL);

    // from bank button down state
    fsm.add_transition(&state_bank_button_down, &state_bank_button_up, RELEASE_BANK_BUTTON, NULL);

    // from bank button up state
    fsm.add_transition(&state_bank_button_up, &state_bank_button_down, BANK_BUTTON, &next_bank);
    fsm.add_transition(&state_bank_button_up, &state_macro_button_down, MACRO_BUTTON, &first_macro);
    fsm.add_timed_transition(&state_bank_button_up, &state_idle, TIMEOUT, NULL);

    // from macro button down state
    fsm.add_transition(&state_macro_button_down, &state_macro_button_up, RELEASE_MACRO_BUTTON, NULL);

    // from macro button up state
    fsm.add_transition(&state_macro_button_up, &state_macro_button_down, MACRO_BUTTON, &next_macro);
    fsm.add_transition(&state_macro_button_up, &state_issue_button_down, ISSUE_BUTTON, NULL);
    fsm.add_timed_transition(&state_macro_button_up, &state_idle, TIMEOUT, NULL);

    // from issue button down state
    fsm.add_transition(&state_issue_button_down, &state_idle, RELEASE_ISSUE_BUTTON, &play_current_macro);
    fsm.add_timed_transition(&state_issue_button_down, &state_idle, LONG_PRESS_TIME, &play_current_macro_with_cr);
}

void startupAnimation() {
    for (int i = 0; i < 10; i++) {
        single_light(i);
        CircuitPlayground.clearPixels();
        delay(50);
    }
}

void play_beep() {
    if (mute)
        return;
    CircuitPlayground.speaker.enable(true);
    CircuitPlayground.playTone(1760, 100);
}

void play_confirmation_beep() {
    if (mute)
        return;
    CircuitPlayground.speaker.enable(true);
    CircuitPlayground.playTone(1760, 100);
    CircuitPlayground.playTone(1760 * 2, 100);
}

void play_current_macro() {
    play_macro(currentBank, currentMacro, false);
}

void play_current_macro_with_cr() {
    play_macro(currentBank, currentMacro, true);
}

void play_macro(int bank, int macro, bool withReturn) {
    Keyboard.print(macros[bank][macro]);
    if (withReturn)
        Keyboard.print("\n");
    //Serial.println("play macro");
    play_confirmation_beep();

    for (int i = 0; i < 5; i++) {
        single_light(macro);
        delay(100);
        single_light(9 - bank);
        delay(100);
    }
}

void loop() {
    fsm.run_machine();
}
