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

// MAC del nodo central
uint8_t direccionNodoCentral[] = {0x78, 0x21, 0x84, 0x98, 0x1b, 0x84};

// MAC del otro receptor
uint8_t direccionOtroReceptor[] = {0x78, 0xe3, 0x6d, 0x11, 0xbb, 0x38};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

typedef struct mensaje {
  float temp;
  int id;
} mensaje;

mensaje datos;

esp_now_peer_info_t peerCentral;
esp_now_peer_info_t peerOtroReceptor;

int ultimoIDMostrado = -1;

void datosenviados(const uint8_t *direccionMAC, esp_now_send_status_t estado) {
  Serial.print("\r\nEstado de envío del último paquete:\t");
  Serial.println(estado == ESP_NOW_SEND_SUCCESS ? "Entrega exitosa" : "Entrega fallida");
}

void mostrarTemperatura(float temp) {
  display.clearDisplay();
  display.setCursor(0, 0);

  if (temp < 30) {
    display.println("Temperatura:");
    display.println(temp);
  } 
  else if (temp < 35) {
    display.println("Alerta, subida");
    display.println("de temperatura.");
    display.println("Temperatura:");
    display.println(temp);
  } 
  else {
    display.println("Peligro,");
    display.println("Alerta Roja.");
    display.println("Temperatura:");
    display.println(temp);
  }

  display.display();
}

void datosrecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  memcpy(&datos, incomingData, sizeof(datos));

  bool vieneDelCentral = memcmp(info->src_addr, direccionNodoCentral, 6) == 0;
  bool vieneDelOtroReceptor = memcmp(info->src_addr, direccionOtroReceptor, 6) == 0;

  // Solo mostramos si este mensaje no se había mostrado antes
  if (datos.id != ultimoIDMostrado) {
    mostrarTemperatura(datos.temp);
    ultimoIDMostrado = datos.id;
  }

  // Si el mensaje viene del nodo central, lo reenviamos al otro receptor
  if (vieneDelCentral) {
    esp_now_send(direccionOtroReceptor, (uint8_t*) &datos, sizeof(datos));
    display.println("Mensaje del nodo central recibido");
    display.display();
  }

  // Tanto si viene del central como del otro receptor, se devuelve confirmación al nodo central
  if (vieneDelCentral || vieneDelOtroReceptor) {
    esp_now_send(direccionNodoCentral, (uint8_t*) &datos, sizeof(datos));
  }

  if(vieneDelOtroReceptor){
    display.println("Mensaje del nodo repetidor recibido");
    display.display();
  }
}

void setup() {
  Serial.begin(115200);

  Wire.begin(4, 15);
  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);
  delay(50);
  digitalWrite(16, HIGH);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 falló");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    display.println("Error initializing ESP-NOW");
    display.display();
    return;
  }

  esp_now_register_recv_cb(datosrecv);
  esp_now_register_send_cb(esp_now_send_cb_t(datosenviados));

  // Añadimos nodo central
  memset(&peerCentral, 0, sizeof(peerCentral));
  memcpy(peerCentral.peer_addr, direccionNodoCentral, 6);
  peerCentral.channel = 0;
  peerCentral.encrypt = false;

  if (esp_now_add_peer(&peerCentral) != ESP_OK) {
    Serial.println("Fallo al añadir nodo central");
    return;
  }

  // Añadimos otro receptor
  memset(&peerOtroReceptor, 0, sizeof(peerOtroReceptor));
  memcpy(peerOtroReceptor.peer_addr, direccionOtroReceptor, 6);
  peerOtroReceptor.channel = 0;
  peerOtroReceptor.encrypt = false;

  if (esp_now_add_peer(&peerOtroReceptor) != ESP_OK) {
    Serial.println("Fallo al añadir otro receptor");
    return;
  }
}

void loop() {

}