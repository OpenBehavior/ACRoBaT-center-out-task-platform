
  // **********************************************
  // Helper Functions - Joystick Zone + Rewards
  // **********************************************
    
  // 
  boolean setJoystickZoneRegister(boolean prev_GBL_JystkZone[4], long distance)
  {
    long thres_GBL_y_axis = (GBL_y_axis < 0) ? 0 : GBL_y_axis;
    long c = sqrt(GBL_x_axis*GBL_x_axis + thres_GBL_y_axis*thres_GBL_y_axis);
    
    double rad = atan2((double)abs(thres_GBL_y_axis), (double)GBL_x_axis);
    int deg = rad*180.0/3.14;
    
    if (GBL_NUM_TARGETS == 2)
    {
      GBL_JystkZone[LEFT]   = c > distance && -30 < deg && deg <= 90;
      GBL_JystkZone[RIGHT]  = c > distance &&  90 < deg && deg <= 210;
    }
    else if (GBL_NUM_TARGETS == 3)
    {
      GBL_JystkZone[LEFT]  = c > distance && -30 < deg && deg <= 65;
      GBL_JystkZone[CNTR]  = c > distance &&  65 < deg && deg <= 120;
      GBL_JystkZone[RIGHT] = c > distance && 120 < deg && deg <= 210;
      
//      GBL_JystkZone[LEFT]  = GBL_x_axis > 50;
//      GBL_JystkZone[RIGHT] = GBL_x_axis < -50;
//      GBL_JystkZone[CNTR]  = GBL_y_axis > distance && !GBL_JystkZone[LEFT] && !GBL_JystkZone[RIGHT];
    }
    else if (GBL_NUM_TARGETS == 4)
    {
      GBL_JystkZone[TOP_L] = c > distance && -30 < deg && deg <= 30;
      GBL_JystkZone[LEFT]  = c > distance &&  30 < deg && deg <= 60;
      GBL_JystkZone[CNTR]  = c > distance &&  60 < deg && deg <= 120;
      GBL_JystkZone[RIGHT] = c > distance && 120 < deg && deg <= 210; 
    }
    else if (GBL_NUM_TARGETS == 5)
    {
      GBL_JystkZone[TOP_L] = c > distance && -30 < deg && deg <= 30;
      GBL_JystkZone[LEFT]  = c > distance &&  30 < deg && deg <= 60;
      GBL_JystkZone[CNTR]  = c > distance &&  60 < deg && deg <= 120;
      GBL_JystkZone[RIGHT] = c > distance && 120 < deg && deg <= 150; 
      GBL_JystkZone[TOP_R] = c > distance && 150 < deg && deg <= 210;
    }   
           
    GBL_JystkZone[HOME]  = c <= distance;
    GBL_JystkZone[RSET]  = c < JOYSTICK_RESET_ZONE;
    
    // Set GBL value for output on Serial
    int prev_GBL_Current_Target = GBL_Current_Target;
    GBL_Current_Target = -1;
    for (int trgt = 0; trgt < GBL_NUM_TARGETS; trgt++)
      if (GBL_JystkZone[trgt] == true)
        GBL_Current_Target = trgt;
          
    // If Joystick Moved Zones
    if (GBL_JystkZone[TOP_L] != prev_GBL_JystkZone[TOP_L] ||
        GBL_JystkZone[LEFT]  != prev_GBL_JystkZone[LEFT]  ||
        GBL_JystkZone[CNTR]  != prev_GBL_JystkZone[CNTR]  ||
        GBL_JystkZone[RIGHT] != prev_GBL_JystkZone[RIGHT] ||
        GBL_JystkZone[TOP_R] != prev_GBL_JystkZone[TOP_R] ||
        GBL_JystkZone[HOME]  != prev_GBL_JystkZone[HOME]     )
    {
      GBL_JystkZone_Time = millis();
    }
    
    // Debounce Signal
    if (GBL_JystkZone[HOME] == true && (millis() - GBL_JystkZone_Time <= DWELL_DBOUNCE_MS))
    {
      GBL_Current_Target = prev_GBL_Current_Target;
    }
    
    return true;    
  }
  
  // Select New Target
  int selectNewAdaptiveTarget(boolean updateAdaptDecisBndries)
  {
    // Check Boundaries all add to 1.0
    double total = 0.0; 
    for (int trgt = 0; trgt < GBL_NUM_TARGETS; trgt++)
      total += GBL_Target_History[trgt];
        
    // Real Calculation 
    double slct_dt = 1.0 / ((double)GBL_NUM_TARGETS * 5.0);
    double othr_dt = slct_dt / ((double)GBL_NUM_TARGETS - 1.0);
        
    // Update Target('s) Decision Boundaries             
    if (updateAdaptDecisBndries)
    {
      if (total < 1.0 || total > 1.0)
        for (int trgt = 0; trgt < GBL_NUM_TARGETS; trgt++)
          GBL_Target_History[trgt] += (1.0 - total) / (double)GBL_NUM_TARGETS;
                 
      for (int trgt = 0; trgt < GBL_NUM_TARGETS; trgt++)
      {
        // Adapt on Success
        if (GBL_Selected_Target == GBL_TARGET)
          if (trgt == GBL_Selected_Target)
            GBL_Target_History[trgt] = max(GBL_Target_History[trgt]-slct_dt, 0.0); 
          else
            GBL_Target_History[trgt] = min(GBL_Target_History[trgt]+othr_dt, 1.0);
        
        // Adapt on Failure
        //if (GBL_Selected_Target != GBL_TARGET)
        //  if (trgt == GBL_Selected_Target)
        //    GBL_Target_History[trgt] = min(GBL_Target_History[trgt]+slct_dt, 1.0); 
        //  else
        //    GBL_Target_History[trgt] = max(GBL_Target_History[trgt]-othr_dt, 0.0);
      }
    }
        
    // Variables for Finding Target
    double rand_dbl = (double)random(0, 100) / 100.0;
    double lower_bound = 0;
    int new_target = -1;
    
    // Find Target
    for (int trgt = 0; trgt < GBL_NUM_TARGETS; trgt++)
    {
      if (lower_bound < rand_dbl && rand_dbl <= (lower_bound + GBL_Target_History[trgt]) )
        new_target = trgt;
      
      lower_bound += GBL_Target_History[trgt];
    }
    
    // DEBUG STATEMENTS
//    Serial.print(" Rand#: ");           Serial.print(rand_dbl, 3);
//    Serial.print(" H0: ");              Serial.print(GBL_Target_History[0], 3);
//    Serial.print(" H1: ");              Serial.print(GBL_Target_History[1], 3);
//    Serial.print(" H2: ");              Serial.print(GBL_Target_History[2], 3);
//    Serial.print(" H3: ");              Serial.print(GBL_Target_History[3], 3);
//    Serial.print(" H4: ");              Serial.print(GBL_Target_History[4], 3);
//    Serial.print(" slct_dt: ");         Serial.print(slct_dt, 3);
//    Serial.print(" othr_dt: ");         Serial.print(othr_dt, 3);
//    Serial.print(" total: ");           Serial.print(total, 3);
//    Serial.print(" New Target: ");      Serial.println(new_target);

    // Return Target
    return new_target;
  }
  
  // 
  boolean updateJoystickZoneDwellTimer(boolean prev_GBL_JystkZone[7])
  {    
    // If the handle has been reset
    if (GBL_handle_reset_flag == true)
    {
      for (int trgt = 0; trgt < GBL_NUM_TARGETS; trgt++)
        if (GBL_JystkZone[trgt] == true)
        {
          // Increment Target Dwell Time
          GBL_JystkZone_DwellTime[trgt] += SYS_REFRESH_TIME_MS; 
          
          // Clear all Other Target's Dwell Time
          for (int clr_trgt = 0; clr_trgt < GBL_NUM_TARGETS; clr_trgt++)
          {
            GBL_JystkZone_DwellTime[clr_trgt] = (clr_trgt == trgt) ? GBL_JystkZone_DwellTime[trgt] : 0;
          }
        } 
      
      if (GBL_JystkZone[HOME] == true && (millis() - GBL_JystkZone_Time > DWELL_DBOUNCE_MS))
      {
        // Clear all Other Target's Dwell Time
        for (int clr_trgt = 0; clr_trgt < GBL_NUM_TARGETS; clr_trgt++)
        {
          GBL_JystkZone_DwellTime[clr_trgt] = 0;
        }
      }     
    }
  }
  
  // Reward Function + Global Variable --------------------------
  boolean pullNotCountedYet = false;
  boolean updateJoystickZoneCounts(boolean prev_GBL_JystkZone[7])
  {
                                    
    //        prev_GBL_JystkZone[HOME] |-------__________
    //             GBL_JystkZone[HOME] |------___________
    //             GBL_JystkZone[LEFT] |______-----------
    //               pullNotCountedYet |______----_______
    //                        unlocked |_________--------
    //      GBL_JystkZone_Counts[LEFT] |______-----------
    // GBL_JystkZone_ValidCounts[LEFT] |_________--------
    //                            time |====================>
    //    
    
    // If the handle has been reset 
    if (GBL_handle_reset_flag == true)
    {
      // If Joystick moves out of HOME (towards a target)
      if (prev_GBL_JystkZone[HOME] == true && GBL_JystkZone[HOME] == false)
      {
        for (int trgt = 0; trgt < GBL_NUM_TARGETS; trgt++)
          if (GBL_JystkZone[trgt] == true)
          {
            // Increment Pull Counts
            GBL_JystkZone_Counts[trgt] += 1;
              
            break;
          }  
      }
    }  
    return true;    
  }
  
  // **********************************************
  // Helper Functions - Locking Joystick
  // **********************************************
  
  // Reward Function + Global Variable --------------------------
  boolean rewardForMovingHandle(long distance)
  {    
    long c = sqrt(GBL_x_axis*GBL_x_axis + GBL_y_axis*GBL_y_axis);
    
    if (GBL_handle_reset_flag == true && c > distance)
      return true;

    return false;
  }

  // Reward Function + Global Variable --------------------------
  long counter_sipperlock = 0;
  boolean joystickUnlocked_Sipper(long duration, boolean subjectTouchingSipper)
  {
    if (subjectTouchingSipper == true)
      counter_sipperlock = millis();
      
    long time_elapsed = millis() - counter_sipperlock;
    return (time_elapsed <= duration);       
  }
  
  // Reward Function + Global Variable --------------------------
  long counter_handlelock = 0; 
  boolean joystickUnlocked_Handle(long duration, boolean subjectTouchingHandle)
  {
    if (subjectTouchingHandle == true)
      counter_handlelock = millis();
      
    long time_elapsed = millis() - counter_handlelock;
    return (time_elapsed <= duration);       
  }
  
  // Reward Function + Global Variable --------------------------
  long counter_handle = 0;
  boolean rewardForTouchingHandle(long duration, boolean subjectTouchingHandle)
  {
    if (subjectTouchingHandle == true)
    {
      long time_elapsed = millis() - counter_handle;
      if (time_elapsed >= duration) 
      {
        counter_handle = millis() + 1000;
        return true;  
      }
    }
    else
      counter_handle = max(counter_handle, millis());

    return false;
  }
  
  // Reward Function + Global Variable --------------------------
  long counter_sipper = 0;     
  boolean rewardForTouchingSipper(long duration, boolean subjectTouchingSipper)
  {
    if (subjectTouchingSipper == true)
    {
      long time_elapsed = millis() - counter_sipper;
      if (time_elapsed >= duration) 
      {
        counter_sipper = millis() + 1000;
        return true;  
      }
    }
    else
      counter_sipper = max(counter_sipper, millis());

    return false;
  }
  
