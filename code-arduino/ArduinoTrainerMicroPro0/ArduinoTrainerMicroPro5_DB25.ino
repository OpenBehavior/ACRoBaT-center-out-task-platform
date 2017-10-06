
  // **********************************************
  // SPI - DB25 Driver (MAX6966) Constants
  // **********************************************
  
  //    Bits:   [A0-A7 B0-B6]   B7      C0-C5      C6      C7
  //    Packet: 15bit value     parity  6bit addr  readEn  stimEn
  
  //          Byte C      GND     Byte A         Byte B
  //     ===================================================
  //     \\  1  2  3  4  \ 5 /  6  7  8  9 /  10 11 12 13 //
  //      \\  14 15 16 17 \ / 18 19 20 21 / 22 23 24 25 //
  //        ============================================
  
  //     Byte:   C0 C1 C2 C3 C4 C5 C6 C7   
  //     Pin #   1  14 2  15 3  16 4  17
  
  //     Byte:   B0 B1 B2 B3 B4 B5 B6 B7   
  //     Pin #   22 10 23 11 24 12 25 13
  
  //     Byte:   A0 A1 A2 A3 A4 A5 A6 A7   
  //     Pin #   18 6  19 7  20 8  21 9
  
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
//  const uint8_t CMD_SHUTDOWN_MODE     = 0b00000000;  //D1=0, D0=0
//  const uint8_t CMD_RUN               = 0b00100001;  //D7=0, D5=1, D0=1
//  const uint8_t CMD_FULL_CURRENT_2mA  = 0b00000000;  //D2=0, D1=0, D0=0
//  const uint8_t CMD_FULL_CURRENT_10mA = 0b00000011;  //D2=0, D1=1, D0=1
//  const uint8_t CMD_FULL_CURRENT_20mA = 0b00000111;  //D2=1, D1=1, D0=1
//  const uint16_t CMD_NO_OP            = 0x2000;
 const uint8_t CMD_NO_OP_8x           = 0x20;
 const uint8_t CMD_CONFIG             = 0x10;
 
//  const uint8_t HI_Z       = 0xFF;  
  const uint8_t LOGIC_HIGH = 0x01;
  const uint8_t LOGIC_LOW  = 0x00;
  
  // Stimulation Parameters
  const uint8_t idxAddr_ParamReadEnable = 6; // Byte C.6
  const uint8_t idxAddr_StimEnable      = 7; // Byte C.7
  const uint8_t idxAddr_Parity          = 7; // Byte B.7
  
  
  // **********************************************
  // Functions
  // **********************************************
      
  void setupDB25Driver()
  {
    // Initalize MAX6966
    db25Write(0x10, CMD_SHUTDOWN_MODE, 0x10, CMD_SHUTDOWN_MODE, 0x10, CMD_SHUTDOWN_MODE);
    db25Write(0x0A, LOGIC_LOW, 0x0A, LOGIC_LOW, 0x0A, LOGIC_LOW);  // Set all ports OFF
    
    // Put Device in Run Mode
    db25Write(0x10, CMD_RUN, 0x10, CMD_RUN, 0x10, CMD_RUN);
    
    // PORTS: P9-P0 current to full (25mA)
    db25Write(0x15, CMD_FULL_CURRENT_20mA, 0x15, CMD_FULL_CURRENT_20mA, 0x15, CMD_FULL_CURRENT_20mA);
  }
  
  boolean db25_ReadDeviceState()
  {
    uint16_t tx_wd = (1<<15) | (0x10<<8);
    
    db25Write( ((uint8_t)(tx_wd>>8)), 0x00, ((uint8_t)(tx_wd>>8)), 0x00, ((uint8_t)(tx_wd>>8)), 0x00);
    
    delayMicroseconds(1);
    
    digitalWrite(pinDigital_DB25DriverSS, LOW);
    
    // Read 3rd MAX6966      
    uint16_t rx_wd_2 = SPI.transfer16( CMD_NO_OP );
    
    // Read 2nd MAX6966      
    uint16_t rx_wd_1 = SPI.transfer16( CMD_NO_OP );
    
    // Read 1st MAX6966      
    uint16_t rx_wd_0 = SPI.transfer16( CMD_NO_OP );
    
    digitalWrite(pinDigital_DB25DriverSS, HIGH);
    
    // Debug
//    Serial.print(" RX_WD #0:"); Serial.print(rx_wd_0, BIN); Serial.print(" | ");
//    Serial.print(" RX_WD #1:"); Serial.print(rx_wd_1, BIN); Serial.print(" | ");
//    Serial.print(" RX_WD #2:"); Serial.print(rx_wd_2, BIN); Serial.print(" | ");
    
    if ( (rx_wd_0 & (1<<0)) && (rx_wd_1 & (1<<0)) && (rx_wd_2 & (1<<0)) )
    {
//      Serial.println("DB25 Run Mode: On ");
      return true;
    }
    else
    {
//      Serial.println("DB25 Run Mode: Off ");
      setupDB25Driver();
      return false;
    }
  }
  
  void db25Write(uint8_t addr_ByteA, uint8_t value_ByteA, uint8_t addr_ByteB, uint8_t value_ByteB, uint8_t addr_ByteC, uint8_t value_ByteC)
  {    
    digitalWrite(pinDigital_DB25DriverSS, LOW);
        
    // Command to 3nd MAX6966 (BYTE B)
    uint16_t rx_wd_1 = SPI.transfer16( ((uint16_t)addr_ByteB<<8) | value_ByteB );
    
    // Command to 2st MAX6966 (BYTE A)
    uint16_t rx_wd_0 = SPI.transfer16( ((uint16_t)addr_ByteA<<8) | value_ByteA );
    
    // Command to 1st MAX6966 (BYTE C)
    uint16_t rx_wd_2 = SPI.transfer16( ((uint16_t)addr_ByteC<<8) | value_ByteC );
            
    digitalWrite(pinDigital_DB25DriverSS, HIGH);
  }
  
  void db25_Send2TDT(uint8_t byte_addr, uint16_t value)
  {    
    // Clear All bits (except for StimEn, ReadEn)
    db25ClearValueBits();
    
    uint8_t value_msb = (uint8_t)(value >> 8);
    uint8_t value_lsb = (uint8_t)value;
    
    uint8_t value_ByteA;  // byte A
    uint8_t value_ByteB;  // byte B
    uint8_t value_ByteC;  // byte C
    
    // Write 16bit word to value registers and 6bit address
    // Ignore last two bits of address, RESERVED:StimEn and ReadEn
    for (uint8_t idx = 0; idx < 8; idx++)
    {
      value_ByteA = (value_lsb & (1 << idx)) ? LOGIC_HIGH : LOGIC_LOW;
      value_ByteB = (value_msb & (1 << idx)) ? LOGIC_HIGH : LOGIC_LOW;
      value_ByteC = (byte_addr & (1 << idx)) ? LOGIC_HIGH : LOGIC_LOW;        
      
      db25Write(idx, value_ByteA, idx, value_ByteB, idx, value_ByteC);
      if (idx < 6)
        db25Write(idx, value_ByteA, idx, value_ByteB, idx, value_ByteC); 
      else
        db25Write(idx, value_ByteA, idx, value_ByteB, CMD_NO_OP_8x, 0x00);
    }

    // Set Parity Bit
    db25SetParityBit(value_msb, value_lsb, byte_addr);

    // Toggle Read Enable for dt microseconds
    int dt = 200;
    db25ReadEnable();
    delayMicroseconds(dt);
    db25ReadDisable();    
  }
  
  void db25TurnOffAll()
  {
    db25Write(0x0A, LOGIC_LOW, 0x0A, LOGIC_LOW, 0x0A, LOGIC_LOW);
  }
  
  void db25ClearValueBits()
  {
    for (uint8_t idx = 0; idx<6; idx++)
      db25Write(0x0A, LOGIC_LOW, 0x0A, LOGIC_LOW, idx, LOGIC_LOW);
  }
  
  void db25SetParityBit(uint8_t value_msb, uint8_t value_lsb, uint8_t byte_addr)
  {
    boolean parity = parity_even_bit(value_msb) ^ parity_even_bit(value_lsb) ^ parity_even_bit(byte_addr);
    uint8_t value = parity ? LOGIC_HIGH : LOGIC_LOW;
    
    db25Write(CMD_NO_OP_8x, 0x00, idxAddr_Parity, value, CMD_NO_OP_8x, 0x00);
  }
  
  void db25StimEnable()
  {
    db25Write(CMD_NO_OP_8x, 0x00, CMD_NO_OP_8x, 0x00, idxAddr_StimEnable, LOGIC_HIGH);
  }
  
  void db25StimDisable()
  {
    db25Write(CMD_NO_OP_8x, 0x00, CMD_NO_OP_8x, 0x00, idxAddr_StimEnable, LOGIC_LOW);
  }
  
  void db25ReadEnable()
  {
    db25Write(CMD_NO_OP_8x, 0x00, CMD_NO_OP_8x, 0x00, idxAddr_ParamReadEnable, LOGIC_HIGH);
  }
  
  void db25ReadDisable()
  {
    db25Write(CMD_NO_OP_8x, 0x00, CMD_NO_OP_8x, 0x00, idxAddr_ParamReadEnable, LOGIC_LOW);
  }   
  
  void db25BootupSequence()
  {
    //db25TurnOnAll();
    db25TurnOffAll();
  }
     
  void db25TurnOnAll()
  {    
    int dt = 500;
    
    for (uint8_t idx = 0; idx<8; idx++)
    {
      db25Write(idx, LOGIC_HIGH, CMD_NO_OP_8x, 0x00, CMD_NO_OP_8x, 0x00);  
      delay(dt);
    } 
     
    for (uint8_t idx = 0; idx<8; idx++)
    {
      db25Write(CMD_NO_OP_8x, 0x00, idx, LOGIC_HIGH, CMD_NO_OP_8x, 0x00);
      delay(dt);
    } 
     
    for (uint8_t idx = 0; idx<8; idx++)
    {
      db25Write(CMD_NO_OP_8x, 0x00, CMD_NO_OP_8x, 0x00, idx, LOGIC_HIGH);
      delay(dt);
    }     
  }
