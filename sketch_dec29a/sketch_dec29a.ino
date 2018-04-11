  #include <bluefruit.h>

  BLEUart bleuart;
  BLEDis bledis;    // DIS (Device Information Service) helper class instance

  
  // Function prototypes for packetparser.cpp
  uint8_t readPacket (BLEUart *ble_uart, uint16_t timeout);
  float   parsefloat (uint8_t *buffer);
  void    printHex   (const uint8_t * data, const uint32_t numBytes);
  
  // Packet buffer
  extern uint8_t packetbuffer[];
  
  int brightness = 0;    // how bright the LED is
  int fadeAmount = 10;    // how many points to fade the LED by
  int crankTimer = -1; // Time spent cranking engine.
  int startTimer = -1; // Time spent between start and crank.
  int runTimer = -1; // Time to run before auto shutoff.
  
  void setup(void) {
  Serial.begin(57600);
  Serial.println(F("-------------------------------------------"));

  Bluefruit.begin();
  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(4);
  Bluefruit.setName("Bluefruit_AlaskaLinuxUser");
  
  // Configure and start the BLE Uart service
  bleuart.begin();

  // Configure and Start the Device Information Service
  Serial.println("Configuring the Device Information Service");
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();

  // Set up and start advertising
  startAdv();

  pinMode(17, OUTPUT); //19 is blue LED, 17 is red LED.
  pinMode(19, OUTPUT); //19 is blue LED, 17 is red LED.
  
  pinMode(16, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(11, OUTPUT);
  
  digitalWrite(16, LOW);
  digitalWrite(15, LOW);
  digitalWrite(7, LOW);
  digitalWrite(11, LOW);
  analogWrite(19, 0);


}
 
void startAdv(void) {
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  
  // Include the BLE UART (AKA 'NUS') 128-bit UUID
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();

  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void connect_callback(uint16_t conn_handle) {
  (void) conn_handle;
  Serial.println("Connected");
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.println("Disconnected");
}

void loop(void) {
  
  analogWrite(17, brightness);

  // change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (brightness <= 0 || brightness >= 50) {
    fadeAmount = -fadeAmount;
  }
  // wait for 500 milliseconds to see the dimming effect
  delay(500);
  // To print the brightness // Serial.println(brightness);
  
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 3.6V):
  float voltage = sensorValue * (3.6 / 1023.0);
  // print out the value you read:
  if (voltage >= 0.8) {
     Serial.print("Pin A01: ");Serial.println(voltage);
     // Brakes have been pushed, shut down all autostart circuits.
     // This will not affect actual engine run circuits, just the autostart
     // ones. So the engine will stay running if the key is in and on.
     digitalWrite(16, LOW);
     digitalWrite(15, LOW);
     digitalWrite(7, HIGH);
     digitalWrite(11, HIGH);
     crankTimer = -1;
     startTimer = -1;
     runTimer = -1;
     Serial.println(" Kill All - Brakes pressed. ");
  }

  if (runTimer == 0) {
        // Turn everything off from the runTimer.
        digitalWrite(16, LOW);
        digitalWrite(15, LOW);
        digitalWrite(7, HIGH);
        digitalWrite(11, HIGH);
        crankTimer = -1;
        startTimer = -1;
        runTimer = -1;
        Serial.println(" Kill All - runTimer elapsed. ");
  } else if (runTimer > 0) {
    runTimer = runTimer -1;
    Serial.print (" runTimer "); Serial.println(runTimer);
  }

  if (startTimer == 0) {
    digitalWrite(15, HIGH);
    crankTimer = 3;
    startTimer = -1;
    Serial.println (" crankTimer ON ");
  } else if (startTimer > 0) {
    startTimer = startTimer -1;
    Serial.print (" startTimer "); Serial.println(startTimer);
  }

  if (crankTimer == 0) {
    digitalWrite(15, LOW);
    crankTimer = -1;
    startTimer = -1;
    runTimer = 600; // 600 seconds = 10 minutes of run time before shutdown.
    Serial.println (" crankTimer OFF, runTimer started. ");
  } else if (crankTimer > 0) {
    crankTimer = crankTimer -1;
    Serial.print (" crankTimer "); Serial.println(crankTimer);
  }
    

  // Wait for new data to arrive
  uint8_t len = readPacket(&bleuart, 500);
  if (len == 0) return;

  // Buttons
  if (packetbuffer[1] == 'B') {
    uint8_t buttnum = packetbuffer[2] - '0';
    boolean pressed = packetbuffer[3] - '0';
    Serial.print ("Button "); Serial.print(buttnum);
    if (pressed) {
      Serial.println(" pressed");
      //bluart.print("pressed");
      analogWrite(19, 50);
    } else {
      Serial.println(" released");
      analogWrite(19, 0);
    }
    if (buttnum == 1) { // Turn on vehicle.
      digitalWrite(16, HIGH);
      startTimer = 5;
    } else if (buttnum == 2) {
        // Turn everything off from the button.
        digitalWrite(16, LOW);
        digitalWrite(15, LOW);
        digitalWrite(7, HIGH);
        digitalWrite(11, HIGH);
        crankTimer = -1;
        startTimer = -1;
        runTimer = -1;
        Serial.println(" Kill All - Button pressed. ");
    }
  }
    
}
