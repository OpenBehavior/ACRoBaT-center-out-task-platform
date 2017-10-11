

  // **********************************************
  // MICRO PRO Arduino Trainer
  // 
  // Trial Type: TRIAL_MOVE_JYSTK
  // Number of Targets: 3
  // **********************************************  
  
  const char CODE_VERSION[] = "V12.3.2";
  
  
  // **********************************************
  // INCLUDE LIBRARIES
  // LINUX: /usr/share/arduino/libraries/
  // WINDOWS: ../My Documents\Arduino\libraries
  // **********************************************

  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include <avr/wdt.h>
  #include <util/delay.h>
  #include <avr/pgmspace.h>
  #include <math.h>
  #include <SPI.h>
  #include  <util/parity.h>
  
  
  // **********************************************
  // Serial Variables to be set    
  // **********************************************

  // Set By Serial Interface
  boolean GBL_verbose           = false;
  boolean GBL_serial_mirror     = true;
  boolean GBL_LED_Cues_Hidden   = false;
  boolean GBL_LED_Cues_Overt    = false;
  boolean GBL_Adpt_Target_Sel   = true;     // If false, random target selection
  boolean GBL_VarDwellTime      = false;    // TODO: Not Implemented Yet
  
                                            // TODO: Not Implemented in Serial Interface
  boolean GBL_Adpt_DwellTime    = true;     // trial_type variable for gradually incrementing dwell time requirement
  
  int GBL_TRIAL_TYPE  = -1;
  int GBL_NUM_TARGETS = 2;
  
  
  // **********************************************
  // PIN DEFINITIONS 
  // **********************************************
  
  //const int pinDigital_UART_TX         = 1;
  //const int pinDigital_UART_RX         = 0;  
  
  const int pinDigital_STIM_ENABLE     = 2;
  const int pinDigital_PUMP_PWM        = 3;
  const int pinAnalog_JoystickY        = A6;  // Pin 4
  const int pinDigital_STIM_PWM        = 5;  
  const int pinAnalog_JoystickX        = A7;  // Pin 6
  const int pinDigital_CapSen_Handle   = 7;
  const int pinDigital_CapSen_Sipper   = 8;
  const int pinDigital_CapSen_PWR      = 9;
  
  const int pinDigital_RwdLED          = 10;
  const int pinDigital_MOSI            = 16;
  const int pinDigital_MISO            = 14;
  const int pinDigital_SCK             = 15;
  const int pinDigital_LEDDriverSS     = 18;  // A0
  const int pinDigital_DB25DriverSS    = 19;  // A1
  const int pinDigital_PushButtonGND   = 20;  // A2
  const int pinDigital_PushButton      = 21;  // A3
    
  // **********************************************
  // CONST DEFINITIONS 
  // **********************************************
  
  // Targets
  const int LEFT    = 0;
  const int RIGHT   = 1;
  const int CNTR    = 2;
  const int TOP_L   = 3;
  const int TOP_R   = 4;
  const int HOME    = 5;
  const int RSET    = 6;
  
  // Zone Definitions
  const int TARGET_THRESHOLD_DIST  = 201;   // [px]
  const int CUE_LED_THRESHOLD_DIST = 350;   // [px]
  const int JOYSTICK_RESET_ZONE    = 100;   // [px]
  
  const int SYS_REFRESH_TIME_MS    = 10;     // [ms]
  
  const int PUMP_PULSE_DUR_MS      = 100;    // [ms]
  const int TRIAL_REWARD_DELAY_MS  = 100;    // [ms]
  
  // "Timeout" Period for Failure
  const int STATE_FAILURE_DELAY_MS = 5000;  // [ms]
 
  // Dwell Time Required for "Selection" of Target
  const int DWELL_DBOUNCE_MS            = 200;    // [ms] 
  const int DWELL_TIME_TARGET_SELECTION = 50;     // [ms]
  int VAR_DWELL_TIME_TARGET_SELECTION   = 1000;   // [ms]
  
  // Delay between Trials
  const int PRE_TRIAL_DELAY_MS     = 2000;  // [ms]
  
  // Minimum reset time for the handle to be still before trial 
  const int RSET_MIN_TIME_MS       = 1000;   // [ms]
    
  // Sets Brightness of the LEDs
  const int RWD_LED_BRIGHTNESS     = 31;
  const int CUE_LED_BRIGHTNESS     = 31;
    
  
  // **********************************************
  // Trial Types   
  // **********************************************
  
  int GBL_trial_number = 0;
    
  const int TRIAL_SIPPER_20                        = 1;
  const int TRIAL_SIPPER_COUNTS_10                 = 2;  
  
  const int TRIAL_TOUCH_JYSTK                      = 3;
  
  const int TRIAL_MOVE_JYSTK                       = 4;
  const int TRIAL_MOVE_JYSTK_LEFT_NoTimeout        = 5;
  const int TRIAL_MOVE_JYSTK_RIGHT_NoTimeout       = 6;
  const int TRIAL_MOVE_JYSTK_CNTR_NoTimeout        = 7;
  const int TRIAL_MOVE_JYSTK_TOP_L_NoTimeout       = 8;
  const int TRIAL_MOVE_JYSTK_TOP_R_NoTimeout       = 9;
  
  const int TRIAL_FIND_TARGET_OVERT_NoTimeout      = 10;
  const int TRIAL_FIND_TARGET_OVERT_Timeout        = 11;
  
  const int TRIAL_FIND_TARGET_HIDDEN_NoTimeout     = 12;
  const int TRIAL_FIND_TARGET_HIDDEN_Timeout_Step1 = 13;
  const int TRIAL_FIND_TARGET_HIDDEN_Timeout_Step2 = 14;
  const int TRIAL_FIND_TARGET_HIDDEN_Timeout_Step3 = 15;
  const int TRIAL_FIND_TARGET_HIDDEN_Timeout_Step4 = 16;
  const int TRIAL_FIND_TARGET_HIDDEN_Timeout_Step5 = 17;
  const int TRIAL_FIND_TARGET_HIDDEN_Timeout_Step6 = 18;
  const int TRIAL_FIND_TARGET_HIDDEN_EqualTimeout  = 19;
  
  const int TRIAL_FIND_STIM_HIDDEN_NoTimeout       = 20;
  const int TRIAL_FIND_STIM_HIDDEN_Timeout_Step1   = 21;
  const int TRIAL_FIND_STIM_HIDDEN_Timeout_Step2   = 22;
  const int TRIAL_FIND_STIM_HIDDEN_Timeout_Step3   = 23;
  const int TRIAL_FIND_STIM_HIDDEN_Timeout_Step4   = 24;
  const int TRIAL_FIND_STIM_HIDDEN_Timeout_Step5   = 25;
  const int TRIAL_FIND_STIM_HIDDEN_Timeout_Step6   = 26;
  const int TRIAL_FIND_STIM_HIDDEN_EqualTimeout    = 27;
  
  const int TRIAL_FIND_STIM_HIDDEN_EqNoErlyShtRwdTr = 28;
  const int TRIAL_FIND_STIM_HIDDEN_EqNoShtRwdTr     = 29;

  
  // **********************************************
  // Main Loop - State Machine  
  // **********************************************
  
  int GBL_STATE_MACHINE = -1;
  const int STATE_WAIT_FOR_JY_RSET = 0;
  const int STATE_RUNNING          = 1;
  const int STATE_COMPLETE         = 2;
  const int STATE_START_DISPENSE   = 3;
  const int STATE_END_DISPENSE     = 4;
  const int STATE_RESET            = 5;
  const int STATE_FAILURE          = 6;
  const int STATE_POST_TRIAL_DELAY = 7;
  const int STATE_ERROR            = 666;
    
  unsigned long GBL_state_changed_tstamp_ms = 0;    // [ms]
  
  
  // **********************************************
  // Deliever Stimulaton - State Machine  
  // **********************************************
     
  int GBL_STIMSTATE_MACHINE = -1;
  
  const int STIMSTATE_ON    = 1;
  const int STIMSTATE_OFF   = 0;
   
  
  // **********************************************
  // Exposed System Varibles
  // **********************************************
  
  boolean GBL_CATCH_TRIAL      = false;
  boolean GBL_SHORT_RWD_TRIAL  = false;
  
  int GBL_TARGET           = 0;
  int GBL_Current_Target   = -1;
  int GBL_Selected_Target  = -1;
  int GBL_reward_counter   = 0; 
  
  int GBL_DWELL_TIME_MS    = 1000;
  int GBL_TIMEOUT_TIME_MS  = 1000;
  
  int  GBL_JystkZone_Counts[5]         = {0,0,0,0,0};  // #[left,right,cntr,top_L,top_R pulls]
  int  GBL_JystkZone_ValidCounts[5]    = {0,0,0,0,0};  // #[left,right,cntr,top_L,top_R pulls]
  long GBL_JystkZone_DwellTime[5]      = {0,0,0,0,0};  // #[left,right,cntr,top_L,top_R dwell]
  long GBL_JystkZone_ValidDwellTime[5] = {0,0,0,0,0};  // #[left,right,cntr,top_L,top_R dwell]
  
  boolean GBL_stimulation_enabled = false;  
  boolean GBL_cue_leds_enabled    = false;
  
  boolean GBL_buttonPressed         = false;
  boolean GBL_subjectTouchingSipper = false;
  boolean GBL_subjectTouchingHandle = false;
  
  long GBL_x_axis = 0;
  long GBL_y_axis = 0;
  
  unsigned long GBL_loop_duration_usec = 0;
  
  // Stimulation Parameters
  int GBL_stimAmp_ua         = 70;            // Byte C.0
  int GBL_stimFreq_hz        = 300;           // Byte C.1
  int GBL_stimPulseWidth_us  = 200;           // Byte C.2
  int GBL_stimNumPulses_int  = 5;             // Byte C.3
  int GBL_stimCarrierFreq_hz = 100;           // Byte C.4
  int GBL_stimElectrode_int  = 5;             // Byte C.5
  int GBL_stimTrialNum_int   = 1;             // Byte C.0 & C.1
  
  boolean GBL_stimParamReadEnable = false;    // Byte C.6
  boolean GBL_stimEnable          = false;    // Byte C.7
  
    
  // **********************************************
  // Internal System Variables
  // **********************************************
  
  int GBL_button_counter       = 0;
  int GBL_Sipper_LickCounts    = 0;  
  
  boolean GBL_handle_reset_flag = false;
  
  unsigned long GBL_subjectTouchingSipper_Time = 0;  // time in [ms] since value changed
  unsigned long GBL_subjectTouchingHandle_Time = 0;  // time in [ms] since value changed
  
  unsigned long GBL_JystkZone_Time     = 0;                 // time in [ms] since zone last changed
  boolean GBL_JystkZone[7]             = {0,0,0,0,0,1,1};   // [left] [right] [center] [top_left] [top_right] [home] [reset zone]
     
  // N Target Adaptive Selective Variables
  double GBL_Target_History[5]         = {0.5, 0.5, 0.5, 0.5, 0.5}; // %[correct of left] %[correct of right]
  
  long GBL_x_axis_home = 0;
  long GBL_y_axis_home = 0; 
  
  // Stim Parameter Choices
  int GBL_stimCrAmpArray[10]   = {70,-1,-1,-1,-1,-1,-1,-1,-1,-1};
  int GBL_stimCrAmpArrLen      = 1;
  int GBL_stimIncrAmpArray[10] = {0,-1,-1,-1,-1,-1,-1,-1,-1,-1};
  int GBL_stimIncrAmpArrLen    = 1;
  
  int GBL_stimCrFreqArray[10]   = {300,-1,-1,-1,-1,-1,-1,-1,-1,-1};
  int GBL_stimCrFreqArrLen      = 1;
  int GBL_stimIncrFreqArray[10] = {0,-1,-1,-1,-1,-1,-1,-1,-1,-1};
  int GBL_stimIncrFreqArrLen    = 1;

  int GBL_stimCrPulseWidthArray[10]   = {200,-1,-1,-1,-1,-1,-1,-1,-1,-1};
  int GBL_stimCrPulseWidthArrLen      = 1;
  int GBL_stimIncrPulseWidthArray[10] = {0,-1,-1,-1,-1,-1,-1,-1,-1,-1};
  int GBL_stimIncrPulseWidthArrLen    = 1;
  
  int GBL_stimCrPulseNumArray[10]   = {5,-1,-1,-1,-1,-1,-1,-1,-1,-1};
  int GBL_stimCrPulseNumArrLen      = 1;
  int GBL_stimIncrPulseNumArray[10] = {5,-1,-1,-1,-1,-1,-1,-1,-1,-1};
  int GBL_stimIncrPulseNumArrLen    = 1;

  int GBL_stimCrRndIdx = 0;
  int GBL_stimIncrRndIdx = 0;

  
  
  
  
  
