/*
 * tape.c
 *
 *  Created on: Dec 22, 2020
 *      Author: David
 */
#include "tape.h"

	// VARIABLES

	tape_t	tape;
	position_t	pos;

void initTape(void){
	tape.status		= status_stop;															// Initial state stop
	pos.OutStatus	= pos_2_5V;																// Default analog status = Tape inside
	pos.Status		= 0;																	// First state
	pos.Direction	= 1;																	// Initial position changing direction
	resetButtons();																			// Set all buttons to idle
}

void handleTape(void){

	tape.polarity	= !HAL_GPIO_ReadPin(POLARITY_GPIO_Port, POLARITY_Pin);					// Update button polarity (So it can be changed on the fly)
	handlePosSensor();																		// Handle position sensor

	  /*********************************************************************************************************************
	   * 	Check Forward/Reverse inputs
	   *********************************************************************************************************************/
	  if(L_Plus && L_Minus){																// L+ =1 and L- =1, this means Tape active in a stable state
		  if(MT_Rev || MT_Fwd){
			  if( ((HAL_GetTick()-tape.freqPhotoTimer) >pulseDelay ) && tape.enablePhoto){	// Simulate the photo sensor pulses if active flag
				  tape.freqPhotoTimer	= HAL_GetTick();
				  HAL_GPIO_TogglePin(PHOTO_R_GPIO_Port, PHOTO_R_Pin);						// Toggle both sensors (they can be joined together)
				  HAL_GPIO_TogglePin(PHOTO_F_GPIO_Port, PHOTO_F_Pin);						// I use separated pins just in case something else can be done in the future
			  }
			  if(!H_Speed){																	// Not in in fast speed (revPlay or fwdPlay mode)
				  if(MT_Rev){
					  if( (tape.status==status_play) && (tape.playMode!=revPlay) ){			// Tape direction changed while in play mode
						  setButton(btn_call);
					  }
					  tape.playMode	= revPlay;												// Update play mode (reverse)
				  }

				  if(MT_Fwd){
					  if( (tape.status==status_play) && (tape.playMode!=fwdPlay) ){			// Tape direction changed while in play mode
						  setButton(btn_call);
					  }
					  tape.playMode	= fwdPlay;												// Update play mode (forward)
				  }

				  if(tape.status!=status_play){												// Wasn't in play mode before?
					  tape.enablePhoto	= 1;												// Enable photo sensor
					  tape.playTimer	= HAL_GetTick();									// Time when playing started

					  if( (tape.status==status_ffwd)||(tape.status==status_frwd) ){			// Was in a fast mode before?
						  if( (HAL_GetTick()-tape.fastTimer) < btnRepTim){					// Check the timer, if it's low, the user exited fast mode by pushing the button again (Repeat button push)
							  if(tape.btnLast==btn_prev){									// Previous track button was pushed
								  setButton(btn_prev);										// Repeat
							  }
							  if(tape.btnLast==btn_next){									// Next track button was pushed
								  setButton(btn_next);										// Repeat
							  }
						  }
					  }
					  tape.status	= status_play;											// In any case, now we are in play mode
				  }
			  }
			  else{																			// In high speed mode (Fast Rewind or Fast Forward)
				  if(MT_Fwd^MT_Rev){														// What direction?
					  if( (tape.status!=status_frwd) && (tape.status!=status_ffwd) ){		// Wasn't in Fast mode before?
						  tape.skipCnt++;													// Increase skipped track count
						  tape.delayPhotoTimer	= HAL_GetTick();							// Update photo sensor disable timer
						  tape.fastTimer		= HAL_GetTick();						  	// Fast mode timer

						  if(MT_Fwd){														// Tape mechanism in Fast Forward mode
							  if(tape.playMode==revPlay){									// Was playing in reverse mode?
								  tape.status	= status_frwd;								// Now we are in fast rewind
								  setButton(btn_prev);										// Send pulse to previous track button
							  }
							  if(tape.playMode==fwdPlay){									// Was playing in forward mode?
								  tape.status	= status_ffwd;								// Now we are in fast forward
								  setButton(btn_next);										// Send pulse to next track button
							  }
						  }

						  if(MT_Rev){														// Tape mechanism in Fast Rewind mode
							  if(tape.playMode==revPlay){									// Was playing in reverse mode?
								  tape.status=status_ffwd;									// Now we are in fast forward
								  setButton(btn_next);										// Send pulse to next track button
							  }
							  if(tape.playMode==fwdPlay){									// Was playing in forward mode?
								  tape.status=status_frwd;									// Now we are in fast rewind
								  setButton(btn_prev);										// Send pulse to previous track button
							  }
						  }
					  }
				  }
			  }
		  }
	  }
	  if((tape.status==status_frwd) || (tape.status==status_ffwd)){							// If in fast mode (Fast rewind or Fast forward)
		  if(tape.skipCnt>=MAX_FAST_SKIP){													// Check we didn't skip more than 3 tracks in quick mode
			  if((HAL_GetTick()-tape.delayPhotoTimer)>longPhotoDelay){						// Otherwise wait the delay to avoid tape error triggering (Or tape controller will think the  tape is stuck)
				  tape.skipCnt	= 0;														// Once the delay is done, Reset count
			  }
		  }
		  else{																				// If less than 3 consecutively skipped tracks, no delay applied
			  tape.enablePhoto	= 0;  	  	  	  	  	  	  	  	  	  	  	 			// Disable photo sensor pulses to simulate end of tape (back to play mode)
		  }
	  }
	  if(tape.skipCnt){																		// If there's any value in the skipped track counter
		  if((tape.status==status_play) && (HAL_GetTick()-tape.playTimer)>resetTimeOnPlay){	// If in play mode for more than 2 seconds
			  tape.skipCnt		= 0;														// Reset the counter
		  }
	  }
	  if(tape.btnPushed){																	// If any button was pushed
		  if((HAL_GetTick()-tape.btnTimer)>btnDelay){										// If the button timer has expired
			  resetButtons();																// Reset all the buttons
		  }
	  }

	  handleLed();																			// Handle led status
}


/*********************************************************************************************************************
 *		Handles tape position sensor. L+ and L- signals set the motor polarity.
 *
 *		If (L+==1  ^ L-==1) ==> Changing position (Only one active, not both or none)
 *		As long as the motor spins, we keep switching between the analog voltages with a small delay.
 *		2.5V <> 1.2V <> 0V
 *
 *		When the tape controller finds the correct position, it stops the motor setting both L+ and L- high
 *********************************************************************************************************************/
void handlePosSensor(void){
	  if(L_Plus ^ L_Minus ){																// Only L+ =1 or L- =1. this means tape position is moving

		  switch(pos.Status){																// Switch internal state
				case 0:																		// Status 0, Load position Delay
					pos.Timer	= HAL_GetTick();											// Load timer
					pos.Status	= 1;														// Set status 1
					break;

				case 1:																		// Status 1, wait position Delay to let the controller process the status and turn on/off the position motor
					if((HAL_GetTick()-pos.Timer)>positionDelay){							// If position Delay time elapsed
						pos.Status	= 2;													// Set Status 2
					}
					break;

				case 2:																		// Status 2, change position
					pos.OutStatus += pos.Direction;											// Change position (will add or subtract 1 to the position depending on the direction)
					if(pos.OutStatus==MIN_POS){												// If reached maximum position
						pos.Direction	= 1;												// Reverse direction

					}
					if(pos.OutStatus==MAX_POS){												// If reached minimum position
						pos.Direction	= -1;												// Reverse direction
					}
					pos.Status	= 0;														// Set Status 0, restart the cycle
					break;

				default:																	// We shouldn't get here
					pos.Status	= 0;														// If this happens, set the default state
					break;
		  }
	  }
	  if(!L_Plus && !L_Minus && (tape.status!=status_stop) ){								// L+ =0 and L- =0, this means tape disabled (ICS not in Tape mode)
		  pos.OutStatus		= pos_2_5V;														// Reset everything
		  tape.enablePhoto	= 1;
		  tape.status		= status_stop;
	  }

	  /*********************************************************************************************************************
	   * 	Set the outputs for the analog sensor based on the position status
	   * 	SET = open drain, pin Hi-Z.
	   * 	RESET = Set to Gnd
	   *********************************************************************************************************************/
	  switch(pos.OutStatus){																// Set the analog position ("POS. SEN" pin)
		  case pos_5V:
			  HAL_GPIO_WritePin(POS_2_5_GPIO_Port,POS_2_5_Pin,SET);							// All the pins floating
			  HAL_GPIO_WritePin(POS_1_2_GPIO_Port,POS_1_2_Pin,SET);
			  HAL_GPIO_WritePin(POS_0_GPIO_Port,POS_0_Pin,SET);
			  break;

		  case pos_2_5V:																	// 6.8K resistor to ground
			  HAL_GPIO_WritePin(POS_2_5_GPIO_Port,POS_2_5_Pin,RESET);
			  HAL_GPIO_WritePin(POS_1_2_GPIO_Port,POS_1_2_Pin,SET);
			  HAL_GPIO_WritePin(POS_0_GPIO_Port,POS_0_Pin,SET);
			  break;

		  case pos_1_2V:																	// 2.2K resistor to ground
			  HAL_GPIO_WritePin(POS_2_5_GPIO_Port,POS_2_5_Pin,SET);
			  HAL_GPIO_WritePin(POS_1_2_GPIO_Port,POS_1_2_Pin,RESET);
			  HAL_GPIO_WritePin(POS_0_GPIO_Port,POS_0_Pin,SET);
			  break;

		  case pos_0V:																		// Direct to ground (Doesn't matter the others)
			  HAL_GPIO_WritePin(POS_0_GPIO_Port,POS_0_Pin,RESET);
			  break;

		  default:
			  pos.OutStatus	= pos_2_5V;
	  }
}

void setButton(uint8_t btn){
	tape.btnLast	= btn;																	// Store button
	tape.btnPushed	= 1;																	// Set flag
	tape.btnTimer	= HAL_GetTick();														// Load timer

	switch(btn){																			// Find what button to enable

		case btn_prev:																		// Previous track button
			HAL_GPIO_WritePin(BTN_PREV_GPIO_Port,BTN_PREV_Pin,tape.polarity);				// Set output depending on the polarity
			break;

		case btn_next:																		// Next track button
			HAL_GPIO_WritePin(BTN_NEXT_GPIO_Port,BTN_NEXT_Pin,tape.polarity);				// Set output depending on the polarity
			break;

		case btn_call:																		// Call button
			HAL_GPIO_WritePin(BTN_CALL_GPIO_Port,BTN_CALL_Pin,tape.polarity);				// Set output depending on the polarity
			break;

		default:																			// We shouldn't get any unknown value
			resetButtons();																	// If this happens,clear all the buttons
			break;
	}
}

void resetButtons(void){
	tape.btnPushed	= 0;																		// Clear flag
	HAL_GPIO_WritePin(BTN_PREV_GPIO_Port,BTN_PREV_Pin,!tape.polarity);						// Set all buttons to idle state depending on the polarity
	HAL_GPIO_WritePin(BTN_NEXT_GPIO_Port,BTN_NEXT_Pin,!tape.polarity);
	HAL_GPIO_WritePin(BTN_CALL_GPIO_Port,BTN_CALL_Pin,!tape.polarity);
}

void handleLed(void){
	static uint32_t time	= 0;																	// Internal variable for timing the blinking times, only initial state = 0
	if(tape.status!=status_stop){															// If not in stop mode
		if(tape.status!=status_play){														// If not in play mode
			HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin, RESET);								// We are in fast mode, set the LED steady on
		}
		else if(tape.status==status_play){													// We are in play mode,quick led blinking
			if(HAL_GetTick()-time>50){														// If time elapsed > 50mS
				HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);									// Toggle LED GPIO
				time	= HAL_GetTick();													// Load timer
			}
		}
	}
	else{																					// We are in stop mode, make small blinks as "I'm alive signal"
		if(!HAL_GPIO_ReadPin(LED_GPIO_Port,LED_Pin)){										// Read LED status, if 0 (LED on)

			if(HAL_GetTick()-time>20){														// If time elapsed > 20mS
				HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin, SET);								// Turn the LED off
				time	= HAL_GetTick();													// Load timer
			}
		}
		else{																				// If LED off
			if(HAL_GetTick()-time>1000){													// If time elapsed > 1000mS
				HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin, RESET);							// Turn the LED on
				time	= HAL_GetTick();													// Load timer
			}
		}
	}
}
