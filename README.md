# Discord push-to-talk with MIDI controller

This is an Arduino sketch to interface with MIDI and the PC keyboard,
to create a controller that allows holding a PC keyboard key
combination with the press of a MIDI keyboard key or CC.

The intended purpose of this project is enable Discord push-to-talk
via MIDI.

## Requirements

 * [Sparkfun MIDI shield](https://www.sparkfun.com/products/12898).
 * The [Arduino IDE](https://www.arduino.cc/en/software/) version 1. Version 2 is out, but it was not tested with it. Use version 1. 
 * Download the following libraries (search for them with the IDE library manager):
 
  * Keyboard (author: arduino)
  * MIDI (author: fortyseven effects)
 * Compile and upload the sketch to your arduino.

## Setup

Once you've assembled the components, and uploaded your sketch to the
device, you can perform the first time setup.

Open up a plain text editor and focus the input. Plug in the arduino
into the USB port.

Press the first button (labeled D2). With this button you can cycle through the various setup modes, which are:

 * `MIDI note trigger` - use this to set the MIDI note that triggers
   the key press.
 * `Keyboard shortcut config` - use this to cycle through the
   available shortcut configurations.
 * `MIDI CC trigger` - use this to set the MIDI CC number that
   triggers the key press.
 * `TEST mode` - use this to debug MIDI messages
 * `Switched to Normal Mode` - use this to enable the default mode and
   save the settings.
 
With the exception of the last two modes, you can use the other two
buttons (labeled D3 and D4) to change the setting for that mode.

`TEST` mode is a mode that prints any MIDI messages received, this is
useful for debugging your physical controller connection.

`Switched to Normal Mode` is the last option, and it wraps around
after that. `Switched to Normal Mode` is the normal default mode of
the device, and when you are done configuring things, make sure you
switch back to `Switched to Normal Mode`. When you enter normal mode,
all of the settings are stored in the EEPROM memory of the device, so
if you lose power the settings will be restored when it turns back on.

## MIDI notes and CC values and keyboard presses.

This controller supports both MIDI notes and CC values, and both can
be used at the same time to trigger the PC keyboard press.

 * If the CC value is more than 50% (>64) then hold the key press down.
 * If the CC value is less than 50% (<=64) then release the key press.
 * If the note value is pressed, regardless of the CC value, hold the
   key press down.
   
If you only ever want to use CC, then you can disable the note
feature, by going into the config and setting the MIDI note value to
127 (the note 127 is ignored by this controller).
