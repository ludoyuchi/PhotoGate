#include <ArduinoBLE.h>
#include <SD.h>

const int photogatePin = 2;  // Pino digital conectado ao photogate
unsigned long startTime = 0;
unsigned long endTime = 0;
bool beamInterrupted = false;

BLEService photogateService("19b10000-e8f2-537e-4f6c-d104768a1214"); // UUID do serviço
BLEUnsignedLongCharacteristic startTimeCharacteristic("19b10001-e8f2-537e-4f6c-d104768a1214", BLERead | BLENotify); // UUID da característica de tempo de início
BLEUnsignedLongCharacteristic interruptionTimeCharacteristic("19b10002-e8f2-537e-4f6c-d104768a1214", BLERead | BLENotify); // UUID da característica de tempo de interrupção

File dataFile;

void setup() {
  pinMode(photogatePin, INPUT);
  Serial.begin(9600);  // Inicia a comunicação serial
  
  // Inicializa a conexão Bluetooth
  if (!BLE.begin()) {
    Serial.println("Falha ao inicializar o Bluetooth");
    while (1);
  }
  
  // Define o serviço e as características BLE
  BLE.setLocalName("Photogate");
  BLE.setAdvertisedService(photogateService);
  photogateService.addCharacteristic(startTimeCharacteristic);
  photogateService.addCharacteristic(interruptionTimeCharacteristic);
  BLE.addService(photogateService);
  
  // Inicia a transmissão Bluetooth
  startTimeCharacteristic.writeValue(startTime);
  interruptionTimeCharacteristic.writeValue(0);
  BLE.advertise();
  
  Serial.println("Aguardando conexão Bluetooth...");

  // Inicializa a comunicação com o cartão SD
  if (!SD.begin()) {
    Serial.println("Falha ao inicializar o cartão SD");
    while (1);
  }
  
  // Abre o arquivo para escrita
  dataFile = SD.open("dados.txt", FILE_WRITE);
  
  if (!dataFile) {
    Serial.println("Falha ao abrir o arquivo");
    while (1);
  }
}

void loop() {
  BLEDevice central = BLE.central();
  
  if (central) {
    Serial.print("Conectado a: ");
    Serial.println(central.address());
    
    while (central.connected()) {
      if (digitalRead(photogatePin) == HIGH && !beamInterrupted) {
        beamInterrupted = true;
        startTime = micros();
        Serial.print("Feixe interrompido. Tempo de início: ");
        Serial.println(startTime);
        startTimeCharacteristic.writeValue(startTime);
      }
  
      if (digitalRead(photogatePin) == LOW && beamInterrupted) {
        beamInterrupted = false;
        endTime = micros();
        unsigned long interruptionTime = endTime - startTime;
        Serial.print("Feixe restaurado. Tempo de interrupção: ");
        Serial.println(interruptionTime);
        interruptionTimeCharacteristic.writeValue(interruptionTime);
        
        // Grava os dados no cartão microSD
        dataFile.println("Feixe interrompido. Tempo de início: " + String(startTime));
        dataFile.println("Feixe restaurado. Tempo de interrupção: " + String(interruptionTime));
      }
    }
    
    Serial.print("Desconectado de: ");
    Serial.println(central.address());
  }
  
  // Fecha o arquivo para liberar recursos
  dataFile.close();
}
