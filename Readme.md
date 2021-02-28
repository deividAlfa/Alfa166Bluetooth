## Alfa romeo 166 tape emulator (aux audio enabler) and bluetooth interface bridge 

<!-- MarkdownTOC -->

* [Project description](#description)
* [How to use](#use)
* [Firmware](#firmware)
* [Requirements](#requirements)
* [Connection diagram](#connection)

<!-- /MarkdownTOC -->

Video:
https://youtu.be/hq1ijzfhlmo

<a id="description"></a>
## Project description
The cassette (or tape) is nowadays something extinct and it's presence has no useful value.<br>
The Alfa Romeo 166 ships with the ICS solution (Integrated Control System), which is a Siemens IDIS / Becker BE2569/2580/2582 (Also used in Porsche PCM1).<br>
In embeds a tape slot and a external CD changer. Actually, bluetooth is almost mandatory to provide a complete experience.<br>
Since the tape is not going to be used, it's a potential audio input that could be used for something else.<br>

This firmware is the result of the reverse engineering of the tape mechanism: it emulates the tape presence, its states and sensors.<br>
It's completely aware of the actual state, and can extract useful data from it, like sending pulses to a bluetooth module buttons.<br>
When fast forward / fast rewind is selected, the firmware interprets this and generates a pulse to  NEXT or PREVIOUS track button.<br>
Also, it will simulate the end of the tape instantly, and force the tape returning to play mode.<br>
The tape controller returns to play state in approximately 1.2 seconds after the tape is simulated to reach its end.<br>
If the user quickly pushes the same button again, the controller returns to play mode before this time has elapsed.<br> 
This premature return to play state is detected, and an aditional pulse is generated for last button. So the tracks can be skipped pretty quick.<br>

The tape controller will detect a jam in the tape mechanism if more than 3 tracks are skipped quickly, so, to avoid that, an adittional 1.2 second delay is added after the 3rd fast skip.<br>
This delay is only needed if the tape doesn't play for at least 2 seconds between a track change.<br>
You will notice that after the 3rd fast skip, the next one delays a bit more.<br>
The counter is resetted after the delay or 2 seconds in play state.<br>

While in play mode, if the user pushes the "1-2" button (change tape play direction), this is detected and the CALL output is activated.<br>
This is useful if your bluetooth module has hands-free function and button input to pick up or hang up a call.<br>

To provide the best compatibility, a polarity input is provided. Leaving it open will set the polarity "Normally low", closing it will set it "Normally high".<br>
Due the inmense number of bluetooth modules, working at different voltages (5, 3.3, 1.8V), the outputs that handle the pulses for the buttons are set as open-drain (They only pull to ground, but don't put voltage).<br>
You will need to connect pull-up resistors to the working voltage of the bluetooth module. That way, any voltage level is compatible. STM32 handles max 5V, don't exceed that!

To debug the state of the board, the led is used.<br>
- Slow, heart beat-like blinks: The board is working but tape not active<br>
- Fast blinking: The tape is in play mode.
- Steady lit: The tape is in fast rewind / fast forward mode.


The firmware is implemented in a cheap STM32F103 "blue pill" board ($3).<br>
The code doesn't use any special peripheral and can be very easily ported to other devices, all needed are some GPIOs and a timer to provide timing control.

**As the ICS mutes the audio for ~3 seconds when skipping tracks, the songs would lose that time.<br>
The last version also modifies the circuit for play/pause control. Now it puts the bluetooth in pause for that time, so the songs start correctly.**

<a id="use"></a>
## How to use

- Connect to the bluetooth module and manually start playing in the phone controls.<br>
- Select Tape playback in the ICS controls.<br>
- Now the music should be playing. Use the steering wheel or ICS controls to skip tracks.<br>

<a id="firmware"></a>
## Firmware

You can download already compiled binaries in the Release folder (tape.bin file)<br>
If you want to compile your own:
- Download STM32 Cube IDE
- Clone or download the code
- Open STM32 Cube IDE, import existing project and select the folder where the code is.<br>
- 
  It should recognize it and be ready for compiling or modifying for your own needs.<br>  
  Check "tape.h" and "tape.c", the code is very well commented and readable. <br>

<a id="Requirements"></a>
## Requirements

You will need, at least:<br>
- Soldering iron
- Resistors:
  - 2x 470ohm  
  - 4x 1K  
  - 1x 6K8  
  - 1x 2K2  
  - 5x 10K...100K (any value will be ok)    
- STM32 "blue pill" board with STM32F103C8 / STM32F103CB 
- Bluetooth module
- Some wire
- Steady hand to not mess things up!
- I strongly recommend applying some glue/silicone to the soldered wires.<br>
  The car vibrations can break them and cause serious damage!<br>  
  I call the experience, 2 years ago, while fitting a previous bluetooth module, a 12V wire splitted, touched the 5V supply and blowed the whole board!<br>  
  
<a id="Connection"></a>
## Connection diagram

(Click for full size picture)

The 5V and 8.5V points are only active when the ICS is on, so there's no need to worry about the battery draining overnight!<br>
8.5V were more than enough for my bluetooth module, althought it could handle 12V. Much more convenient!<br> 
 ![IMAGE](https://raw.githubusercontent.com/deividAlfa/Alfa166Bluetooth/master/Pictures/Connection.jpg)
 
 
 This was my approach (Adding a physical button to reset the pairing list, in case the module goes mad :) ):<br>
 ![IMAGE](https://raw.githubusercontent.com/deividAlfa/Alfa166Bluetooth/master/Pictures/Board.jpg)
