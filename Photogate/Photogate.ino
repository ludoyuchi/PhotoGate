#include <ArduinoBLE.h>
#include <SD.h>

const int photogatePin = 2;
unsigned long startTime = 0;
unsigned long endTime = 0;
bool beamInterrupted = false;
int voltas = 0;

BLEService photogateService("19b10000-e8f2-537e-4f6c-d104768a1214");
BLEUnsignedLongCharacteristic startTimeCharacteristic("19b10001-e8f2-537e-4f6c-d104768a1214", BLERead | BLENotify);
BLEUnsignedLongCharacteristic interruptionTimeCharacteristic("19b10002-e8f2-537e-4f6c-d104768a1214", BLERead | BLENotify);

File dataFile;
String dataString = "";

void setup() {
  pinMode(photogatePin, INPUT);
  Serial.begin(9600);

  if (!BLE.begin()) {
    Serial.println("Failed to initialize Bluetooth");
    while (1);
  }

  BLE.setLocalName("Photogate");
  BLE.setAdvertisedService(photogateService);
  photogateService.addCharacteristic(startTimeCharacteristic);
  photogateService.addCharacteristic(interruptionTimeCharacteristic);
  BLE.addService(photogateService);

  startTimeCharacteristic.writeValue(startTime);
  interruptionTimeCharacteristic.writeValue(0);
  BLE.advertise();

  Serial.println("Waiting for Bluetooth connection...");

  if (!SD.begin()) {
    Serial.println("Failed to initialize SD card");
    while (1);
  }

  dataFile = SD.open("dados.txt", FILE_WRITE);

  if (!dataFile) {
    Serial.println("Failed to open file");
    while (1);
  }
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to: ");
    Serial.println(central.address());

    while (central.connected()) {
      if (digitalRead(photogatePin) == HIGH && !beamInterrupted) {
        beamInterrupted = true;
        startTime = micros();
        Serial.print("Beam interrupted. Start time: ");
        Serial.println(startTime);
        startTimeCharacteristic.writeValue(startTime);

        // Send start time via serial
        Serial.println(startTime);

        dataString = String(startTime) + ",";
      }

      if (digitalRead(photogatePin) == LOW && beamInterrupted) {
        beamInterrupted = false;
        endTime = micros();
        unsigned long interruptionTime = endTime - startTime;
        Serial.print("Beam restored. Interruption time: ");
        Serial.println(interruptionTime);
        interruptionTimeCharacteristic.writeValue(interruptionTime);

        // Send interruption time via serial
        Serial.println(interruptionTime);

        unsigned long tempoVolta = endTime - startTime;
        unsigned long tempoInterrupcao = interruptionTime;

        // Send current lap and times via serial
        Serial.print("Volta: ");
        Serial.println(voltas);
        Serial.print("Tempo de volta: ");
        Serial.println(tempoVolta);
        Serial.print("Tempo de interrupção: ");
        Serial.println(tempoInterrupcao);

        dataString += String(tempoVolta) + "," + String(tempoInterrupcao) + ",";
        dataFile.println(dataString);
        dataString = "";

        // Reset voltas and times for the next lap
        voltas++;
        startTime = 0;
        endTime = 0;
        tempoVolta = 0;
        tempoInterrupcao = 0;
      }
    }

    Serial.print("Disconnected from: ");
    Serial.println(central.address());
  }

  dataFile.close();
}
