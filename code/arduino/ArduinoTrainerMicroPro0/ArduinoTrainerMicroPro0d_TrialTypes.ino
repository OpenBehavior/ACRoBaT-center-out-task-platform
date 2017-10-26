
  // **********************************************
  // Trial Helper Functions - Trial Types
  // **********************************************

  boolean runTrial()
  {
    boolean trial_completed = false;
    
    // SIPPER and HANDLE TRIALS **********************************************
    if (GBL_TRIAL_TYPE <= TRIAL_TOUCH_JYSTK)
    {
      GBL_LED_Cues_Overt  = false;
      GBL_LED_Cues_Hidden = false;
      
      switch (GBL_TRIAL_TYPE)
      {
        // SIPPER TRIALS **********************************************
        case TRIAL_SIPPER_20:
          trial_completed = rewardForTouchingSipper(20, GBL_subjectTouchingSipper);  // [ms]
          break;
          
        case TRIAL_SIPPER_COUNTS_10:
          trial_completed = GBL_Sipper_LickCounts > 10;
          break;
          
        case TRIAL_TOUCH_JYSTK:
          trial_completed = rewardForMovingHandle(50);
          break;
      }
      
      if (trial_completed)
        GBL_Selected_Target = GBL_TARGET;
    }
        
    // MOVE JYSTK ANY DIRECTION ********************************************
    else if (GBL_TRIAL_TYPE == TRIAL_MOVE_JYSTK)
    {
      GBL_LED_Cues_Overt  = false;
      GBL_LED_Cues_Hidden = false;
      
      for (int trgt = 0; trgt < GBL_NUM_TARGETS; trgt++)
        if (GBL_JystkZone_DwellTime[trgt] > 50)
        {
          GBL_TARGET = trgt;
          GBL_Selected_Target = trgt;
          trial_completed = true;
        }    
    }
        
    // MOVE JYSTK **********************************************
    else if (GBL_TRIAL_TYPE <= TRIAL_MOVE_JYSTK_TOP_R_NoTimeout)
    {       
      
      GBL_LED_Cues_Overt  = true;
      GBL_LED_Cues_Hidden = false;
        
      trial_completed = GBL_JystkZone_DwellTime[GBL_TARGET] > 50;
      if (trial_completed)
        GBL_Selected_Target = GBL_TARGET;    
    }
        
    // Overt move JYSTK **********************************************
    else if (GBL_TRIAL_TYPE == TRIAL_FIND_TARGET_OVERT_NoTimeout)
    {
      GBL_LED_Cues_Overt  = true;
      GBL_LED_Cues_Hidden = false;
      
      trial_completed = GBL_JystkZone_DwellTime[GBL_TARGET] > 50;
      if (trial_completed)
        GBL_Selected_Target = GBL_TARGET;
    }
        
    else if (GBL_TRIAL_TYPE == TRIAL_FIND_TARGET_OVERT_Timeout)
    {
      GBL_LED_Cues_Overt  = true;
      GBL_LED_Cues_Hidden = false;
      
      for (int trgt = 0; trgt < GBL_NUM_TARGETS; trgt++)
        if (GBL_JystkZone_DwellTime[trgt] > 50)
        {
          GBL_Selected_Target = trgt;
          trial_completed = true;
        }
    }
        
    // Hidden Find Target **********************************************
    else if (GBL_TRIAL_TYPE >= TRIAL_FIND_TARGET_HIDDEN_NoTimeout && 
             GBL_TRIAL_TYPE <= TRIAL_FIND_STIM_HIDDEN_EqualTimeout)
    {
      // If Just Stim
      int temp_trial_type = GBL_TRIAL_TYPE;
      if (temp_trial_type >= TRIAL_FIND_STIM_HIDDEN_NoTimeout)
      {
        temp_trial_type = temp_trial_type - 8;
        GBL_LED_Cues_Hidden = false;
        GBL_LED_Cues_Overt  = false;
        GBL_Adpt_DwellTime  = true;
      }
      // Just LEDs
      else
      {
        GBL_LED_Cues_Hidden = true;
        GBL_LED_Cues_Overt  = false;
        GBL_Adpt_DwellTime  = true;
      }
      

      GBL_DWELL_TIME_MS = 1000;
      GBL_TIMEOUT_TIME_MS = 1000;
      
      switch (temp_trial_type)
      {
        case TRIAL_FIND_TARGET_HIDDEN_NoTimeout:
          GBL_DWELL_TIME_MS = 250;  
          GBL_TIMEOUT_TIME_MS = 30000; 
          break;
        
        case TRIAL_FIND_TARGET_HIDDEN_Timeout_Step1:  
          GBL_DWELL_TIME_MS = 250;
          GBL_TIMEOUT_TIME_MS = 5000; 
          break;
       
        case TRIAL_FIND_TARGET_HIDDEN_Timeout_Step2:
          GBL_DWELL_TIME_MS = 400;  
          GBL_TIMEOUT_TIME_MS = 4000; 
          break;
          
        case TRIAL_FIND_TARGET_HIDDEN_Timeout_Step3:
          GBL_DWELL_TIME_MS = 550;  
          GBL_TIMEOUT_TIME_MS = 3000; 
          break;
          
        case TRIAL_FIND_TARGET_HIDDEN_Timeout_Step4:
          GBL_DWELL_TIME_MS = 700;  
          GBL_TIMEOUT_TIME_MS = 2000; 
          break;
          
        case TRIAL_FIND_TARGET_HIDDEN_Timeout_Step5:
          GBL_DWELL_TIME_MS = 850;  
          GBL_TIMEOUT_TIME_MS = 1750; 
          break;
          
        case TRIAL_FIND_TARGET_HIDDEN_Timeout_Step6:
          GBL_DWELL_TIME_MS = 1000;  
          GBL_TIMEOUT_TIME_MS = 1500; 
          break;
          
        case TRIAL_FIND_TARGET_HIDDEN_EqualTimeout:
          GBL_DWELL_TIME_MS = 1250;  
          GBL_TIMEOUT_TIME_MS = 1250; 
          break;
      }
      
      // Increment dwell time requirement to increase difficulty gradually
      if (GBL_Adpt_DwellTime)
      {
        int rewardCount_incmnt = 5;
        int num_trial_post_20 = max(0, GBL_reward_counter-20);
        int num_levels = round(num_trial_post_20/rewardCount_incmnt);
        
        GBL_DWELL_TIME_MS   = min(1250, GBL_DWELL_TIME_MS + 25*num_levels);
        GBL_TIMEOUT_TIME_MS = max(1250, GBL_TIMEOUT_TIME_MS - 50*num_levels);
      }
      
      // Motivation/Reward Pairing (easy trials to maintain motivation)
      if (GBL_TRIAL_TYPE > TRIAL_FIND_TARGET_HIDDEN_Timeout_Step3 && GBL_SHORT_RWD_TRIAL)
      {
        GBL_DWELL_TIME_MS = round(GBL_DWELL_TIME_MS / 2);
        //GBL_TIMEOUT_TIME_MS = 2000;
      }
      
      // Very short rewards in beginning of trial
      if (GBL_trial_number <= 20)
      {
        GBL_DWELL_TIME_MS = 250;
        GBL_TIMEOUT_TIME_MS = 5000;
      }
        
      for (int trgt = 0; trgt < GBL_NUM_TARGETS; trgt++)
        if ((trgt == GBL_TARGET && GBL_JystkZone_DwellTime[trgt] > GBL_DWELL_TIME_MS) ||
            (trgt != GBL_TARGET && GBL_JystkZone_DwellTime[trgt] > GBL_TIMEOUT_TIME_MS) )
        {          
          GBL_Selected_Target = trgt;
          trial_completed = true;
        }
    }
    
//    // Hidden Find Target **********************************************
//    else if (GBL_TRIAL_TYPE >= TRIAL_FIND_STIM_HIDDEN_EqNoErlyShtRwdTr &&
//             GBL_TRIAL_TYPE <= TRIAL_FIND_STIM_HIDDEN_EqNoShtRwdTr)
//    {
//      GBL_LED_Cues_Hidden = false;
//      GBL_LED_Cues_Overt  = false;
//      GBL_Adpt_DwellTime  = true;
//
//      GBL_DWELL_TIME_MS = 1000;
//      GBL_TIMEOUT_TIME_MS = 1000;
//      
//      switch (GBL_TRIAL_TYPE)
//      {          
//        case TRIAL_FIND_STIM_HIDDEN_EqNoErlyShtRwdTr:
//          GBL_DWELL_TIME_MS = 1250;  
//          GBL_TIMEOUT_TIME_MS = 1250; 
//          break;
//          
//        case TRIAL_FIND_STIM_HIDDEN_EqNoShtRwdTr:
//          GBL_DWELL_TIME_MS = 1250;  
//          GBL_TIMEOUT_TIME_MS = 1250; 
//          break;  
//      }
//      
//      // Increment dwell time requirement to increase difficulty gradually
//      if (GBL_Adpt_DwellTime)
//      {
//        int rewardCount_incmnt = 5;
//        int num_levels = round(GBL_reward_counter/rewardCount_incmnt);
//        
//        GBL_DWELL_TIME_MS   = min(1250, GBL_DWELL_TIME_MS + 25*num_levels);
//        GBL_TIMEOUT_TIME_MS = max(1250, GBL_TIMEOUT_TIME_MS - 25*num_levels);
//      }
//      
//      // Motivation/Reward Pairing in the beginning of each trial
//      if (GBL_TRIAL_TYPE > TRIAL_FIND_STIM_HIDDEN_Timeout_Step3 && GBL_SHORT_RWD_TRIAL)
//      {
//        GBL_DWELL_TIME_MS = 500;
//        //GBL_TIMEOUT_TIME_MS = 2000;
//      }
//              
//      for (int trgt = 0; trgt < GBL_NUM_TARGETS; trgt++)
//        if ((trgt == GBL_TARGET && GBL_JystkZone_DwellTime[trgt] > GBL_DWELL_TIME_MS) ||
//            (trgt != GBL_TARGET && GBL_JystkZone_DwellTime[trgt] > GBL_TIMEOUT_TIME_MS) )
//        {          
//          GBL_Selected_Target = trgt;
//          trial_completed = true;
//        }      
//    }         
    
    // Return Value **********************************************
    return trial_completed;
  }
