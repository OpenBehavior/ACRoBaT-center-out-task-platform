
  // **********************************************
  // SPI - LED Driver (MAX6966) Constants
  // **********************************************
  
  // Set High: (1<<BIT0) |  (1<<BIT5) = 0b00100001
  // Set Low: ~(1<<BIT0) & ~(1<<BIT5) = 0boo0oooo0
  
  //Configuration Register
  //D7 = 0, DOUT = MISO, Internal Oscilator drives PWM
  //D6 = 0, Don't Care
  //D5 = 1, PWM Stagger = in phase
  //D4 = 0, Hold-off Status
  //D3 = 0, Fade-off Status
  //D2 = 0, Ramp up Enable
  //D1 = 0, CS Run
  //D0 = 1, Run Mode
  const uint8_t CMD_SHUTDOWN_MODE     = 0b00000000;  //D1=0, D0=0
  const uint8_t CMD_RUN               = 0b00100001;  //D7=0, D5=1, D0=1
  const uint8_t CMD_FULL_CURRENT_2mA  = 0b00000000;  //D2=0, D1=0, D0=0
  const uint8_t CMD_FULL_CURRENT_10mA = 0b00000011;  //D2=0, D1=1, D0=1
  const uint8_t CMD_FULL_CURRENT_20mA = 0b00000111;  //D2=1, D1=1, D0=1
  const uint16_t CMD_NO_OP            = 0x2000;  
  
  const uint8_t HI_Z = 0xFF;
  const uint8_t OFF  = 0x01;
  const uint8_t ON   = 0x02;
  
  
  // **********************************************
  // Functions
  // **********************************************
    
  void turnOn_LED(int led_index, int value)
  {
    turnOffAll_LED();
    
    value = max(0x03, value);
    value = min(0xFE, value);
    
    switch(led_index)
    {
      case LEFT:
        ledDriverWrite(0x03, value, 0x0A, 0xFF);
        break;
        
      case RIGHT:
        ledDriverWrite(0x09, value, 0x0A, 0xFF);
        break;
        
      case CNTR:
        ledDriverWrite(0x06, value, 0x0A, 0xFF);
        break;
        
      case TOP_L:
        ledDriverWrite(0x00, value, 0x0A, 0xFF);
        break;
      
      case TOP_R:
        ledDriverWrite(0x0A, 0xFF, 0x08, value);       
        break;
    }         
  }
  
  void turnOffAll_LED()
  {
    ledDriverWrite(0x0A, OFF, 0x0A, OFF);   
  }
    
  void setupLEDDriver()
  {
    // Initalize MAX6966
    ledDriverWrite(0x10, CMD_SHUTDOWN_MODE, 0x10, CMD_SHUTDOWN_MODE);
    ledDriverWrite(0x0A, HI_Z, 0x0A, HI_Z);  // Set all ports OFF
    
    // Put Device in Run Mode
    ledDriverWrite(0x10, CMD_RUN, 0x10, CMD_RUN);
    
    // PORTS: P9-P0 current to full (25mA)
    ledDriverWrite(0x15, CMD_FULL_CURRENT_20mA, 0x15, CMD_FULL_CURRENT_20mA);
  }
    
  void ledDriverWrite(uint8_t address_0, uint8_t value_0, uint8_t address_1, uint8_t value_1)
  {
    digitalWrite(pinDigital_LEDDriverSS, LOW);
        
    // Command to 2nd MAX6966
    uint16_t rx_wd_1 = SPI.transfer16( ((uint16_t)address_1<<8) | value_1 );
    
    // Command to 1st MAX6966
    uint16_t rx_wd_0 = SPI.transfer16( ((uint16_t)address_0<<8) | value_0 );
    
    digitalWrite(pinDigital_LEDDriverSS, HIGH);
  }

  boolean ledDriverWriteAndRead(uint8_t address_0, uint8_t value_0, uint8_t address_1, uint8_t value_1)
  {
    ledDriverWrite(address_0, value_0, address_1, value_1);
    
    return ledDriverReadDeviceState();
  }
  
  boolean ledDriverReadDeviceState()
  {
    uint16_t tx_wd = (1<<15) | (0x10<<8);
    
    ledDriverWrite( ((uint8_t)(tx_wd>>8)), 0x00, ((uint8_t)(tx_wd>>8)), 0x00);
    
    delayMicroseconds(1);
    
    digitalWrite(pinDigital_LEDDriverSS, LOW);
    
    // Read 2nd MAX6966      
    uint16_t rx_wd_1 = SPI.transfer16( CMD_NO_OP ); //
    
    // Read 1st MAX6966      
    uint16_t rx_wd_0 = SPI.transfer16( CMD_NO_OP );
    
    digitalWrite(pinDigital_LEDDriverSS, HIGH);
    
    // Debug
//    Serial.print(" RX_WD #0:"); Serial.print(rx_wd_0, BIN); Serial.print(" | ");
//    Serial.print(" RX_WD #1:"); Serial.print(rx_wd_1, BIN); Serial.print(" | ");
    
    if ( (rx_wd_0 & (1<<0)) && (rx_wd_1 & (1<<0)) )
    {
//      Serial.println("LED Run Mode: On ");
      return true;
    }
    else
    {
//      Serial.println("LED Run Mode: Off ");
      setupLEDDriver();
      return false;
    }
  }
  
  void bootupLEDSequence()
  {
    int dt_ms = 50;
    int pwm__on_val  = ON;
    int pwm__off_val = OFF;
    
    for (int i = 0; i < 10; i++)
    {
      ledDriverWrite(i, pwm__on_val, 0x00, pwm__off_val);  
      delay(dt_ms);
    }
    
    for (int i = 6; i < 9; i++)
    {
      ledDriverWrite(0x09, pwm__on_val, i, pwm__on_val);  
      delay(dt_ms);
    }
    
    for (int i = 0; i < 10; i++)
    {
      ledDriverWrite(i, pwm__off_val, 0x00, pwm__off_val); 
      delay(dt_ms);
    }
  
    for (int i = 6; i < 9; i++)
    {
      ledDriverWrite(0x09, pwm__off_val, i, pwm__off_val);  
      delay(dt_ms);
    }
    
    turnOffAll_LED();
  }  
  
  void bootupLEDSequence_discrete()
  {
    int dt_ms = 200;
    turnOn_LED(TOP_L, 255);  delay(dt_ms);
    turnOn_LED(LEFT,  255);  delay(dt_ms);
    turnOn_LED(CNTR,  255);  delay(dt_ms);
    turnOn_LED(RIGHT, 255);  delay(dt_ms);
    turnOn_LED(TOP_R, 255);  delay(dt_ms);
    turnOn_LED(RIGHT, 255);  delay(dt_ms);
    turnOn_LED(CNTR,  255);  delay(dt_ms);
    turnOn_LED(LEFT,  255);  delay(dt_ms);
    turnOn_LED(TOP_L, 255);  delay(dt_ms);
    
    turnOffAll_LED();
  }
    
  void bootupLEDSequence_blink()
  {
    ledDriverWrite(0x0A, 0xFF, 0x0A, 0xFF);  // Set all ports OFF
    delay(1000);
    
    ledDriverWrite(0x0A, 0x00, 0x0A, 0x00);  // Set all ports ON
    delay(1000);

    ledDriverWrite(0x0A, 0xFF, 0x0A, 0xFF);  // Set all ports OFF
    delay(1000);
    
    ledDriverWrite(0x0A, 0x00, 0x0A, 0x00);  // Set all ports ON
    delay(1000);
  }
  
  void bootupLEDSequence_heartbeat()
  {    
    int LEDlevel_0 = 0;
    int LEDlevel_1 = 0;
    boolean up = true;
  
    long start_time = millis();
    while(millis() - start_time < 2000)
    {
      ledDriverWrite(0x0A, LEDlevel_0, 0x0A, LEDlevel_1);  // Set all ports ON
      delay(25);
      
      int dt = 10;
      if (LEDlevel_0 > 255-2*dt)
        up = false;
      else if (LEDlevel_0 < 2*dt)
        up = true;
      
      if (up) 
        LEDlevel_0 += dt;
      else
        LEDlevel_0 -= dt;
        
      LEDlevel_1 = 255-LEDlevel_0;    
    }
  }
