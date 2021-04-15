/*
 * tape.c
 *
 *  Created on: Jan 12, 2021 Dec 22, 2020
 *      Author: David
 */
#include "tape.h"

  // VARIABLES
  tape_t   	  tape;
  position_t  pos;

void initTape(void){
  tape.status     = status_stop;                                                      // Initial tape state stop
  tape.BTstatus   = status_pause;                                                     // Assume BT is paused
  pos.OutLevel    = pos_2_5V;                                                         // Default analog status = Tape inside
  pos.Status      = 0;                                                                // First state
  pos.Direction   = 1;                                                                // Initial position changing direction
  tape.polarity   = !HAL_GPIO_ReadPin(POLARITY_GPIO_Port, POLARITY_Pin);              // Read polarity before resetting the buttons

  HAL_GPIO_WritePin(BTN_PREV_GPIO_Port,BTN_PREV_Pin,tape.polarity);                   // Reset all buttons
  HAL_GPIO_WritePin(BTN_NEXT_GPIO_Port,BTN_NEXT_Pin,tape.polarity);
  HAL_GPIO_WritePin(BTN_CALL_GPIO_Port,BTN_CALL_Pin,tape.polarity);
  HAL_GPIO_WritePin(BTN_PLAY_PAUSE_GPIO_Port,BTN_PLAY_PAUSE_Pin,tape.polarity);
  HAL_GPIO_WritePin(BTN_PLAY_PAUSE_CALL_GPIO_Port,BTN_PLAY_PAUSE_CALL_Pin,tape.polarity);
}

void handleTape(void){
  uint32_t currentTime = HAL_GetTick();
  tape.polarity = !HAL_GPIO_ReadPin(POLARITY_GPIO_Port, POLARITY_Pin);                // Update button polarity (So it can be changed on the fly)
  tape.skipResume = HAL_GPIO_ReadPin(RESUME_GPIO_Port, RESUME_Pin);                   // Update skip resume status (So it can be changed on the fly)

  handlePosSensor();                                                                  // Handle position sensor

  /*********************************************************************************************************************
   *     Check Forward/Reverse inputs
   *********************************************************************************************************************/

  if((tape.status==status_play) && ((currentTime-tape.freqPhotoTimer) > pulseDelay ) && tape.enablePhoto){         // Simulate the photo sensor pulses if active flag
    tape.freqPhotoTimer = currentTime;
    HAL_GPIO_TogglePin(PHOTO_R_GPIO_Port, PHOTO_R_Pin);                               // Toggle both sensors (they can be joined together)
    HAL_GPIO_TogglePin(PHOTO_F_GPIO_Port, PHOTO_F_Pin);                               // I use separated pins just in case something else can be done in the future
  }

  if(L_Plus && L_Minus && !tape.btnPushed && (MT_Rev || MT_Fwd)){                     // L+ =1 and L- =1, this means Tape active in a stable state
    tape.stopTimer = currentTime;
    if(!H_Speed){                                                                     // Not in in fast speed (revPlay or fwdPlay mode)
      if(tape.status==status_play){
        if(tape.BTstatus==status_pause){                                              // If BT paused after track change
          if(!tape.skipResume){                                                       // If this phone doesn't resume playback automatically after skipping track
            if(currentTime-tape.playTimer>resumeDelay){                               // Wait some time, as we just send the skip track pulse. If too fast, it will not work.
              setButton(btn_play_pause);                                              // Resume now
              tape.BTstatus=status_play;                                              // Set play state
            }
          }
          else{                                                                       // This phone resumes playback automatically
            tape.BTstatus=status_play;                                                // Set play state
          }
        }
        if(MT_Rev){
          if(tape.playMode!=revPlay){                                                 // Tape direction changed while in play mode
            setButton(btn_call);                                                      // Press call button
          }
          tape.playMode = revPlay;                                                    // Update play mode (reverse)
        }
        else if(MT_Fwd){
          if(tape.playMode != fwdPlay){                                               // Tape direction changed while in play mode
            setButton(btn_call);                                                      // Press call button
          }
          tape.playMode = fwdPlay;                                                    // Update play mode (forward)
        }
      }
      else{                                                                           // Wasn't in play mode before?
        tape.enablePhoto = 1;                                                         // Enable photo sensor
        if( (tape.status==status_ffwd) || (tape.status==status_frwd) ){               // Was in a fast mode before?
          if((currentTime-tape.skipTimer) < btnRepTim){                               // Check the timer, if it's low, the user exited fast mode by pushing the button again (Repeat button push)
            tape.skipTimer = 0;                                                       // Clear timer to avoid setting repeat again
            tape.repeatSkip = EnableRepeat;                                           // Enable repeat
          }

          if(tape.skipBtn==btn_prev){                                                 // Previous track button was pushed
            setButton(btn_prev);                                                      // Repeat
          }
          else if ( tape.skipBtn == btn_next ){                                       // Next track button was pushed
            setButton(btn_next);                                                      // Repeat
          }

          if(tape.repeatSkip){                                                        // If repeat flag is set
            tape.repeatSkip = 0;                                                      // Reset flag, the button will be presses again
          }
          else{                                                                       // If no repeat flag
            tape.status = status_play;                                                // Set play mode
            tape.playTimer = currentTime;                                             // Time when playing started
          }
        }
        else{                                                                         // If not coming from skipping track
          if(tape.status==status_stop && tape.BTstatus==status_pause){                // If coming from stop mode and BT paused
            setButton(btn_play_pause);                                                // Resume playback
          }
          tape.status = status_play;                                                  // Set play mode
          tape.playTimer = currentTime;                                               // Time when playing started
        }
        if(MT_Rev){
          tape.playMode = revPlay;                                                    // Update play mode (reverse)
        }
        else if(MT_Fwd){
          tape.playMode = fwdPlay;                                                    // Update play mode (forward)
        }
      }
    }
    else{                                                                             // In high speed mode (Fast Rewind or Fast Forward)
      if(MT_Fwd^MT_Rev){                                                              // What direction?
        if( (tape.status!=status_frwd) && (tape.status!=status_ffwd) ){               // Wasn't in Fast mode before?
          tape.repeatSkip = 0;                                                        // Reset repeat state
          tape.skipCnt++;                                                             // Increase skipped track count
          tape.delayPhotoTimer = currentTime;                                         // Update photo sensor disable timer
          tape.skipTimer = currentTime;                                               // Update skip timer

          if(MT_Fwd){                                                                 // Tape mechanism in Fast Forward mode
            if(tape.playMode==revPlay){                                               // Was playing in reverse mode?
              tape.status = status_frwd;                                              // Now we are in fast rewind
              tape.skipBtn = btn_prev;                                                // Send pulse to previous track button
            }
            if(tape.playMode==fwdPlay){                                               // Was playing in forward mode?
              tape.status = status_ffwd;                                              // Now we are in fast forward
              tape.skipBtn = btn_next;                                                // Send pulse to next track button
            }
          }

          if(MT_Rev){                                                                 // Tape mechanism in Fast Rewind mode
            if(tape.playMode==revPlay){                                               // Was playing in reverse mode?
              tape.status=status_ffwd;                                                // Now we are in fast forward
              tape.skipBtn = btn_next;                                                // Send pulse to next track button
            }
            if(tape.playMode==fwdPlay){                                               // Was playing in forward mode?
              tape.status=status_frwd;                                                // Now we are in fast rewind
              tape.skipBtn = btn_prev;                                                // Send pulse to previous track button
            }
          }
          if(tape.BTstatus!=status_pause){                                            // If not in pause
            tape.BTstatus=status_pause;
            setButton(btn_play_pause);                                                // Pause now
          }
    	  }
    	}
    }
  }
  if((tape.status==status_frwd) || (tape.status==status_ffwd)){                       // If in fast mode (Fast rewind or Fast forward)
    if(tape.skipCnt>=MAX_FAST_SKIP){                                                  // Check we didn't skip more than 3 tracks in quick mode
      if((currentTime-tape.delayPhotoTimer)>longPhotoDelay){                          // Otherwise wait the delay to avoid tape error triggering (Or tape controller will think the  tape is stuck)
        tape.skipCnt = 0;                                                             // Once the delay is done, Reset count
      }
    }
    else{                                                                             // If less than 3 consecutively skipped tracks, no delay applied
      tape.enablePhoto = 0;                                                           // Disable photo sensor pulses to simulate end of tape (back to play mode)
    }
  }
  if(tape.skipCnt){                                                                   // If there's any value in the skipped track counter
    if((tape.status==status_play) && (currentTime-tape.playTimer)>resetTimeOnPlay){   // If in play mode for more than 2 seconds
      tape.skipCnt = 0;                                                               // Reset the counter
    }
  }
  if(tape.btnPushed){                                                                 // If any button active
    handleButtonReset();                                                              // handle button reset
  }
  handleLed();                                                                        // Handle led status
}


/*********************************************************************************************************************
 *        Handles tape position sensor. L+ and L- signals set the motor polarity.
 *
 *        If (L+==1  ^ L-==1) ==> Changing position (Only one active, not both or none)
 *        As long as the motor spins, we keep switching between the analog voltages with a small delay.
 *        2.5V <> 1.2V <> 0V
 *
 *        When the tape controller finds the correct position, it stops the motor setting both L+ and L- high
 *********************************************************************************************************************/
void handlePosSensor(void){
  uint32_t currentTime=HAL_GetTick();

  if(L_Plus ^ L_Minus ){                                                              // Only L+ =1 or L- =1. this means tape position is moving
    switch(pos.Status){                                                               // Switch internal state
      case 0:                                                                         // Status 0, Load position Delay
        pos.Timer = currentTime;                                                      // Load timer
        pos.Status = 1;                                                               // Set status 1
        break;

      case 1:                                                                         // Status 1, wait position Delay to let the controller process the status and turn on/off the position motor
        if((currentTime-pos.Timer)>positionDelay){                                    // If position Delay time elapsed
          pos.Status = 2;                                                             // Set Status 2
        }
        break;

      case 2:                                                                         // Status 2, change position
        pos.OutLevel += pos.Direction;                                                // Change position (will add or subtract 1 to the position depending on the direction)
        if(pos.OutLevel<=MIN_POS){                                                    // If reached maximum position
          pos.Direction = 1;                                                          // Reverse direction

        }
        if(pos.OutLevel>=MAX_POS){                                                    // If reached minimum position
          pos.Direction = -1;                                                         // Reverse direction
        }
        pos.Status = 0;                                                               // Set Status 0, restart the cycle
        break;

      default:                                                                        // We shouldn't get here
        pos.Status = 0;                                                               // If this happens, set the default state
        break;
    }
  }
  if(!tape.btnPushed && !L_Plus && !L_Minus && (tape.status!=status_stop) ){          // L+ =0 and L- =0, this means tape disabled (ICS not in Tape mode)
    pos.OutLevel = pos_2_5V;                                                          // Reset everything
    tape.enablePhoto = 1;
    if(currentTime-tape.stopTimer>1000){                                              // If in stop state for more than 1 second
      if(tape.status==status_play){                                                   // If it was in playback status
        setButton(btn_play_pause);                                                    // Pause playback
        tape.BTstatus=status_pause;
      }
      tape.status = status_stop;
    }
  }

/*********************************************************************************************************************
 *     Set the outputs for the analog sensor based on the position status
 *     SET = open drain, pin Hi-Z.
 *     RESET = Set to Gnd
 *********************************************************************************************************************/
  switch(pos.OutLevel){                                                               // Set the analog position ("POS. SEN" pin)
    case pos_5V:
      HAL_GPIO_WritePin(POS_2_5_GPIO_Port,POS_2_5_Pin,SET);                           // All the pins floating
      HAL_GPIO_WritePin(POS_1_2_GPIO_Port,POS_1_2_Pin,SET);
      HAL_GPIO_WritePin(POS_0_GPIO_Port,POS_0_Pin,SET);
      break;

    case pos_2_5V:                                                                    // 6.8K resistor to ground
      HAL_GPIO_WritePin(POS_2_5_GPIO_Port,POS_2_5_Pin,RESET);
      HAL_GPIO_WritePin(POS_1_2_GPIO_Port,POS_1_2_Pin,SET);
      HAL_GPIO_WritePin(POS_0_GPIO_Port,POS_0_Pin,SET);
      break;

    case pos_1_2V:                                                                    // 2.2K resistor to ground
      HAL_GPIO_WritePin(POS_2_5_GPIO_Port,POS_2_5_Pin,SET);
      HAL_GPIO_WritePin(POS_1_2_GPIO_Port,POS_1_2_Pin,RESET);
      HAL_GPIO_WritePin(POS_0_GPIO_Port,POS_0_Pin,SET);
      break;

    case pos_0V:                                                                      // Direct to ground (Doesn't matter the others)
      HAL_GPIO_WritePin(POS_0_GPIO_Port,POS_0_Pin,RESET);
      break;

    default:
      pos.OutLevel = pos_2_5V;                                                        // By default set 2.5V state (Tape inserted)
  }
}

void setButton(status_t btn){
  tape.btnPushed = 1;                                                                 // Set flag
  tape.btnTimer = HAL_GetTick();                                                      // Load timer

  switch(btn){                                                                        // Find what button to enable
    case btn_prev:                                                                    // Previous track button
      HAL_GPIO_WritePin(BTN_PREV_GPIO_Port,BTN_PREV_Pin,!tape.polarity);              // Set output depending on the polarity
      break;

    case btn_next:                                                                    // Next track button
      HAL_GPIO_WritePin(BTN_NEXT_GPIO_Port,BTN_NEXT_Pin,!tape.polarity);              // Set output depending on the polarity
      break;

    case btn_call:                                                                    // Call button
      HAL_GPIO_WritePin(BTN_CALL_GPIO_Port,BTN_CALL_Pin,!tape.polarity);              // Set output depending on the polarity
      HAL_GPIO_WritePin(BTN_PLAY_PAUSE_CALL_GPIO_Port,BTN_PLAY_PAUSE_CALL_Pin,!tape.polarity);
      break;

    case btn_play_pause:                                                              // Play/pause button
      HAL_GPIO_WritePin(BTN_PLAY_PAUSE_GPIO_Port,BTN_PLAY_PAUSE_Pin,!tape.polarity);  // Set output depending on the polarity
      HAL_GPIO_WritePin(BTN_PLAY_PAUSE_CALL_GPIO_Port,BTN_PLAY_PAUSE_CALL_Pin,!tape.polarity);
      break;

    default:                                                                          // We shouldn't get any unknown value
      HAL_GPIO_WritePin(BTN_PREV_GPIO_Port,BTN_PREV_Pin,tape.polarity);               // If this happens,clear all the buttons
      HAL_GPIO_WritePin(BTN_NEXT_GPIO_Port,BTN_NEXT_Pin,tape.polarity);
      HAL_GPIO_WritePin(BTN_CALL_GPIO_Port,BTN_CALL_Pin,tape.polarity);
      HAL_GPIO_WritePin(BTN_PLAY_PAUSE_GPIO_Port,BTN_PLAY_PAUSE_Pin,tape.polarity);
      HAL_GPIO_WritePin(BTN_PLAY_PAUSE_CALL_GPIO_Port,BTN_PLAY_PAUSE_CALL_Pin,tape.polarity);
      tape.btnPushed = 0;
      break;
  }
}

void handleButtonReset(void){
  enum{
	  status_isHigh,
	  status_isLow
  };
  static uint8_t status = status_isHigh;
  uint32_t currentTime  = HAL_GetTick();
  uint8_t OldStatus     = status;

  if(status==status_isHigh){                                                          // Button is already high
    if((currentTime-tape.btnTimer)>btnHighTime){                                      // If enough time in high state
      HAL_GPIO_WritePin(BTN_PREV_GPIO_Port,BTN_PREV_Pin,tape.polarity);               // Reset all buttons
      HAL_GPIO_WritePin(BTN_NEXT_GPIO_Port,BTN_NEXT_Pin,tape.polarity);
      HAL_GPIO_WritePin(BTN_CALL_GPIO_Port,BTN_CALL_Pin,tape.polarity);
      HAL_GPIO_WritePin(BTN_PLAY_PAUSE_GPIO_Port,BTN_PLAY_PAUSE_Pin,tape.polarity);
      HAL_GPIO_WritePin(BTN_PLAY_PAUSE_CALL_GPIO_Port,BTN_PLAY_PAUSE_CALL_Pin,tape.polarity);
      status = status_isLow;                                                          // Set status = low
    }
  }
  else if((currentTime-tape.btnTimer)>btnLowTime){                                    // If enough time in low state
    status = status_isHigh;                                                           // reset state
    tape.btnPushed = 0;                                                               // Reset flag
  }
  if(OldStatus!=status){                                                              // If status changes
    tape.btnTimer = currentTime;                                                      // Update time
  }
}

void handleLed(void){
  static uint32_t time = 0;                                                           // Internal variable for timing the blinking times, only initial state = 0
  uint32_t currentTime = HAL_GetTick();

  if(tape.status!=status_stop){                                                       // If not in stop mode
    if(tape.status!=status_play){                                                     // If not in play mode
      HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin, RESET);                                // We are in fast mode, set the LED steady on
    }
    else if(tape.status==status_play){                                                // We are in play mode,quick led blinking
      if(currentTime-time>50){                                                        // If time elapsed > 50mS
        HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);                                    // Toggle LED GPIO
        time = currentTime;                                                           // Load timer
      }
    }
  }
  else{                                                                               // We are in stop mode, make small blinks as "I'm alive signal"
    if(!HAL_GPIO_ReadPin(LED_GPIO_Port,LED_Pin)){                                     // Read LED status, if 0 (LED on)
      if(currentTime-time>20){                                                        // If time elapsed > 20mS
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin, SET);                                // Turn the LED off
        time = currentTime;                                                           // Load timer
      }
    }
    else{                                                                             // If LED off
      if(currentTime-time>1000){                                                      // If time elapsed > 1000mS
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin, RESET);                              // Turn the LED on
        time = currentTime;                                                           // Load timer
      }
    }
  }
}
