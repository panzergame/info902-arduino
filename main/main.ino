#include <SoftwareSerial.h>
constexpr int bluetoothRX = 2;
constexpr int bluetoothTX = 3;
SoftwareSerial bluetoothSerial(bluetoothRX, bluetoothTX); // RX | TX

bool sendBluetoothCommand(const char *command)
{
   bluetoothSerial.print(command);
   int tries = 0;
   const int maxTries = 1000;
   while (!bluetoothSerial.available() && tries < maxTries) {
      ++tries;
      delay(100);
   }

   if (tries == maxTries) {
      Serial.print("Timeout: ");
      Serial.println(command);
      return false;
   }
   else {
      int i = 0;
      Serial.print(command);
      Serial.print(" -> ");
      while (bluetoothSerial.available()) {
        const char recvChar = bluetoothSerial.read();
        Serial.print(recvChar);
      }
      Serial.println();
      return true;
   }
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Hello");
  
  pinMode(bluetoothRX, INPUT);
  pinMode(bluetoothTX, OUTPUT);
  bluetoothSerial.begin(9600);

  sendBluetoothCommand("AT+RENEW");
  sendBluetoothCommand("AT+BAUD2");
  //sendBluetoothCommand("AT+RESET");

  /*bluetoothSerial.begin(9600);
  delay(3500);*/

  sendBluetoothCommand("AT+NAMESplash");
  //sendBluetoothCommand("AT+NOTI1"); // Connection notification
  /*sendBluetoothCommand("AT+MODE1");
  sendBluetoothCommand("AT+DUAL1");
  sendBluetoothCommand("AT+VERR?");
  sendBluetoothCommand("AT+CLEAE");
  sendBluetoothCommand("AT+CLEAB");
  sendBluetoothCommand("AT+BONDE");
  sendBluetoothCommand("AT+BONDB");*/
  sendBluetoothCommand("AT+ROLB0");
  /*sendBluetoothCommand("AT+AUTH?");
  sendBluetoothCommand("AT+PINE000000");*/
  //sendBluetoothCommand("AT+PINB654321");
  sendBluetoothCommand("AT+PINE?");
  sendBluetoothCommand("AT+PINB?");
  
  sendBluetoothCommand("AT+SCAN0");
  //sendBluetoothCommand("AT+PIO01"); // led behaviour

  sendBluetoothCommand("AT+ADDE?");
  sendBluetoothCommand("AT+RESET");
  //sendBluetoothCommand("AT");
}

void loop()
{
  //char response[20];
  //sendBluetoothCommand("AT", response);
  delay(1000);
  bluetoothSerial.print("hi");
  //bluetoothSerial.println(millis());
  Serial.print("hi");
  Serial.println(millis());

  /*bluetoothSerial.print("AT+RENEW");
  bluetoothSerial.print("AT+BAUD2");
  bluetoothSerial.print("AT+RESET");*/
  //Serial.println(response);
  if (bluetoothSerial.available()) {
    const char recvChar = bluetoothSerial.read();
    Serial.print(recvChar);
  }
}
