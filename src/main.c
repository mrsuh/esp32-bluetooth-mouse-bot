#include <BleMouse.h>
#include <EEPROM.h>

BleMouse bleMouse("ESP32 Mouse", "ESP32", 100);

#define EEPROM_SIZE 512

#define PIN_LED1 17
#define PIN_LED2 5
#define PIN_LED3 18
#define PIN_LED4 19
#define PIN_LED5 16
#define PIN_LED6 2
#define PIN_LED7 4
#define PIN_LED8 15

#define PIN_BTN1 33
#define PIN_BTN2 35
#define PIN_BTN3 14
#define PIN_BTN4 27
#define PIN_BTN5 32
#define PIN_BTN6 34
#define PIN_BTN7 25
#define PIN_BTN8 26

#define PERIOD_MS 150

#define COMMANDS_LIST_SIZE 200

int commands[COMMANDS_LIST_SIZE];

void nullArray(int *list) {
    for (int i = 0; i < COMMANDS_LIST_SIZE; i++) {
        list[i] = 0;
    }
}

void loadCommands() {
    int eerpomIndex = 0;
    for (int i = 0; i < COMMANDS_LIST_SIZE; i += 2) {
        commands[i] = EEPROM.read(eerpomIndex);
        commands[i + 1] = EEPROM.read(eerpomIndex + 1) + EEPROM.read(eerpomIndex + 2) + EEPROM.read(eerpomIndex + 3);
        eerpomIndex += 4;
    }
}

void saveCommands() {
    for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();

    int eerpomIndex = 0;
    for (int i = 0; i < COMMANDS_LIST_SIZE; i += 2) {

        int counter = commands[i + 1];
        if(counter == 0) {
            continue;
        }

        EEPROM.write(eerpomIndex, commands[i]);

        for (int c = 1; c <= 3; c++) {
            EEPROM.write(eerpomIndex + c, 0);
        }

        for (int c = 1; c <= 3; c++) {
            counter -= 255;
            if (counter >= 0) {
                EEPROM.write(eerpomIndex + c, 255);
                continue;
            }

            if (counter < 0) {
                EEPROM.write(eerpomIndex + c, 255 + counter);
                break;
            }
        }

        eerpomIndex += 4;
    }
    EEPROM.commit();
}

void moveMouse(int button) {

    switch(button) {
    case 1://scroll up
        bleMouse.move(0, 0, -50);
        break;
    case 2:// up
        bleMouse.move(0, -10, 0);
        break;
    case 3://scroll down
        bleMouse.move(0, 0, 50);
        break;
    case 5://left
        bleMouse.move(-10, 0, 0);
        break;
    case 6://down
        bleMouse.move(0, 10, 0);
        break;
    case 7://right
        bleMouse.move(10, 0, 0);
        break;
    case 8://click
        bleMouse.click();
        break;
    }
}

int getPressedButton() {
    int value = 0;
    int button = 0;

    value = digitalRead(PIN_BTN1);
    if (value == HIGH) {
        button = 1;
    }

    value = digitalRead(PIN_BTN2);
    if (value == HIGH) {
        button = 2;
    }

    value = digitalRead(PIN_BTN3);
    if (value == HIGH) {
        button = 3;
    }

    value = digitalRead(PIN_BTN4);
    if (value == HIGH) {
        button = 4;
    }

    value = digitalRead(PIN_BTN5);
    if (value == HIGH) {
        button = 5;
    }

    value = digitalRead(PIN_BTN6);
    if (value == HIGH) {
        button = 6;
    }

    value = digitalRead(PIN_BTN7);
    if (value == HIGH) {
        button = 7;
    }

    value = digitalRead(PIN_BTN8);
    if (value == HIGH) {
        button = 8;
    }

    return button;
}

void switchLed(int led, int value) {

    int pin = 0;
    switch(led) {
    case 1:
        pin = PIN_LED1;
        break;
    case 2:
        pin = PIN_LED2;
        break;
    case 3:
        pin = PIN_LED3;
        break;
    case 4:
        pin = PIN_LED4;
        break;
    case 5:
        pin = PIN_LED5;
        break;
    case 6:
        pin = PIN_LED6;
        break;
    case 7:
        pin = PIN_LED7;
        break;
    case 8:
        pin = PIN_LED8;
        break;
    }

    digitalWrite(pin, value);
}

void printCommands() {
    Serial.print("Commands: ");
    for (int i = 0; i < COMMANDS_LIST_SIZE; i += 2) {

        if (commands[i] == 0) {
            break;
        }
        char* command = (char*)malloc(sizeof(char) * 20);
        switch(commands[i]) {
        case 1:
            command = "scroll-up";
            break;
        case 2:
            command = "up";
            break;
        case 3:
            command = "scroll-down";
            break;
        case 5:
            command = "left";
            break;
        case 6:
            command = "down";
            break;
        case 7:
            command = "right";
            break;
        case 8:
            command = "click";
            break;
        case 9:
            command = "wait";
            break;
        }

        Serial.print(command);
        Serial.print(":");
        Serial.print(commands[i + 1]);
        Serial.print(" ");
    }
    Serial.println();
}

int pressedButton = 0;
int pressedButtonCounter = 0;
int pressedButtonTime = 0;

int buttonIndex = 0;

bool recordModeInited = false;

#define STATE_INIT 1
#define STATE_NOT_CONNECTED 2
#define STATE_CONNECTED 3
#define STATE_RECORD 4
#define STATE_PLAY 5

int state = STATE_NOT_CONNECTED;
int stateChangedAt = 0;

int lastButton = 0;
int lastButtonChangedAt = 0;


void setup() {

    pinMode(PIN_LED1, OUTPUT);
    pinMode(PIN_LED2, OUTPUT);
    pinMode(PIN_LED3, OUTPUT);
    pinMode(PIN_LED4, OUTPUT);
    pinMode(PIN_LED5, OUTPUT);
    pinMode(PIN_LED6, OUTPUT);
    pinMode(PIN_LED7, OUTPUT);
    pinMode(PIN_LED8, OUTPUT);

    pinMode(PIN_BTN1, INPUT);
    pinMode(PIN_BTN2, INPUT);
    pinMode(PIN_BTN3, INPUT);
    pinMode(PIN_BTN4, INPUT);
    pinMode(PIN_BTN5, INPUT);
    pinMode(PIN_BTN6, INPUT);
    pinMode(PIN_BTN7, INPUT);
    pinMode(PIN_BTN8, INPUT);

    digitalWrite(PIN_LED1, LOW);
    digitalWrite(PIN_LED2, LOW);
    digitalWrite(PIN_LED3, LOW);
    digitalWrite(PIN_LED4, LOW);
    digitalWrite(PIN_LED5, LOW);
    digitalWrite(PIN_LED6, LOW);
    digitalWrite(PIN_LED7, LOW);
    digitalWrite(PIN_LED8, LOW);

    Serial.begin(115200);
    Serial.println("\nController enabled");

    EEPROM.begin(EEPROM_SIZE);

    nullArray(commands);

    bleMouse.begin();
}

void loop() {

    int time = millis();

    if (!bleMouse.isConnected()) {

        if (state != STATE_NOT_CONNECTED) {
            state = STATE_NOT_CONNECTED;
            stateChangedAt = time;
            Serial.println("State: not-connected");

            bleMouse.end();
            delay(1000);
            bleMouse.begin();
            return;
        }

        switchLed(1, HIGH);
        switchLed(2, HIGH);
        switchLed(3, HIGH);
        switchLed(4, HIGH);
        delay(500);
        switchLed(1, LOW);
        switchLed(2, LOW);
        switchLed(3, LOW);
        switchLed(4, LOW);
        delay(500);
        return;
    }

    if (state == STATE_NOT_CONNECTED) {
        state = STATE_CONNECTED;
        stateChangedAt = time;
        Serial.println("State: connected");
        return;
    }

    int button = getPressedButton();

    if (button == 4 && state == STATE_CONNECTED && (time - stateChangedAt > 500)) {
        state = STATE_RECORD;
        stateChangedAt = time;
        Serial.println("State: record");
        switchLed(4, HIGH);
        nullArray(commands);
        return;
    }

    if (button == 4 && state == STATE_RECORD && (time - stateChangedAt > 500)) {
        state = STATE_CONNECTED;
        stateChangedAt = time;

        switchLed(4, LOW);
        printCommands();
        saveCommands();
        Serial.println("Commands saved to persistent memory");
        Serial.println("State: connected");
        return;
    }

    if (button == 8 && state == STATE_CONNECTED) {
        state = STATE_PLAY;
        stateChangedAt = time;
        Serial.println("State: play");

        nullArray(commands);
        loadCommands();
        Serial.println("Commands loaded from persistent memory");
        printCommands();
        return;
    }

    if (state == STATE_CONNECTED && button > 0 && button != 4 && button != 8) {
        switchLed(button, HIGH);
        moveMouse(button);
        delay(PERIOD_MS);
        switchLed(button, LOW);
        return;
    }

    if (state == STATE_PLAY) {
        for (int i = 0; i < COMMANDS_LIST_SIZE; i += 2) {
            int buttonNumber = commands[i];

            if (buttonNumber == 0) {
                break;
            }

            switchLed(buttonNumber, HIGH);
            for (int c = 0; c < commands[i + 1]; c++) {
                moveMouse(buttonNumber);
                delay(PERIOD_MS);
            }
            switchLed(buttonNumber, LOW);
        }
        return;
    }

    if (state == STATE_RECORD) {

        if (!recordModeInited) {
            pressedButtonTime = time;
            recordModeInited = true;
        }

        if(button == 4) {
            return;
        }

        if (button != 0 && pressedButton == 0) {  // start button press

            commands[buttonIndex] = 9;
            commands[buttonIndex + 1] = pressedButtonCounter;

            buttonIndex += 2;

            pressedButton = button;
            pressedButtonTime = time;
            pressedButtonCounter = 0;

            commands[buttonIndex] = pressedButton;
            commands[buttonIndex + 1] = 0;
            switchLed(pressedButton, HIGH);
        }

        if (button == 0 && pressedButton != 0) {  // end button press
            commands[buttonIndex + 1] = pressedButtonCounter;
            switchLed(pressedButton, LOW);

            pressedButton = 0;
            pressedButtonTime = time;
            pressedButtonCounter = 0;

            buttonIndex += 2;
        }

        int counter = (time - pressedButtonTime) / PERIOD_MS;
        if(button == 8) {
            counter = 1;
        }

        if (counter > pressedButtonCounter) {
            pressedButtonCounter = counter;
            moveMouse(button);
        }
    }
}
