
  // **********************************************
  // Motor Functions 
  // **********************************************
  
  boolean PUMP_PWM_EN = false;
  int duty_time = 150;     // 200 will give at 488hz a PPM value of 1600us
  
  // start pulse to peristallic pump motor
  void startPulse(int motorSpeed) 
  {
    PUMP_PWM_EN = true;
    duty_time = motorSpeed;
  }
  
  // end pulse to peristallic pump motor
  void endPulse() 
  {
    PUMP_PWM_EN = false;
    digitalWrite(pinDigital_PUMP_PWM, LOW);   
  }
  
  void initialize_Timer0()
  {
    cli();

    // clear timer 0's TCCRnA register so it works in normal counting mode
    TCCR0A = 0; 

    // set timer interrupt mask register to Enable CTC interrupt with timer 0's comperator A 
    TIMSK0 |= (1<<OCIE0A); 
    
    sei();
  }

  // the register for timer 0's comperator A is OCR0A
  ISR(TIMER0_COMPA_vect)
  {
    static uint8_t state = 0; // we need 4 states .. two high and two low to have a 488Hz Frequency
  
    if (PUMP_PWM_EN)
    {
      switch(state)
      {
        case 0:
          digitalWrite(pinDigital_PUMP_PWM, HIGH);
          //PORTD |= (1<<1); // set pin 1=3 high 
          OCR0A += duty_time;// load the rigister with the duty time .. the interrupt loop will be called again if this time is over 
          break;
        
        case 1:  
          OCR0A += duty_time;// load it again because we simulate a fast PWM mode
          break;
          
        case 3:  
          OCR0A += duty_time;// load it again because we simulate a fast PWM mode
          break;
     
        case 4:
          digitalWrite(pinDigital_PUMP_PWM, LOW);
          //PORTD &= ~(1<<1);// set the pin 3 state to low
          OCR0A += 255-duty_time;// now we neet to wait for the reminding time to dont have just a high signal
          break;
          
        case 5:
          OCR0A += 255-duty_time;// load it again because we simulate a fast PWM mode
          break;
          
        case 6:
          OCR0A += 255-duty_time;// load it again because we simulate a fast PWM mode
          break;
      }
      
      // if state == 3, state = 0, else, state = state + 1
      state = (state == 6) ? 0:state += 1;      
    }
  }
