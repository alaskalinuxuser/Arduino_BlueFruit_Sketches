void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println(F("-------------------------------------------"));
}

void loop() {
  
  delay(500);
  
  // put your main code here, to run repeatedly:

  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 3.6V):
  float voltage = sensorValue * (3.6 / 1023.0);
  // print out the value you read:

     Serial.print("Pin A0: ");Serial.println(voltage);
  

  // read the input on analog pin 3:
  int voltValue = analogRead(A3);
  float voltVoltage = voltValue * (3.6 / 1023.0);

      // It has enough voltage, it is running.
      Serial.print("Pin A3: ");Serial.println(voltVoltage);

}
