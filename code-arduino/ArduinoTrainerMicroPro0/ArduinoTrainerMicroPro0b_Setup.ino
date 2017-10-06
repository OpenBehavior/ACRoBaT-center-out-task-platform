
  // **********************************************
  // Setup Functions 
  // **********************************************
  
  void setup() 
  {        
    // Initialize Random Number Generator
    randomSeed(1);
        
    // Initalize Serial Connection
    Serial.begin(115200);
    
    // Initialize SPI bus
    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
    SPI.endTransaction();
    
    // Initalize LED Array
    pinMode(pinDigital_LEDDriverSS, OUTPUT); 
    digitalWrite(pinDigital_LEDDriverSS, HIGH);
    setupLEDDriver();
    bootupLEDSequence();
    
    // Initalize DB25 Array
    pinMode(pinDigital_DB25DriverSS, OUTPUT); 
    digitalWrite(pinDigital_DB25DriverSS, HIGH);
    setupDB25Driver();
    db25BootupSequence();
    
    
    // Initalize Joystick
    pinMode(pinAnalog_JoystickX, INPUT);
    pinMode(pinAnalog_JoystickY, INPUT);
    
    
    // Read From Joystick
    delay(100);
    GBL_x_axis_home = analogRead(pinAnalog_JoystickX);
     
    delay(100);
    GBL_y_axis_home = analogRead(pinAnalog_JoystickY); 
        
    
    // Initialize Manual Reward Trigger
    pinMode(pinDigital_PushButton, INPUT_PULLUP);
    pinMode(pinDigital_PushButtonGND, OUTPUT);
    digitalWrite(pinDigital_PushButtonGND, LOW);
    
    
    // Initalize LEDs
    pinMode(pinDigital_RwdLED, OUTPUT); 
    
    
    // Initalize Capacitive Sensor Inputs
    pinMode(pinDigital_CapSen_Handle, INPUT);
    pinMode(pinDigital_CapSen_Sipper, INPUT);
    pinMode(pinDigital_CapSen_PWR, OUTPUT);
    digitalWrite(pinDigital_CapSen_PWR, LOW);
    delay(100);
    digitalWrite(pinDigital_CapSen_PWR, HIGH);
        
    
    // Initalize Pump Motor Driver
    pinMode(pinDigital_PUMP_PWM, OUTPUT);
    initialize_Timer0();
    
    
    // Initalize Timers for Stimulate Pulse Generator
    pinMode(pinDigital_STIM_ENABLE, OUTPUT);
    pinMode(pinDigital_STIM_PWM, OUTPUT);
            
            
    //Initialize Stim State Machine
    GBL_STIMSTATE_MACHINE = STIMSTATE_OFF;
    
    
    // Initalize STATE MACHINE, TRIAL_TYPE
    GBL_trial_number  = 1;
    GBL_STATE_MACHINE = STATE_WAIT_FOR_JY_RSET;
       
    GBL_TARGET        = LEFT;  //REWARD_SCHEDULE_1[GBL_Target_Index];
    GBL_CATCH_TRIAL   = false; //CATCH_TR_SCHEDULE[GBL_Target_Index];
    
    
     // Trial Type
    GBL_TRIAL_TYPE = TRIAL_FIND_TARGET_OVERT_NoTimeout; 
    

    // ******************** ALL TRIAL TYPES ************************
    //    TRIAL_SIPPER_20
    //    TRIAL_SIPPER_100
    //    TRIAL_SIPPER_COUNTS_10
    
    //    TRIAL_HANDLE_20
    //    TRIAL_HANDLE_100
    
    //    TRIAL_MOVE_JYSTK
    //    TRIAL_MOVE_JYSTK_LEFT_NoTimeout
    //    TRIAL_MOVE_JYSTK_RIGHT_NoTimeout
    //    TRIAL_MOVE_JYSTK_CNTR_NoTimeout
    //    TRIAL_MOVE_JYSTK_TOP_L_NoTimeout
    //    TRIAL_MOVE_JYSTK_TOP_R_NoTimeout
    
    //    TRIAL_FIND_TARGET_OVERT_NoTimeout
    //    TRIAL_FIND_TARGET_OVERT_Timeout
    
    //    TRIAL_FIND_TARGET_HIDDEN_NoTimeout
    //    TRIAL_FIND_TARGET_HIDDEN_Timeout_Step1
    //    TRIAL_FIND_TARGET_HIDDEN_Timeout_Step2
    //    TRIAL_FIND_TARGET_HIDDEN_Timeout_Step3
    //    TRIAL_FIND_TARGET_HIDDEN_Timeout_Step4
    //    TRIAL_FIND_TARGET_HIDDEN_Timeout_Step5
    //    TRIAL_FIND_TARGET_HIDDEN_Timeout_Step6
    //    TRIAL_FIND_TARGET_HIDDEN_EqualTimeout
    
    //    TRIAL_FIND_STIM_HIDDEN_NoTimeout
    //    TRIAL_FIND_STIM_HIDDEN_Timeout_Step1
    //    TRIAL_FIND_STIM_HIDDEN_Timeout_Step2
    //    TRIAL_FIND_STIM_HIDDEN_Timeout_Step3
    //    TRIAL_FIND_STIM_HIDDEN_Timeout_Step4
    //    TRIAL_FIND_STIM_HIDDEN_Timeout_Step5
    //    TRIAL_FIND_STIM_HIDDEN_Timeout_Step6
    //    TRIAL_FIND_STIM_HIDDEN_EqualTimeout
    //    TRIAL_FIND_STIM_HIDDEN_EqNoErlyShtRwdTr = 30;
    //    TRIAL_FIND_STIM_HIDDEN_EqNoShtRwdTr    = 31;
    
    //    TRIAL_FIND_STIM_SingleTap_toStart      = 32;
    //    TRIAL_FIND_STIM_TapAllTrgt_toStart     = 33;
    //      
    //    TRIAL_FIND_STIM_SingleTap_GenNewTrgt_toStart = 34;
    //    TRIAL_FIND_STIM_SingleTap_GenNewTrgt10_toStart = 35;
    //    TRIAL_FIND_STIM_SingleTap_GenNewTrgt20_toStart = 36;
    //    TRIAL_FIND_STIM_SingleTap_GenNewTrgt50_toStart = 37;   
    
    // DELAY for Everything to be Initialized
    delay(1000);    
  }
