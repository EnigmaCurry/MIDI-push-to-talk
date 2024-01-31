#include <MIDI.h>
#include <Keyboard.h>
#include <EEPROM.h>

MIDI_CREATE_DEFAULT_INSTANCE();

const int buttonMode = 2; // Mode button
const int buttonUp = 3; // Up button
const int buttonDown = 4; // Down button
const bool isMac = true; // Set this true for Mac, or false for PC.

int note = 45; // Default note value
int lastPotValue1 = -1; // Last value of the first potentiometer
int targetCCNumber = 66; // Replace with your specific CC number
int lastCCValue = -1; // To track the last CC value

enum MemoryRegister {
    NoteRegister,
    KeyRegister,
    CCRegister,
    NumberOfRegisters // Sentinel, always leave this as the last one.
};

// These are the modes of the device, toggled by D2
enum Mode {
    NormalMode,
    SetupNoteMode,
    SetupKeyMode,
    SetupCCMode,
    TestMode,
    NumberOfModes // Sentinel, always leave this as the last one.
};

enum KeyConfig {
  RightShiftKeyConfig,
  RightControlKeyConfig,
  NumberOfKeyConfigs // Sentinel
};

Mode currentMode = NormalMode;
KeyConfig key_config = RightShiftKeyConfig;
int key = KEY_RIGHT_SHIFT;
bool key_pressed = false;

void setup() {
    Serial.begin(9600);
    delay(5000);
    MIDI.begin(MIDI_CHANNEL_OMNI);
    Keyboard.begin();
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.setHandleControlChange(handleControlChange);
    pinMode(buttonMode, INPUT_PULLUP); // Set button pin as input with pull-up
    pinMode(buttonUp, INPUT_PULLUP); 
    pinMode(buttonDown, INPUT_PULLUP); 

    // Read the note value from EEPROM
    note = readIntFromEEPROM(NoteRegister);
    // Read the key config from the EEPROM
    key_config = static_cast<KeyConfig>(readIntFromEEPROM(KeyRegister));
    // Read the targetCCNumber from the EEPROM
    targetCCNumber = readIntFromEEPROM(CCRegister);
}

void loop() {
    MIDI.read();

    // Button press logic
    if (digitalRead(buttonMode) == LOW) {
        delay(50); // Debounce delay
        while (digitalRead(buttonMode) == LOW); // Wait for button release
        currentMode = static_cast<Mode>((currentMode + 1) % NumberOfModes); // Cycle through modes
        updateMode(); // Output the new mode
    }

    // Potentiometer handling based on the current mode
    switch (currentMode) {
        case SetupNoteMode:
            //handlePotentiometer(potPin1, lastPotValue1, 0, 127); // MIDI note range
            handleButtons();
            break;
        case SetupKeyMode:
            //handlePotentiometer(potPin1, lastPotValue1, 0, NumberOfKeyConfigs); // Keyboard config
            handleButtons();
            break;
        case SetupCCMode:
            handleButtons();
            break;
        case TestMode:
            break;
        default:
            break; // No action in NormalOperation mode
    }
}

void handleButtons() {
    if (digitalRead(buttonUp) == LOW) {
        delay(50);
        while(digitalRead(buttonUp) == LOW);
        switch (currentMode) {
        case SetupNoteMode:
            note = (note + 1) % 128;
            clearText();
            Keyboard.print("MIDI note trigger: ");
            Keyboard.print(note);
            break;
        case SetupKeyMode:
            incrementKeyConfig();
            break;
        case SetupCCMode:
            targetCCNumber = (targetCCNumber + 1) % 128;
            clearText();
            Keyboard.print("MIDI CC trigger: ");
            Keyboard.print(targetCCNumber);
            break;
        default:
            break; // No action
        }
    } else if (digitalRead(buttonDown) == LOW) {
        delay(50);
        while(digitalRead(buttonDown) == LOW);
        switch (currentMode) {
        case SetupNoteMode:
            note = (note - 1 + 128) % 128;
            clearText();
            Keyboard.print("MIDI note trigger: ");
            Keyboard.print(note);
            break;
        case SetupKeyMode:
            decrementKeyConfig();
            break;
        case SetupCCMode:
            targetCCNumber = (targetCCNumber - 1 + 128) % 128;
            clearText();
            Keyboard.print("MIDI CC trigger: ");
            Keyboard.print(targetCCNumber);
            break;
        default:
            break; // No action
        }
    }
}

void handleNoteOn(byte channel, byte currentNote, byte velocity) {
    if (currentMode == NormalMode){
      if (currentNote == note && note < 127) {
          Keyboard.press(key);
          key_pressed = true;
      }
    } else if (currentMode == TestMode) {
      Keyboard.print("Note ON: ");
      Keyboard.print(currentNote);
      Keyboard.print(" - Channel: ");
      Keyboard.print(channel);
      Keyboard.print("\n");
    }
}

void handleNoteOff(byte channel, byte currentNote, byte velocity) {
    if (currentMode == NormalMode) {
      if (currentNote == note && note < 127) {
         Keyboard.release(key);
         key_pressed = false;
      } 
    } else if (currentMode == TestMode) {
      Keyboard.print("Note OFF: ");
      Keyboard.print(currentNote);
      Keyboard.print(" - Channel: ");
      Keyboard.print(channel);
      Keyboard.print("\n");
    }
}

void handleControlChange(byte channel, byte number, byte value) {
    if (currentMode == NormalMode) {
        if (number == targetCCNumber && !key_pressed) {
            // Check if the CC value crosses the midpoint from low to high
            if (lastCCValue < 64 && value >= 64) {
                // CC value went from less than halfway to more than halfway
                Keyboard.press(key); // Trigger the key press
            } else if (lastCCValue >= 64 && value < 64) {
                // CC value went from more than halfway to less than halfway
                Keyboard.release(key); // Release the key press
            }
            lastCCValue = value; // Update the last CC value
        }
    } else if (currentMode == TestMode) {
      Keyboard.print("CC : ");
      Keyboard.print(number);
      Keyboard.print(" - Channel: ");
      Keyboard.print(channel);
      Keyboard.print(" value: ");
      Keyboard.print(value); 
      Keyboard.print("\n");
    }
}

int readPotentiometer(int pin, int minVal, int maxVal) {
    int value = analogRead(pin);
    return map(value, 0, 1023, minVal, maxVal);
}


void clearText() {
    if (isMac) {
        // Mac: Command (GUI) + A
        Keyboard.press(KEY_LEFT_GUI);
    } else {
        // PC: Ctrl + A
        Keyboard.press(KEY_LEFT_CTRL);
    }
    Keyboard.press('a');
    Keyboard.releaseAll();
    Keyboard.write(KEY_BACKSPACE);
}


void updateMode() {
    clearText();
    switch (currentMode) {
        case NormalMode:
            writeIntToEEPROM(NoteRegister, note);
            writeIntToEEPROM(KeyRegister, int(key_config));
            writeIntToEEPROM(CCRegister, targetCCNumber);
            Keyboard.print("Switched to Normal Mode.");
            break;
        case SetupNoteMode:
            Keyboard.print("MIDI note trigger: ");
            Keyboard.print(note);
            break;
        case SetupKeyMode:
            setKeyConfig();
            break;
        case SetupCCMode:
            Keyboard.print("MIDI CC trigger: ");
            Keyboard.print(targetCCNumber);
            break;
        case TestMode:
            Keyboard.print("TEST mode");
            break;
    }
}

void incrementKeyConfig() {
    // Increment the key_config
    if (key_config < NumberOfKeyConfigs - 1) {
        key_config = static_cast<KeyConfig>(static_cast<int>(key_config) + 1);
    } else {
        key_config = static_cast<KeyConfig>(RightShiftKeyConfig);
    }
    setKeyConfig();
}

void decrementKeyConfig() {
    // Decrement the key_config
    if (key_config > RightShiftKeyConfig) {
        key_config = static_cast<KeyConfig>(static_cast<int>(key_config) - 1);
    } else {
        key_config = static_cast<KeyConfig>(NumberOfKeyConfigs - 1);
    }
    setKeyConfig();
}

void setKeyConfig(){
  clearText();
  if (key_config == RightShiftKeyConfig) {
      key = KEY_RIGHT_SHIFT;
      Keyboard.print("Keyboard shortcut config: Right hand shift key.");
  } else if (key_config == RightControlKeyConfig) {
      key = KEY_RIGHT_CTRL;
      Keyboard.print("Keyboard shortcut config: Right hand control key."); 
  } else {
      key = KEY_RIGHT_SHIFT;
      Keyboard.print("Keyboard shortcut config: Undefined.");
  }
}

// Function to calculate the EEPROM address for a given register
int calculateEEPROMAddress(MemoryRegister reg) {
    return static_cast<int>(reg) * sizeof(int);
}

void writeIntToEEPROM(MemoryRegister reg, int value) {
    int address = calculateEEPROMAddress(reg);
    EEPROM.put(address, value);
    Serial.print("Writing to EEPROM at address ");
    Serial.print(address);
    Serial.print(": ");
    Serial.println(value);
}

int readIntFromEEPROM(MemoryRegister reg) {
    int value = 0;
    int address = calculateEEPROMAddress(reg);
    EEPROM.get(address, value);
    Serial.print("Reading from EEPROM at address ");
    Serial.print(address);
    Serial.print(": ");
    Serial.println(value);
    return value;
}

