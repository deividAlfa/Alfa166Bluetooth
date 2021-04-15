/*
 * tape.h
 *
 *  Created on: Jan 12, 2021 Dec 22, 2020
 *      Author: David
 */

#ifndef INC_TAPE_H_
#define INC_TAPE_H_
#include "main.h"


/***********************************************************
 *     TO CHANGE THE GPIO PINS, FOLLOW THIS TABLE (GPIO USER NAME IS SET IN CUBEMX):
 *
 *     GPIO NAME    MODE      OPTIONS
 *
 *     MT_FWD       INPUT     NO PULL
 *     MTD_RVS      INPUT     NO PULL
 *     H_SPEED      INPUT     NO PULL
 *     L_plus       INPUT     NO PULL
 *     L_minus      INPUT     NO PULL
 *     POLARITY     INPUT     PULL UP
 *     RESUME       INPUT     PULL UP
 *
 *     POS_0V       OUTPUT    OPEN DRAIN
 *     POS_1_2V     OUTPUT    OPEN DRAIN
 *     POS_2_5V     OUTPUT    OPEN DRAIN
 *     PHOTO_R      OUTPUT    OPEN DRAIN
 *     PHOTO_F      OUTPUT    OPEN DRAIN
 *     BTN_PREV     OUTPUT    OPEN DRAIN
 *     BTN_NEXT     OUTPUT    OPEN DRAIN
 *     BTN_CALL     OUTPUT    OPEN DRAIN
 *
 *    Be careful! If the outputs are not set as open drain, the pin will output 3.3V when high
 *    This might damage the bluetooth module if it works at a lower voltage, ex. 1.8V
 *    Also, the photo sensors won't recognize a 3.3V signal, they need 5V pulses to work!
 *
 ************************************************************/

typedef enum{
  reset_val=0,

  pos_0V,                                       // Play position. Signal direct to GND, 0V
  pos_1_2V,                                     // Transition from/to play position. 2.2K resistor to GND, 1.2V
  pos_2_5V,                                     // Tape in, also RWD and FF position. 6.8K resistor to GND, 2.5V
  pos_5V,                                       // No tape, all positions floating (Unused)

  status_stop,                                  // In stop mode
  status_play,                                  // Playing
  status_pause,                                 // Pause
  status_frwd,                                  // Fast rewind
  status_ffwd,                                  // Fast forward

  fwdPlay,                                      // Playing in forward mode
  revPlay,                                      // Playing in reverse mode

  btn_next,                                     // Next track button
  btn_prev,                                     // Previous track button
  btn_call,                                     // Call button
  btn_play_pause,                               // Play-Pause button
  btn_play_pause_Call,                          // Play-Pause-call button

}status_t;


// Struct for tape management
typedef struct{
  volatile uint32_t         delayPhotoTimer;    // Timer for photo sensor disable delay
  volatile uint32_t         playTimer;          // Time when the playback started
  volatile uint32_t         freqPhotoTimer;     // Timer for photo sensor signal generation
  volatile uint32_t         changedTimer;       // Timer for delay after skip tracks.
  volatile uint32_t         skipTimer;          // Timer To recognize if the tape returned to play mode by itself or if a button was pulsed again (add another pulse to the BT module)
  volatile uint32_t         stopTimer;          // Time to measure stop state

  volatile uint8_t          skipCnt;            // Counter for consecutive skips. More than 4 fast skips will make the tape think it's stuck. Wait longer this time.
  volatile status_t         playMode;           // Last known play mode
  volatile uint32_t         btnTimer;           // Timer used for buttons
  volatile status_t         skipBtn;            // Stored skip track button
  volatile status_t         status;             // Actual tape status
  volatile status_t         BTstatus;           // Actual BT status

  volatile bool             btnPushed;          // Flag to indicate a button was pushed (for reseting after timer expired timer)
  volatile bool             enablePhoto;        // Flag to enable or disable the photo sensor signal (For forcing RWD/FF modes to stop and resume and play mode)
  volatile bool             polarity;           // Flag to set the button polarity depending on the POL_TAPE input (Normally low, normally high)
  volatile bool             skipResume;         // Flag to indicate if the phone resumes playback after skipping a track (being n pause before)
  volatile bool             callOrPause;        // Flag to indicate the use of 1-2 button. As pause/resume or call output.
  volatile bool             repeatSkip;         // Flag to indicate there's a repeat pending
}tape_t;

// Struct for tape position function
typedef struct{
  volatile status_t         OutLevel;           // Actual analog position
  volatile uint32_t         Timer;              // Timer for position transition delays
  volatile uint8_t          Status;             // For position state machine
volatile int8_t             Direction;          // Switching direction
}position_t;

// Extensively tested, different times will probably cause malfunctioning
// Pretty critical timings! Delays in mS

#define positionDelay       100                 // Delay before changing positions, to let the controller process the signal and turn on/off the position motor. Less than 80mS will randomly fail
#define pulseDelay          10                  // High/low times for photo sensor signal generation (F = 50Hz)
#define longPhotoDelay      1300                // Photo sensor long delay time (to prevent fault detection if too many fast skips)
#define btnHighTime         100                 // Button active time (Most modules will ignore shorter times)
#define btnLowTime          400                 // Button idle time, to ensure enough time between button presses
#define btnRepTimHigh       1200                // Max time after returning to play state to recognize repeated button presses (The controller reverts automatically to play mode in 1.3 seconds)
#define btnRepTimLow        10                  // Min time to recognize repeated button presses
#define resetTimeOnPlay     2000                // Time in play state to reset the fast skip counter
#define resumeDelay         300                 // After returning to playback state, time to wait before sending play pulse. If too fast the phone might ignore it.
#define stopDelay           50                  // Min time in stop mode to actually set stop status
#define delay1_2            100                 // Min time in play mode to read 1-2 button (to avoid noises)

#define MIN_POS             pos_0V              // Minimum position
#define MAX_POS             pos_2_5V            // Maximum position
#define MAX_FAST_SKIP       4                   // Maximum fast skips before adding delay, to prevent fault detection

#define H_Speed             HAL_GPIO_ReadPin(H_SPEED_GPIO_Port, H_SPEED_Pin)
#define MT_Fwd              HAL_GPIO_ReadPin(MT_FWD_GPIO_Port, MT_FWD_Pin)
#define MT_Rev              HAL_GPIO_ReadPin(MT_RVS_GPIO_Port, MT_RVS_Pin)
#define L_Plus              HAL_GPIO_ReadPin(L_plus_GPIO_Port, L_plus_Pin)
#define L_Minus             HAL_GPIO_ReadPin(L_minus_GPIO_Port, L_minus_Pin)


#define setPos_2_5vHigh()   HAL_GPIO_WritePin(POS_2_5_GPIO_Port,POS_2_5_Pin,SET)
#define setPos_2_5vLow()    HAL_GPIO_WritePin(POS_2_5_GPIO_Port,POS_2_5_Pin,RESET)
#define setPos_1_2vHigh()   HAL_GPIO_WritePin(POS_1_2_GPIO_Port,POS_1_2_Pin,SET)
#define setPos_1_2vLow()    HAL_GPIO_WritePin(POS_1_2_GPIO_Port,POS_1_2_Pin,RESET)
#define setPos_0vHigh()     HAL_GPIO_WritePin(POS_0_GPIO_Port,POS_0_Pin,SET)
#define setPos_0vLow()      HAL_GPIO_WritePin(POS_0_GPIO_Port,POS_0_Pin,RESET)

#define EnableRepeat        1

void initTape(void);
void handleTape(void);
void handlePosSensor(void);
void handleLed(void);
void setButton(status_t btn);
void handleButtonReset(void);

#endif /* INC_TAPE_H_ */
