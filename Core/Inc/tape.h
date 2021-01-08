/*
 * tape.h
 *
 *  Created on: Dec 22, 2020
 *      Author: David
 */

#ifndef INC_TAPE_H_
#define INC_TAPE_H_
#include "main.h"


/***********************************************************
 * 		TO CHANGE THE GPIO PINS, FOLLOW THIS TABLE (GPIO USER NAME IS SET IN CUBEMX):
 *
 * 		GPIO NAME		MODE			OPTIONS
 *
 * 		MT_FWD			INPUT			NO PULL
 * 		MTD_RVS			INPUT			NO PULL
 * 		H_SPEED			INPUT			NO PULL
 * 		L_plus 			INPUT			NO PULL
 * 		L_minus			INPUT			NO PULL
 * 		POLARITY		INPUT			PULL DOWN
 *
 * 		POS_0V			OUTPUT			OPEN DRAIN
 * 		POS_1_2V		OUTPUT			OPEN DRAIN
 * 		POS_2_5V		OUTPUT			OPEN DRAIN
 * 		PHOTO_R			OUTPUT			OPEN DRAIN
 * 		PHOTO_F			OUTPUT			OPEN DRAIN
 * 		BTN_PREV		OUTPUT			OPEN DRAIN
 * 		BTN_NEXT		OUTPUT			OPEN DRAIN
 * 		BTN_CALL		OUTPUT			OPEN DRAIN
 * 		ALWAYS_HIGH		OUTPUT			PUSH-PULL
 *
 *		Be careful! If the outputs are not set as open drain, the pin will output 3.3V when high
 *		This might damage the bluetooth module if it works at a lower voltage, ex. 1.8V
 *		Also, the photo sensors won't recognize a 3.3V signal, they need 5V pulses to work!
 *
 ************************************************************/

enum{
  pos_5V,														// No tape, all positions floating (Unused)
  pos_2_5V,														// Tape in, also RWD and FF position. 6.8K resistor to GND, 2.5V
  pos_1_2V,														// Transition from/to play position. 2.2K resistor to GND, 1.2V
  pos_0V,														// Play position. Signal direct to GND, 0V

  status_stop,													// In stop mode
  status_play,													// Playing
  status_frwd,													// Fast rewind
  status_ffwd,													// Fast forward

  fwdPlay,														// Playing in forward mode
  revPlay,														// Playing in reverse mode

  btn_next,														// Next track button
  btn_prev,														// Previous track button
  btn_call,														// Call button
};


// Struct for tape management
typedef struct{
  volatile uint32_t 	delayPhotoTimer;						// Timer for photo sensor disable delay
  volatile uint32_t 	playTimer;								// Time when the play started
  volatile uint32_t 	freqPhotoTimer;							// Timer for photo sensor signal generation
  volatile uint32_t 	changedTimer;							// Timer for delay after skip tracks.
  volatile uint32_t 	fastTimer;								// Timer To recognize if the tape returned to play mode by itself or if a button was pulsed again (add another pulse to the BT module)

  volatile uint8_t		skipCnt;								// Counter for consecutive skips. More than 4 fast skips will make the tape think it's stuck. Wait longer this time.
  volatile uint8_t 		playMode;								// Last known play mode
  volatile uint32_t		btnTimer;
  volatile uint8_t 		btnLast;								// Last pushed button
  volatile uint8_t 		status;									// Actual status

  volatile bool 		btnPushed;								// Flag to indicate a button was pushed (for reseting after timer expired timer)
  volatile bool			enablePhoto;							// Flag to enable or disable the photo sensor signal (For forcing RWD/FF modes to stop and resume and play mode)
  volatile bool 		polarity;								// Flag to set the button polarity depending on the POL_TAPE input (Normally low, normally high)
}tape_t;

// Struct for tape position function
typedef struct{
	volatile uint8_t 	OutStatus;								// Actual analog position
	volatile uint32_t 	Timer;									// Timer for position transition delays
	volatile uint8_t 	Status;									// For position state machine
	volatile int8_t		Direction;								// Switching direction
}position_t;

// Pretty critical timings. Extensively tested, lower times can cause ocassional failures
																// Delays in mS
#define positionDelay					(uint8_t)	80			// Delay before changing positions, to let the controller process the signal and turn on/off the position motor. Less than 80mS will randomly fail
#define pulseDelay						(uint8_t)	10			// High/Low times for photo sensor signal generation (F = 50Hz)
#define longPhotoDelay					(uint16_t)	1200		// Photo sensor long delay time (to prevent fault detection if too many fast skips)
#define btnDelay						(uint8_t)	100			// Button pulsed time (Most modules will ignore shorter times)
#define btnRepTim						(uint16_t)	1000		// Time limit to recognize repeated button presses (The controller reverts automatically to play mode in 1.2-1.3 seconds, so below 1 second it's safe)
#define resetTimeOnPlay					(uint16_t)	2000		// Time in play mode to reset the fast skip counter

#define MIN_POS							pos_2_5V				// Minimum position
#define MAX_POS							pos_0V					// Maximum position
#define MAX_FAST_SKIP					(uint8_t)	4			// Maximum fast skips before adding delay, to prevent fault detection

#define H_Speed 						HAL_GPIO_ReadPin(H_SPEED_GPIO_Port, H_SPEED_Pin)
#define MT_Fwd 							HAL_GPIO_ReadPin(MT_FWD_GPIO_Port, MT_FWD_Pin)
#define MT_Rev 							HAL_GPIO_ReadPin(MT_RVS_GPIO_Port, MT_RVS_Pin)
#define L_Plus 							HAL_GPIO_ReadPin(L_plus_GPIO_Port, L_plus_Pin)
#define L_Minus 						HAL_GPIO_ReadPin(L_minus_GPIO_Port, L_minus_Pin)


#define setPos_2_5vHigh()				HAL_GPIO_WritePin(POS_2_5_GPIO_Port,POS_2_5_Pin,SET)
#define setPos_2_5vLow()				HAL_GPIO_WritePin(POS_2_5_GPIO_Port,POS_2_5_Pin,RESET)
#define setPos_1_2vHigh()				HAL_GPIO_WritePin(POS_1_2_GPIO_Port,POS_1_2_Pin,SET)
#define setPos_1_2vLow()				HAL_GPIO_WritePin(POS_1_2_GPIO_Port,POS_1_2_Pin,RESET)
#define setPos_0vHigh()					HAL_GPIO_WritePin(POS_0_GPIO_Port,POS_0_Pin,SET)
#define setPos_0vLow()					HAL_GPIO_WritePin(POS_0_GPIO_Port,POS_0_Pin,RESET)


void initTape(void);
void handleTape(void);
void handlePosSensor(void);
void handleLed(void);
void setButton(uint8_t btn);
void resetButtons(void);

#endif /* INC_TAPE_H_ */
