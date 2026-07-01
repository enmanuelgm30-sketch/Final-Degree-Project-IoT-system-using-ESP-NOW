/*
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/get-change-esp32-esp8266-mac-address-arduino/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  Autor de las modificaciones: Enmanuel García Martínez
  Se ha usado como base el codigo de Rui Santos y Sara Santos.
*/

#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 16
#define SCREEN_ADDRESS 0x3C

uint8_t direcciontransm2[] = {0x78, 0x21, 0x84, 0x98, 0x1b, 0x84};


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

typedef struct mensaje{
  float temp;
  int id;
}mensaje;

mensaje datos;

esp_now_peer_info_t peerInfo1;
esp_now_peer_info_t peerInfo2;

void datosenviados(const uint8_t *direccionMAC,esp_now_send_status_t estado){
  Serial.print("\r\nEstado de envío del último paquete:\t");
  Serial.println(estado==ESP_NOW_SEND_SUCCESS ? "Entrega exitosa" : "Entrega fallida");
}


// Función callback que se ejecutará cuando se reciban los datos
void datosrecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  memcpy(&datos, incomingData, sizeof(datos));
  if(datos.temp < 30){

  display.clearDisplay(); 
  display.setCursor(0,0);
  display.println("Temperatura: "); 
  display.println(datos.temp);
  display.display(); 

  }else if(datos.temp < 35){

  display.clearDisplay(); 
  display.setCursor(0,0);
  display.println("Alerta, Subida de temperatura."); 
  display.println("Temperatura: "); 
  display.println(datos.temp);
  display.display(); 

  }else{

  display.clearDisplay(); 
  display.setCursor(0,0);
  display.println("Peligro, Alerta Roja."); 
  display.println("Temperatura: "); 
  display.println(datos.temp);
  display.display(); 

  }
  
  esp_now_send(direcciontransm2, (uint8_t*) &datos, sizeof(datos));
  }

void setup() {
  Serial.begin(115200);
  Wire.begin(4, 15);   // SDA, SCL
  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);
  delay(50);
  digitalWrite(16, HIGH);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 falló");
    while(true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  WiFi.mode(WIFI_STA);


  if (esp_now_init() != ESP_OK) {
    display.println("Error initializing ESP-NOW");
    display.display();
    return;
  }
  
  // Registrar el callback de recepción
  esp_now_register_recv_cb(datosrecv);

  esp_now_register_send_cb(esp_now_send_cb_t(datosenviados));

    //registramos al peer
  memset(&peerInfo2, 0, sizeof(peerInfo2));
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

}