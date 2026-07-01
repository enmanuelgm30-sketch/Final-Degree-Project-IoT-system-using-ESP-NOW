/*
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/get-change-esp32-esp8266-mac-address-arduino/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  Autor de las modificaciones: Enmanuel García Martínez
  Se ha usado como base el codigo de Rui Santos y Sara Santos.
*/

#include <math.h>
#include <esp_now.h>
#include <WiFi.h>

#define LED_VERDE_PIN 18
#define LED_AMARILLO_PIN 19
#define LED_ROJO_PIN 23
#define LED_OK 32
#define LED_NO_OK 33

uint8_t direcciontransm1[] = {0x78, 0xe3, 0x6d, 0x11, 0xb7, 0xd4};
uint8_t direcciontransm2[] = {0x78, 0xe3, 0x6d, 0x11, 0xbb, 0x38};

typedef struct mensaje{
  float temp;
}mensaje;

mensaje datos;

esp_now_peer_info_t peerInfo1;
esp_now_peer_info_t peerInfo2;

volatile bool esperando_respuesta = false;
unsigned long tiempoEnvio = 0;
const unsigned long tiempoEspera = 1000; // ms

void datosenviados(const uint8_t *direccionMAC,esp_now_send_status_t estado){
  Serial.print("\r\nEstado de envío del último paquete:\t");
  Serial.println(estado==ESP_NOW_SEND_SUCCESS ? "Entrega exitosa" : "Entrega fallida");
}

void datosrecibidos(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
if (esperando_respuesta) {

  digitalWrite(LED_OK, HIGH);
  digitalWrite(LED_NO_OK, LOW);
  esperando_respuesta = false;
}
}


const int sensorPin = 34; 
const float ResistenciaNominal = 1615.0; // Lo que lee ADC a temp. ambiente
const float TempNominal = 26;          // La temperatura de la habitación
const float Beta = 3950.0;               // Coeficiente estándar
const float ResistenciaFija = 10000.0;   // Resistencia 103 de la placa


void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  pinMode(LED_VERDE_PIN, OUTPUT);
  pinMode(LED_AMARILLO_PIN, OUTPUT);
  pinMode(LED_ROJO_PIN, OUTPUT);
  pinMode(LED_OK, OUTPUT);
  pinMode(LED_NO_OK, OUTPUT);  
  WiFi.mode(WIFI_STA);

  //iniciamos ESP NOW
  if(esp_now_init() != ESP_OK){
    Serial.println("ERROR: No se pudo inicializar ESP-NOW");
    return;
  }

  esp_now_register_send_cb(esp_now_send_cb_t(datosenviados));
  esp_now_register_recv_cb(datosrecibidos);

  //registramos al peer
  memcpy(peerInfo1.peer_addr, direcciontransm1, 6);
  peerInfo1.channel = 0;  
  peerInfo1.encrypt = false;

  //añadimos al peer
  if(esp_now_add_peer(&peerInfo1) != ESP_OK){
    Serial.println("Fallo al añadir peer");
    return;
  }

  //registramos al peer
  memcpy(peerInfo2.peer_addr, direcciontransm2, 6);
  peerInfo2.channel = 0;  
  peerInfo2.encrypt = false;

  //añadimos al peer
  if(esp_now_add_peer(&peerInfo2) != ESP_OK){
    Serial.println("Fallo al añadir peer");
    return;
  }

}

void loop() {

  long sumaADC = 0;
  for(int i=0; i<15; i++) { // Aumentamos a 15 muestras para suavizar
    sumaADC += analogRead(sensorPin);
    delay(10);
  }
  float adcPromedio = sumaADC / 15.0;

  //Formula Divisor de tensión
  float rTermistor = ResistenciaFija * (adcPromedio / (4095.0 - adcPromedio));

  // Steinhart-Hart
  float tempK = 1.0 / ( (1.0 / (TempNominal + 273.15)) + (1.0 / Beta) * log(rTermistor / ResistenciaNominal) );
  float tempC = tempK - 273.15 - 6.6;

  datos.temp = tempC;

  esperando_respuesta = true;
  tiempoEnvio = millis();

  digitalWrite(LED_OK, LOW);
  digitalWrite(LED_NO_OK, LOW);

    // Enviamos el mensaje por ESP-NOW
    esp_err_t result1 = esp_now_send(direcciontransm1, (uint8_t*) &datos, sizeof(datos));
    esp_err_t result2 = esp_now_send(direcciontransm2, (uint8_t*) &datos, sizeof(datos));


    if(result1 == ESP_OK){

      Serial.print("Enviado con exito: ");
      digitalWrite(LED_NO_OK, LOW);

    }else{

      Serial.println("ERROR: No se pudo enviar la informacion");
      digitalWrite(LED_NO_OK, HIGH);
      esperando_respuesta = false; 
    }

    if(result2 == ESP_OK){
      
      Serial.print("Enviado con exito: ");
      digitalWrite(LED_NO_OK, LOW);

    }else{

      Serial.println("ERROR: No se pudo enviar la informacion");
      digitalWrite(LED_NO_OK, HIGH);
      esperando_respuesta = false; 

    }

    while (esperando_respuesta && millis() - tiempoEnvio < tiempoEspera) {
      delay(10);
    }

    // Si sigue esperando, ninguno respondió
    if (esperando_respuesta) {
      digitalWrite(LED_OK, LOW);
      digitalWrite(LED_NO_OK, HIGH);
      esperando_respuesta = false;
    }
    
  
    if(tempC < 30){
      digitalWrite(LED_VERDE_PIN, HIGH);
      digitalWrite(LED_AMARILLO_PIN, LOW);
      digitalWrite(LED_ROJO_PIN, LOW);
    }
    else if(tempC < 35){
      digitalWrite(LED_VERDE_PIN, LOW);
      digitalWrite(LED_AMARILLO_PIN, HIGH);
      digitalWrite(LED_ROJO_PIN, LOW);
    }
    else{
      digitalWrite(LED_VERDE_PIN, LOW);
      digitalWrite(LED_AMARILLO_PIN, LOW);

      // Parpadeo rojo
      digitalWrite(LED_ROJO_PIN, HIGH);
      delay(300);
      digitalWrite(LED_ROJO_PIN, LOW);
      delay(100);
    }
  


  Serial.print("ADC: "); Serial.print(adcPromedio);
  Serial.print(" | R_NTC: "); Serial.print(rTermistor);
  Serial.print(" | Temp: "); Serial.print(tempC);
  Serial.println(" C");
  
  delay(1000);
}