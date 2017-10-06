
  // **********************************************
  // Serial Comm Functions 
  // **********************************************
                                           
  String verboseSerialOutput1()
  {    
    String output = String(GBL_trial_number, DEC) + ", " +
                    String(GBL_STATE_MACHINE, DEC) + ", " +
                    String(GBL_reward_counter, DEC) + ", " +     
                    
                    String(GBL_Target_History[0], 2) + ", " +
                    
                    String(GBL_DWELL_TIME_MS, DEC) + ", " +
                    String(GBL_TIMEOUT_TIME_MS, DEC) + ", " +
                    
                    String(GBL_Current_Target, DEC) + ", " +
                    String((GBL_Current_Target < 0) ? 0:GBL_JystkZone_DwellTime[GBL_Current_Target], DEC) + ", " +
                                       
                    String(GBL_cue_leds_enabled, BIN) + ", " +
                    String(GBL_stimulation_enabled, BIN) + ", ";
    return output;
  }
                    
  String verboseSerialOutput2()
  {
    String output = String(GBL_TARGET, DEC) + ", " +
                    String(GBL_Selected_Target, DEC) + ", " +
                    String(GBL_SHORT_RWD_TRIAL, BIN) + ", " +
                    
                    String(GBL_buttonPressed, BIN) + ", " +
                    String(GBL_subjectTouchingHandle, BIN) + ", " +
                    String(GBL_subjectTouchingSipper, BIN) + ", " +
                    
                    String(GBL_x_axis, DEC) + ", " +
                    String(GBL_y_axis, DEC) + ", " +
                    
                    String(GBL_loop_duration_usec, DEC) + ", " +
                    String(millis(), DEC);

    return output;
  }
  
  String verboseSerialOutput3()
  {
    String output = ", " + String(GBL_stimAmp_ua, DEC) + ", " +
                    String(GBL_stimFreq_hz, DEC) + ", " +
                    String(GBL_stimPulseWidth_us, DEC) + ", " +
                    String(GBL_stimNumPulses_int, DEC) + ", " +
                    String(GBL_stimCarrierFreq_hz, DEC) + ", " +
                    String(GBL_stimElectrode_int, DEC) + ", " +
                    String(GBL_stimTrialNum_int, DEC) + ", " +
                    
                    String(GBL_stimParamReadEnable, BIN) + ", " +
                    String(GBL_stimEnable, BIN);

    return output;
  }

  void parseRXSerial(String rxSerialString)
  {
    // Echo Serial Command 
    if (GBL_serial_mirror)
      Serial.println("\"" + rxSerialString + "\"");
          
    String name = "";
    String value_str = "";
    int value = 0;
 
    for (int i = 0; i < rxSerialString.length(); i++) 
      if (rxSerialString.substring(i, i+1) == ",") 
      {
        name = rxSerialString.substring(0, i);
        value_str = rxSerialString.substring(i+1);
        value = value_str.toInt();       
        break;
      }
        
    // ********************************************** // **********************************************
    // Legacy Commands, Version 1.0.0 -> 5.3.0
    // ********************************************** // **********************************************
    
    // **********************************************
    // DEBUG COMMANDS: Verbose Output
    if (name.equals("verbose"))
      GBL_verbose = value; 
    
    // **********************************************
    // Confirm Ard
    else if (name.equals("identity"))
      Serial.println(String("Arduino Remote Trainer: " + String(millis())));
      
    // **********************************************
    // Get Data
    else if (name.equals("all"))
    {
      Serial.print(verboseSerialOutput1());
      Serial.print(verboseSerialOutput2());
      Serial.println(verboseSerialOutput3());
    }
    
    // **********************************************
    // Version Variables
    else if (name.equals("version"))
      Serial.println(CODE_VERSION);
                  
    // **********************************************
    // Mirror Variables
    else if (name.equals("mirror"))
      GBL_serial_mirror = value;
  
    // **********************************************
    // Data Value Variables
    else if (name.equals("data_list") && value == 0)
    {
      Serial.println( F ( "GBL_trial_number, " 
                          "GBL_STATE_MACHINE, "  
                          "GBL_reward_counter, "
                         
                          "GBL_Decis_Bound, "
                         
                          "GBL_DWELL_TIME_MS, "
                          "GBL_TIMEOUT_TIME_MS, "
                         
                          "GBL_Current_Target, " 
                          "GBL_JystkZone_DwellTime, "
                         
                          "GBL_cue_leds_enabled, "
                          "GBL_stimulation_enabled, ") );
    }
    
    else if (name.equals("data_list") && value == 1)                    
    {
      Serial.println( F ( "GBL_TARGET, "
                          "GBL_Selected_Target, "
                          "GBL_SHORT_RWD_TRIAL, "
                         
                          "GBL_buttonPressed,  "
                          "GBL_subjectTouchingHandle, "
                          "GBL_subjectTouchingSipper, "
                          
                          "GBL_x_axis, "
                          "GBL_y_axis, "
                         
                          "GBL_loop_duration_usec, "
                          "time_tstamp_msec" ) );   
    }
    
    else if (name.equals("data_list") && value == 2)                    
    {
      Serial.println( F ( ", GBL_stimAmp_ua, "
                          "GBL_stimFreq_hz, "
                          "GBL_stimPulseWidth_us, "
                          "GBL_stimNumPulses_int,  "
                          "GBL_stimCarrierFreq_hz, "
                          "GBL_stimElectrode_int, "
                          "GBL_stimTrialNum_int, "

                          "GBL_stimParamReadEnable, "                         
                          "GBL_stimEnable") );   
    }
                    
    // ********************************************** // **********************************************
    // Version 6.x.x
    // ********************************************** // **********************************************
    
    // **********************************************
    // Trial Type Variables
    else if (name.equals("trial_list"))
    {
      Serial.println( F ( "HEADER, 29" ) );
      
      Serial.println( F ( "TRIAL_SIPPER_20" ) );
      Serial.println( F ( "TRIAL_SIPPER_COUNTS_10" ) );
      
      Serial.println( F ( "TRIAL_TOUCH_JYSTK" ) );
      
      Serial.println( F ( "TRIAL_MOVE_JYSTK" ) );
      Serial.println( F ( "TRIAL_MOVE_JYSTK_LEFT_NoTimeout" ) );
      Serial.println( F ( "TRIAL_MOVE_JYSTK_RIGHT_NoTimeout" ) );
      Serial.println( F ( "TRIAL_MOVE_JYSTK_CNTR_NoTimeout" ) );
      Serial.println( F ( "TRIAL_MOVE_JYSTK_TOP_L_NoTimeout" ) );
      Serial.println( F ( "TRIAL_MOVE_JYSTK_TOP_R_NoTimeout" ) );
      
      Serial.println( F ( "TRIAL_FIND_TARGET_OVERT_NoTimeout" ) );
      Serial.println( F ( "TRIAL_FIND_TARGET_OVERT_Timeout" ) );
      
      Serial.println( F ( "TRIAL_FIND_TARGET_HIDDEN_NoTimeout" ) );
      Serial.println( F ( "TRIAL_FIND_TARGET_HIDDEN_Timeout_Step1" ) );
      Serial.println( F ( "TRIAL_FIND_TARGET_HIDDEN_Timeout_Step2" ) );
      Serial.println( F ( "TRIAL_FIND_TARGET_HIDDEN_Timeout_Step3" ) );
      Serial.println( F ( "TRIAL_FIND_TARGET_HIDDEN_Timeout_Step4" ) );
      Serial.println( F ( "TRIAL_FIND_TARGET_HIDDEN_Timeout_Step5" ) );
      Serial.println( F ( "TRIAL_FIND_TARGET_HIDDEN_Timeout_Step6" ) );
      Serial.println( F ( "TRIAL_FIND_TARGET_HIDDEN_EqualTimeout" ) );
      
      Serial.println( F ( "TRIAL_FIND_STIM_HIDDEN_NoTimeout" ) );
      Serial.println( F ( "TRIAL_FIND_STIM_HIDDEN_Timeout_Step1" ) );
      Serial.println( F ( "TRIAL_FIND_STIM_HIDDEN_Timeout_Step2" ) );
      Serial.println( F ( "TRIAL_FIND_STIM_HIDDEN_Timeout_Step3" ) );
      Serial.println( F ( "TRIAL_FIND_STIM_HIDDEN_Timeout_Step4" ) );
      Serial.println( F ( "TRIAL_FIND_STIM_HIDDEN_Timeout_Step5" ) );
      Serial.println( F ( "TRIAL_FIND_STIM_HIDDEN_Timeout_Step6" ) );
      Serial.println( F ( "TRIAL_FIND_STIM_HIDDEN_EqualTimeout" ) );
      
      Serial.println( F ( "TRIAL_FIND_STIM_HIDDEN_EqNoErlyShtRwdTr" ) );
      Serial.println( F ( "TRIAL_FIND_STIM_HIDDEN_EqNoShtRwdTr" ) );     
    }

    else if (name.equals("get_trial_type"))
      Serial.println(GBL_TRIAL_TYPE);
      
    else if (name.equals("set_trial_type"))
      GBL_TRIAL_TYPE = value;
      
    // **********************************************
    // Explicit Serial Commands                                      
    else if (name.equals("var_list"))
      Serial.println( F ( "GBL_verbose, "
                          "GBL_serial_mirror, "
                          "GBL_LED_Cues_Hidden, "
                          "GBL_LED_Cues_Overt, "
                          "GBL_Adpt_Target_Sel, "
                          "GBL_VarDwellTime" ) );  
      
    else if (name.equals("get_var_data"))
    {
      String output = String(GBL_verbose, BIN) +
                      String(GBL_serial_mirror, BIN) +
                      String(GBL_LED_Cues_Hidden, BIN) +
                      String(GBL_LED_Cues_Overt, BIN) +
                      String(GBL_Adpt_Target_Sel, BIN) +
                      String(GBL_VarDwellTime, BIN);
      Serial.println(output);               
    }
      
    else if (name.equals("set_var_data"))
    {
      if (value_str.length() == 6)
      {
        if (value_str[0] != 'x') GBL_verbose           = (int)value_str[0] - 48;
        if (value_str[1] != 'x') GBL_serial_mirror     = (int)value_str[1] - 48;
        if (value_str[2] != 'x') GBL_LED_Cues_Hidden   = (int)value_str[2] - 48;
        if (value_str[3] != 'x') GBL_LED_Cues_Overt    = (int)value_str[3] - 48;
        if (value_str[4] != 'x') GBL_Adpt_Target_Sel   = (int)value_str[4] - 48;
        if (value_str[5] != 'x') GBL_VarDwellTime      = (int)value_str[5] - 48;
      }        
    }

    // ********************************************** // **********************************************
    // Version 7.0.0+
    // ********************************************** // **********************************************      
    
    else if (name.equals("set_num_targets"))
    {
      GBL_NUM_TARGETS = value;
    }
    
    else if (name.equals("get_num_targets"))
    {
      Serial.println(GBL_NUM_TARGETS);
    }    
    
    // ********************************************** // **********************************************
    // Version 12.0.0+
    // ********************************************** // **********************************************
  
    else if (name.substring(0, 8).equals("set_stim"))
    { 
      int valstr_idx = 0;
      int value_idx = 0;
      int GBL_stimParamArray[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
   
      value_str = value_str + " ";
      for (int i = 0; i < value_str.length(); i++) 
        if ( (value_str.substring(i, i+1) == ",") || 
             (valstr_idx != i && i == value_str.length()-1) )
        {          
          GBL_stimParamArray[value_idx] = value_str.substring(valstr_idx, i).toInt();
          value_idx += 1;
          valstr_idx = i+1;
        }            
          
      // If Only one value is entered, repeat it for all instances
      if (value_idx == 1)
        for (int i = 1; i < 10; i++)
          GBL_stimParamArray[i] = GBL_stimParamArray[0];
          
      // DEBUG
//      for (int i = 0; i < value_idx; i++) 
//        Serial.println(String(GBL_stimParamArray[i], DEC));
//      Serial.println(value_idx);
       
      
      if (name.equals("set_stim_amp_cr"))
      {
        GBL_stimCrAmpArrLen = value_idx;
        memcpy(GBL_stimCrAmpArray, GBL_stimParamArray, 10 * sizeof(int));
      }
      
      else if (name.equals("set_stim_amp_incr"))
      {
        GBL_stimIncrAmpArrLen = value_idx;
        memcpy(GBL_stimIncrAmpArray, GBL_stimParamArray, 10 * sizeof(int));
      }
      
      else if (name.equals("set_stim_freq_cr"))
      {
        GBL_stimCrFreqArrLen = value_idx;
        memcpy(GBL_stimCrFreqArray, GBL_stimParamArray, 10 * sizeof(int));
      }
      
      else if (name.equals("set_stim_freq_incr"))
      {
        GBL_stimIncrFreqArrLen = value_idx;
        memcpy(GBL_stimIncrFreqArray, GBL_stimParamArray, 10 * sizeof(int));
      }
      
      else if (name.equals("set_stim_pulsewidth_cr"))
      {
        GBL_stimCrPulseWidthArrLen = value_idx;
        memcpy(GBL_stimCrPulseWidthArray, GBL_stimParamArray, 10 * sizeof(int)); 
      }
      
      else if (name.equals("set_stim_pulsewidth_incr"))
      {
        GBL_stimIncrPulseWidthArrLen = value_idx;
        memcpy(GBL_stimIncrPulseWidthArray, GBL_stimParamArray, 10 * sizeof(int)); 
      }
      
      else if (name.equals("set_stim_pulsenum_cr"))
      {
        GBL_stimCrPulseNumArrLen = value_idx;
        memcpy(GBL_stimCrPulseNumArray, GBL_stimParamArray, 10 * sizeof(int)); 
      }
      
      else if (name.equals("set_stim_pulsenum_incr"))
      {
        GBL_stimIncrPulseNumArrLen = value_idx; 
        memcpy(GBL_stimIncrPulseNumArray, GBL_stimParamArray, 10 * sizeof(int)); 
      }
      
      else if (name.equals("set_stim_carrierfreq"))
      {
        GBL_stimCarrierFreq_hz = value;
      }
      
      else if (name.equals("set_stim_electrode"))
      {
        GBL_stimElectrode_int = value;
      }
    }
    
    
    
  }
  
  
  
  
