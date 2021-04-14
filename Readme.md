## Alfa romeo 166 tape emulator (aux audio enabler) and bluetooth interface bridge 

<!-- MarkdownTOC -->

* [Project description](#description)
* [How to use](#use)
* [Limitations](#limitations)
* [Firmware](#firmware)
* [Requirements](#requirements)
* [Connection diagram](#connection)

<!-- /MarkdownTOC -->

**Latest update fixes compatibility issues with some phones!**

Video:
https://youtu.be/hq1ijzfhlmo

<a id="description"></a>
## Project description
The cassette (or tape) is nowadays something extinct and it's presence has no useful value.<br>
The Alfa Romeo 166 ships with the ICS solution (Integrated Control System), which is a Siemens IDIS / Becker BE2569/2580/2582 (Also used in Porsche PCM1).<br>
Since the tape is not going to be used, it's a potential audio input that could be used for something else.<br>

This firmware fully emulates the tape presence, its states and sensors.<br>

While in play mode, if the user pushes the "1-2" button (change tape play direction), this is detected and the CALL output is activated.<br>
This is useful if your bluetooth module has hands-free function and button input to pick up or hang up a call.<br>

To provide the best compatibility there are two adjustable options:
* Button polarity ("POL").<br>
	Open = Button idle state 0V (Low Level).<br>
	Closed = Button idle state VCC (High level).<br>
			
* Auto resume ("AR").<br>
	Make a test on your phone: Start playing a song. Put in pause and then skip to the next song.<br>
	Adjust as follows:<br>
	Open = In pause state, cellphone resumes playback automatically after skipping tracks.<br>
	Closed = In pause state, cellphone doesn't resume playback automatically after skipping tracks.<br>
	
The pins can be left floating (open) or connect to ground (closed).<br>

The outputs that handle the pulses for the buttons are set as open-drain (They only pull to ground, but don't put out voltage).<br>
You will need to connect pull-up resistors to the working voltage of the bluetooth module.<br>
This way, any voltage level is compatible. STM32 handles max 5V, don't exceed that!<br>

To debug the state of the board, the led is used.<br>
- Slow, heart beat-like blinks: The board is working but tape not active<br>
- Fast blinking: The tape is in play mode.
- Steady lit: The tape is in fast rewind / fast forward mode.


The firmware is implemented in a cheap STM32F103 "blue pill" board ($3).<br>
The code doesn't use any special peripheral and can be very easily ported to other devices, all needed are some GPIOs and a timer to provide timing control.<br>

<a id="use"></a>
## How to use
- Connect to the bluetooth module and manually start playback in the phone.<br>
- Set ICS in Tape mode.<br>
- Now the music should be playing. Use the steering wheel or ICS controls to skip tracks.<br>

<a id="limitations"></a>
## Limitations
The controller is unable to know if the song is playing, or the current song playback time.<br>
Due that, it might stay in pause when skipping backwards.<br>
In that case just skip forwards, wait for the song to start and skip backwards again.<br>
The tape system is slow. If you skip very quick, it will miss some steps. Nothing to worry about.<br>

Some phones or bluetooh modules won't like pressing the buttons too fast.<br>
If you have issues with the fastest (200mS), then try slower options (400, 600, 800, 1000mS).
If it always fail when quickly pressing ICS buttons, and none of the builds fixes it, it means the phone or bluetooth module can't take two consecutive next/prev pulses.<br>
Use NoRepeat build in that case.<br>


<a id="firmware"></a>
## Firmware

You can download already compiled binaries in the Release folder (tape_btnOff_200mS.bin)<br>
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
