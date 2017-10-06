  
  // **********************************************
  // Loop Function
  // **********************************************
  
  void loop()
  {
    // Initalize Values
    unsigned long start_millis = millis();
    unsigned long start_micros = micros();
  
  
    // ********************************************************************************************
    // Sample Values
    // ********************************************************************************************
  
    // Read From Serial Port and Mirror
    if (Serial.available() > 0) {
      String rxSerialString = Serial.readStringUntil('\n');
      parseRXSerial(rxSerialString);
    }
  
  
    // Read From Capacitive Sensors
    boolean newSipperValue = digitalRead(pinDigital_CapSen_Sipper) == HIGH;
    if (newSipperValue != GBL_subjectTouchingSipper)
    {
      GBL_subjectTouchingSipper_Time = millis();
      GBL_subjectTouchingSipper = newSipperValue;
  
      if (GBL_subjectTouchingSipper)
        GBL_Sipper_LickCounts += 1;
    }
  
    boolean newHandleValue = digitalRead(pinDigital_CapSen_Handle) == HIGH;
    if (newHandleValue != GBL_subjectTouchingHandle)
    {
      GBL_subjectTouchingHandle_Time = millis();
      GBL_subjectTouchingHandle = newHandleValue;
    }
  
  
    // Read From Joystick
    GBL_x_axis = analogRead(pinAnalog_JoystickX) - GBL_x_axis_home;
    GBL_y_axis = analogRead(pinAnalog_JoystickY) - GBL_y_axis_home;
  
  
    // Set Joystick Zone Register
    boolean prev_GBL_JystkZone[7];
    memcpy(prev_GBL_JystkZone, GBL_JystkZone, 7 * sizeof(boolean));
    int GBL_prev_Current_Target = GBL_Current_Target;
  
    // update boolean GBL_JystkZone[7]
    setJoystickZoneRegister(prev_GBL_JystkZone, TARGET_THRESHOLD_DIST);
  
    // update int GBL_JystkZone_Counts[7] and int GBL_JystkZone_ValidCounts[3]
    updateJoystickZoneCounts(prev_GBL_JystkZone);
    updateJoystickZoneDwellTimer(prev_GBL_JystkZone);
  
  
    // Read from Button
    GBL_buttonPressed = digitalRead(pinDigital_PushButton) == LOW;
    if (GBL_buttonPressed == true)
      GBL_button_counter = GBL_button_counter + 1;
    else
      GBL_button_counter = 0;
  
  
    // New Variable
    static unsigned long GBL_time_trial_end_ms = 0;
    if (GBL_STATE_MACHINE == STATE_RUNNING)
    {
      GBL_time_trial_end_ms = millis();
    }
   
  
    // ********************************************************************************************
    // Stimulation State Machine
    // ********************************************************************************************
  
    int r = GBL_stimCrRndIdx;
    int w = GBL_stimIncrRndIdx;
    boolean crTrgt = GBL_Current_Target == GBL_TARGET;
    
    GBL_stimAmp_ua        = (crTrgt) ? GBL_stimCrAmpArray[r]        : GBL_stimIncrAmpArray[w];
    GBL_stimFreq_hz       = (crTrgt) ? GBL_stimCrFreqArray[r]       : GBL_stimIncrFreqArray[w];
    GBL_stimPulseWidth_us = (crTrgt) ? GBL_stimCrPulseWidthArray[r] : GBL_stimIncrPulseWidthArray[w];   
    GBL_stimNumPulses_int = (crTrgt) ? GBL_stimCrPulseNumArray[r]   : GBL_stimIncrPulseNumArray[w];
    
    //GBL_stimNumPulses_int  = 5;
    GBL_stimCarrierFreq_hz = 100;
    GBL_stimElectrode_int  = 5;
    GBL_stimTrialNum_int   = GBL_trial_number;
      
    const uint8_t byteAddr_Amp             = 0x01;
    const uint8_t byteAddr_Freq            = 0x02;
    const uint8_t byteAddr_PulseWidth      = 0x03;
    const uint8_t byteAddr_NumPulses       = 0x04;
    const uint8_t byteAddr_CarrierFreq     = 0x05;
    const uint8_t byteAddr_Electrode       = 0x06;
    const uint8_t byteAddr_TrialNum        = 0x07;
    
    const uint8_t byteAddr_ParamReadEnable = 0x40; // Byte C.6
    const uint8_t byteAddr_StimEnable      = 0x80; // Byte C.7
    
  
    // ***********************************
    // Calculate Stim State
    boolean trial_ended_lessThan_2000ms_ago = millis() - GBL_time_trial_end_ms < 2000;
    boolean crct_trgt_slcted                = GBL_Selected_Target == GBL_TARGET;
    boolean currntly_in_crct_trgt           = GBL_Current_Target == GBL_TARGET;
    boolean trial_is_running                = GBL_STATE_MACHINE == STATE_RUNNING;
    boolean joystick_in_a_target            = GBL_Current_Target >= 0;
    boolean dwll_time_less_than_2sec        = GBL_JystkZone_DwellTime[GBL_Current_Target] <= 2000;
    boolean prev_GBL_STIMSTATE_MACHINE      = GBL_STIMSTATE_MACHINE;
    
    GBL_STIMSTATE_MACHINE = STIMSTATE_OFF;
    
    if (!GBL_CATCH_TRIAL)
    {
      if (trial_is_running && dwll_time_less_than_2sec)
      {
        if (joystick_in_a_target)
          GBL_STIMSTATE_MACHINE = STIMSTATE_ON;
      }
      else if (!trial_is_running && trial_ended_lessThan_2000ms_ago)
      {
        if (crct_trgt_slcted && currntly_in_crct_trgt)
            GBL_STIMSTATE_MACHINE = STIMSTATE_ON;
      }
    }
    
    // ***********************************
    // Send info to TDT via db25

    // Check connectivity of DB25 driver chips, if disconnected attempt reconnection
    //boolean db25Driver_connected = db25_ReadDeviceState();
    //if (!db25Driver_connected && GBL_TRIAL_TYPE >= TRIAL_FIND_STIM_HIDDEN_NoTimeout)
    //{
    //  GBL_STATE_MACHINE = STATE_ERROR;
    //}
    
    boolean first_sample_in_new_target = GBL_prev_Current_Target != GBL_Current_Target;
    
    if ( (first_sample_in_new_target && trial_is_running) || 
         (first_sample_in_new_target && !trial_is_running && crct_trgt_slcted && currntly_in_crct_trgt) ||
         (prev_GBL_STIMSTATE_MACHINE == STIMSTATE_OFF && GBL_STIMSTATE_MACHINE == STIMSTATE_ON) )
    {

      // OLD 2 Wire Interface
      digitalWrite(pinDigital_STIM_ENABLE, LOW);
      
      // NEW DB25 Interface      
      db25StimDisable();
      
      db25_Send2TDT(byteAddr_Amp,         GBL_stimAmp_ua);
      db25_Send2TDT(byteAddr_Freq,        GBL_stimFreq_hz);
      db25_Send2TDT(byteAddr_PulseWidth,  GBL_stimPulseWidth_us);
      db25_Send2TDT(byteAddr_NumPulses,   GBL_stimNumPulses_int);
      db25_Send2TDT(byteAddr_CarrierFreq, GBL_stimCarrierFreq_hz);
      db25_Send2TDT(byteAddr_Electrode,   GBL_stimElectrode_int);
      db25_Send2TDT(byteAddr_TrialNum,    GBL_stimTrialNum_int);
            
      db25ClearValueBits();
      GBL_stimParamReadEnable = true;
    }
    else
      GBL_stimParamReadEnable = false; 
          
    
    // ***********************************
    // Turn On/Off Stimulator
    boolean switched_stimstates = prev_GBL_STIMSTATE_MACHINE != GBL_STIMSTATE_MACHINE;
          
    if (switched_stimstates || GBL_stimParamReadEnable)
    {
      switch (GBL_STIMSTATE_MACHINE)
      {
        // **********************************************
        // Communicate to TDT, and turn stim on
        // **********************************************
        case STIMSTATE_ON:
        {
          db25StimEnable();
          GBL_stimEnable = true;
          

//          // OLD INTERFACE - Stim
//          if (GBL_Current_Target != GBL_TARGET)
//          {
//            //Serial.println("Not Target");
//            digitalWrite(pinDigital_STIM_ENABLE, HIGH);
//            delayMicroseconds(100);
//            digitalWrite(pinDigital_STIM_ENABLE, LOW);
//            delayMicroseconds(100);
//          }
//          
//          //Serial.println("Not Target");
//          digitalWrite(pinDigital_STIM_ENABLE, HIGH);
//          GBL_stimulation_enabled = true;
          
          // TTL Pulse On Correct Target for Training - Stim
          if (GBL_Current_Target == GBL_TARGET)
          {
            //Serial.println("Target");
            digitalWrite(pinDigital_STIM_ENABLE, HIGH);
          }
          else
          {
            //Serial.println("Not Target");
            digitalWrite(pinDigital_STIM_ENABLE, LOW);
          }
          
          GBL_stimulation_enabled = true;
        }
        break;
    
        // **********************************************
        // Turn off stimulation
        // **********************************************
        case STIMSTATE_OFF:
        {
          // NEW DB25 Interface
          db25StimDisable();
          GBL_stimEnable = false;  
        
          // OLD INTERFACE 
          digitalWrite(pinDigital_STIM_ENABLE, LOW);
          
          GBL_stimulation_enabled = false;
        }    
        break;
      }
    }
    
    // OLD INTERFACE - Tell TDT The Trial Number 
    if (GBL_STATE_MACHINE == STATE_RESET)
      digitalWrite(pinDigital_STIM_PWM, HIGH);
      
    else
      digitalWrite(pinDigital_STIM_PWM, LOW);
      
  
    // ********************************************************************************************
    // LED Cues for Training
    // ********************************************************************************************
  
    // Check connectivity of LED driver chips, if disconnected attempt reconnection
    boolean ledDriver_connected = ledDriverReadDeviceState();   
    if (!ledDriver_connected && GBL_TRIAL_TYPE <= TRIAL_FIND_TARGET_HIDDEN_EqualTimeout)
    {
      GBL_STATE_MACHINE = STATE_ERROR;
    }
    
    boolean leave_LEDs_on_for_pairing = (GBL_STATE_MACHINE != STATE_RUNNING) &&
                                        (millis() - GBL_time_trial_end_ms < 2000) &&
                                        (GBL_Selected_Target == GBL_TARGET);
    // GBL VAR: Cue LEDs are on
    GBL_cue_leds_enabled = false;
  
    bool cueLEDCheck = true;
  
    // If trial state is no longer running, and paired LEDs are finished
    if (GBL_STATE_MACHINE != STATE_RUNNING && !leave_LEDs_on_for_pairing)
      cueLEDCheck = false;
  
    // If Cue LEDs are eligable
    if (cueLEDCheck)
    {
      // LED Cues Always on?
      if (GBL_LED_Cues_Overt)
        GBL_cue_leds_enabled = true;
  
      // LED Cues on only in Target
      else if (GBL_LED_Cues_Hidden)
      {
        if (GBL_JystkZone[GBL_TARGET] == true || leave_LEDs_on_for_pairing)
          GBL_cue_leds_enabled = true;
      }
    }
  
    // If Cue LEDs have been turned on
    if ( GBL_cue_leds_enabled ) 
      turnOn_LED(GBL_TARGET, CUE_LED_BRIGHTNESS);
    else
      turnOffAll_LED();
  
  
    // ********************************************************************************************
    // STATE MACHINE
    // ********************************************************************************************
  
    int prev_GBL_STATE_MACHINE = GBL_STATE_MACHINE;
  
    switch (GBL_STATE_MACHINE)
    {
      // **********************************************
      // Joystick Reset Flag
      // **********************************************
      case STATE_WAIT_FOR_JY_RSET:
        if ( GBL_handle_reset_flag == false )
        {
          // Reset LEDs Turn On
          analogWrite(pinDigital_RwdLED, RWD_LED_BRIGHTNESS);
  
          // If trial type is touching sipper, wait until sipper isn't touched
          if (GBL_TRIAL_TYPE <= 3)  // TRIAL_SIPPER_20, TRIAL_SIPPER_100, TRIAL_SIPPER_COUNTS_10
          {
            GBL_handle_reset_flag = (GBL_subjectTouchingSipper == false) &&
                                    (millis() - GBL_subjectTouchingSipper_Time > RSET_MIN_TIME_MS);
          }
  
          // If trial type is touching handle, wait until handle isn't touched
          else if (GBL_TRIAL_TYPE <= 5)  // TRIAL_HANDLE_20, TRIAL_HANDLE_100
          {
            GBL_handle_reset_flag = (GBL_subjectTouchingHandle == false) &&
                                    (millis() - GBL_subjectTouchingHandle_Time > RSET_MIN_TIME_MS);
          }
  
          // If handle is in RSET location, reset the joystick
          else
          {
            GBL_handle_reset_flag = (GBL_JystkZone[RSET]) &&
                                    (millis() - GBL_JystkZone_Time > RSET_MIN_TIME_MS);
          }
        }
        else
        {
          // Reset LEDs Turn Off
          analogWrite(pinDigital_RwdLED, LOW);
  
          // Reset Values
          GBL_Sipper_LickCounts = 0;
  
          // Update State Variable
          GBL_STATE_MACHINE = STATE_RUNNING;
        }
        break;
  
  
      // **********************************************
      // Trial is running
      // **********************************************
      case STATE_RUNNING:
        {
          boolean trial_completed = runTrial();
  
          // Lock Joystick Rewards?
          //trial_completed = trial_completed & unlocked;
  
          // Manual Reward
          trial_completed = trial_completed | GBL_buttonPressed;
          if (GBL_buttonPressed)
            GBL_Selected_Target = GBL_TARGET;
  
          // Update State Machine
          if (trial_completed == true)
            GBL_STATE_MACHINE = STATE_COMPLETE;
  
          break;
        }
  
      // **********************************************
      // Trial is Complete
      // **********************************************
      case STATE_COMPLETE:
        {
          if (millis() >= TRIAL_REWARD_DELAY_MS + GBL_state_changed_tstamp_ms)
          {
            // Success!
            if (GBL_Selected_Target == GBL_TARGET)
            {
              // Total Number of Rewards Counter
              GBL_reward_counter = GBL_reward_counter + 1;
              if (GBL_verbose)
                Serial.print("Reward #" + String(GBL_reward_counter, DEC) + " \n");
  
              // Update State Machine
              GBL_STATE_MACHINE = STATE_START_DISPENSE;
            }
  
            // Failure!
            else if (GBL_Selected_Target != GBL_TARGET)
            {
              if (GBL_verbose)
                Serial.print("Failure!! \n");
  
              // Update State Machine
              GBL_STATE_MACHINE = STATE_FAILURE;
            }
  
            // Turn on "End of Trial" Cue LEDs
            analogWrite(pinDigital_RwdLED, RWD_LED_BRIGHTNESS);
          }
  
          break;
        }
  
      // **********************************************
      // Liquid Reward
      // **********************************************
      case STATE_START_DISPENSE:
        {
          // Dispense One Liquid Reward
          int N_cycles = 150;   // State Machine Cycles
          if (GBL_button_counter < N_cycles)
          {
            // 20 drops at this flow rate equals 1 mL
            // Each drop is approx. 0.05 mL
  
            // TODO: REMOVE HACK
            //int list_of_catch_trials[20] = {24, 29, 37, 40, 42, 46, 48, 53, 57, 60, 65, 69, 71, 73, 76, 81, 83, 90, 92, 95};
            //bool found_value = false;
            //for (int i = 0; i < 20; i++)
            //  if (GBL_trial_number == list_of_catch_trials[i])
            //  {
            //    found_value = true;
            //    break;
            //  }
            //for (int j = 0; j < 20; j++)
            //  if (GBL_trial_number == (list_of_catch_trials[j]+99))
            //  {
            //    found_value = true;
            //    break;
            //  }
            //
            //if (found_value == false)
            startPulse(45);    //179
          }
          // Dispense Burst for Cleaning and Priming
          else
          {
            startPulse(255);
          }
  
          // Update State Machine
          GBL_STATE_MACHINE = STATE_END_DISPENSE;
        }
  
      case STATE_END_DISPENSE:
        {
          int factor = (GBL_button_counter < 200) ? 1 : 10;
          if (millis() >= GBL_state_changed_tstamp_ms + PUMP_PULSE_DUR_MS * factor)
          {
            // Wait Until Button Is release to stop despensing
            if (GBL_button_counter == 0)
            {
              endPulse();
  
              // Update State Machine
              GBL_STATE_MACHINE = STATE_POST_TRIAL_DELAY;
            }
          }
  
          break;
        }
  
  
      // **********************************************
      // Failure State
      // **********************************************
      case STATE_FAILURE:
        {
          if (millis() >= STATE_FAILURE_DELAY_MS + GBL_state_changed_tstamp_ms)
          {
            // Update State Machine
            GBL_STATE_MACHINE = STATE_POST_TRIAL_DELAY;
          }
  
          break;
        }
  
  
      // **********************************************
      // Pause before Next Trial Begins
      // **********************************************
      case STATE_POST_TRIAL_DELAY:
        {
          if (millis() >= GBL_state_changed_tstamp_ms + PRE_TRIAL_DELAY_MS)
          {
            // Reset LEDs
            analogWrite(pinDigital_RwdLED, LOW);
  
            GBL_STATE_MACHINE = STATE_RESET;
          }
          else
          {
            // Keep LEDs On
            analogWrite(pinDigital_RwdLED, RWD_LED_BRIGHTNESS);
          }
  
          break;
        }
  
  
      // **********************************************
      // Reset Trial: Flags, Triggers, and Counters
      // **********************************************
      case STATE_RESET:
        {
          // **********************************************
          // Select Next Target
          GBL_CATCH_TRIAL = false;
          switch (GBL_TRIAL_TYPE)
          {
            case TRIAL_MOVE_JYSTK_LEFT_NoTimeout:
            {
              GBL_TARGET = LEFT;
              break;
            }
            
            case TRIAL_MOVE_JYSTK_RIGHT_NoTimeout:
            {
              GBL_TARGET = RIGHT;
              break;
            }
            
            case TRIAL_MOVE_JYSTK_CNTR_NoTimeout:
            {
              GBL_TARGET = CNTR;
              break;
            }
            
            case TRIAL_MOVE_JYSTK_TOP_L_NoTimeout:
            {
              GBL_TARGET = TOP_L;
              break;
            }
            
            case TRIAL_MOVE_JYSTK_TOP_R_NoTimeout:
            {
              GBL_TARGET = TOP_R;
              break;
            }
            
            default: 
            {
              if (GBL_Adpt_Target_Sel)
              {
                // Set Global Variables
                GBL_CATCH_TRIAL = false;
                GBL_TARGET      = selectNewAdaptiveTarget(true);
              }
      
              // **********************************************
              // RANDOM TARGET PATTERN
              else
              {
                double rand_dbl = (double)random(0, GBL_NUM_TARGETS);
                int new_target = rand_dbl;
      
                // Set Global Variables
                GBL_CATCH_TRIAL = false;
                GBL_TARGET      = new_target;
              }
              
              break;
            }            
          }              
  
          // **********************************************
          // Short Reward Trial To Motivate Rat
          double rand_dbl = (double)random(0, 100) / 100.0;
          double threshold = 1;
          if (GBL_trial_number <= 20)
            threshold = 0.1;
          else
            threshold = 0.8;
  
          if (GBL_TRIAL_TYPE > TRIAL_FIND_STIM_HIDDEN_EqualTimeout)
          {
            if (GBL_TRIAL_TYPE == TRIAL_FIND_STIM_HIDDEN_EqNoShtRwdTr)
              threshold = 1.0;
            else if (GBL_TRIAL_TYPE != TRIAL_FIND_STIM_HIDDEN_EqNoErlyShtRwdTr)
              threshold = 0.8;
          }
          if (GBL_SHORT_RWD_TRIAL)
            GBL_SHORT_RWD_TRIAL = false;
          else
            GBL_SHORT_RWD_TRIAL = rand_dbl > threshold;


          // **********************************************
          // Select Correct/Incorrect Stim Parameters
          
          int minCrLen = 11;
          minCrLen = (GBL_stimCrAmpArrLen > 1)        ? min(GBL_stimCrAmpArrLen,        minCrLen) : minCrLen;
          minCrLen = (GBL_stimCrFreqArrLen > 1)       ? min(GBL_stimCrFreqArrLen,       minCrLen) : minCrLen;
          minCrLen = (GBL_stimCrPulseWidthArrLen > 1) ? min(GBL_stimCrPulseWidthArrLen, minCrLen) : minCrLen;
          minCrLen = (GBL_stimCrPulseNumArrLen > 1)   ? min(GBL_stimCrPulseNumArrLen,   minCrLen) : minCrLen;
          GBL_stimCrRndIdx = (minCrLen != 11) ? random(0, minCrLen) : 0;
                      
          int minIncrLen = 11;
          minIncrLen = (GBL_stimIncrAmpArrLen > 1)         ? min(GBL_stimIncrAmpArrLen,        minIncrLen) : minIncrLen;
          minIncrLen = (GBL_stimIncrFreqArrLen > 1)        ? min(GBL_stimIncrFreqArrLen,       minIncrLen) : minIncrLen;
          minIncrLen = (GBL_stimIncrPulseWidthArrLen > 1)  ? min(GBL_stimIncrPulseWidthArrLen, minIncrLen) : minIncrLen;
          minIncrLen = (GBL_stimIncrPulseNumArrLen > 1)    ? min(GBL_stimIncrPulseNumArrLen,   minIncrLen) : minIncrLen;
          GBL_stimIncrRndIdx = (minIncrLen != 11) ? random(0, minIncrLen) : 0;
         
  
          // **********************************************
          // Reset Trial Variables
          
          // Clear Selected Target
          GBL_Selected_Target = -1;
  
          // Reset Zone Monitors
          GBL_Sipper_LickCounts             = 0;
  
          // Clear all Target Dwell Times / Counters / ETC
          for (int clr_trgt = 0; clr_trgt < GBL_NUM_TARGETS; clr_trgt++)
          {
            GBL_JystkZone_Counts[clr_trgt]       = 0;
            GBL_JystkZone_ValidCounts[clr_trgt]  = 0;
  
            GBL_JystkZone_DwellTime[clr_trgt]      = 0;
            GBL_JystkZone_ValidDwellTime[clr_trgt] = 0;
          }
  
          GBL_handle_reset_flag = false;    // Joystick Reset Flag
          GBL_trial_number += 1;
  
          GBL_STATE_MACHINE = STATE_WAIT_FOR_JY_RSET;
  
          break;
        }
        
        // **********************************************
        // Reset Trial: Flags, Triggers, and Counters
        // **********************************************
        case STATE_ERROR:
          {
            // Turn on "End of Trial" Cue LEDs
            analogWrite(pinDigital_RwdLED, RWD_LED_BRIGHTNESS);
            
            if (db25_ReadDeviceState() && GBL_TRIAL_TYPE >= TRIAL_FIND_STIM_HIDDEN_NoTimeout)
              GBL_STATE_MACHINE = STATE_RESET;
              
            if (ledDriverReadDeviceState() && GBL_TRIAL_TYPE <= TRIAL_FIND_TARGET_HIDDEN_EqualTimeout)
              GBL_STATE_MACHINE = STATE_RESET;
            
            break;
          }
    }
  
    // ********************************************************************************************
    // If Verbose flag is enabled, print out to serial
    // ********************************************************************************************
    if (GBL_verbose)
    {
      Serial.print(verboseSerialOutput1());
      Serial.print(verboseSerialOutput2());
      Serial.println(verboseSerialOutput3());
    }
  
  
    // ********************************************************************************************
    // State Machine Duration
    // ********************************************************************************************
  
    // Time Stamp of Last State Change
    if (prev_GBL_STATE_MACHINE != GBL_STATE_MACHINE)
      GBL_state_changed_tstamp_ms = millis();
  
    // Length of time to complete loop
    GBL_loop_duration_usec = micros() - start_micros;
  
    // Maintain SYS_REFRESH_TIME loop duration
    unsigned long time_elapsed_usec = GBL_loop_duration_usec % 1000;
    unsigned long time_elapsed_msec = (GBL_loop_duration_usec - time_elapsed_usec) / 1000;
  
    if (time_elapsed_usec > 0)
      delayMicroseconds(1000 - time_elapsed_usec);
    if (time_elapsed_msec < SYS_REFRESH_TIME_MS - 1)
      delay(SYS_REFRESH_TIME_MS - 1 - time_elapsed_msec);
  
  }
  

