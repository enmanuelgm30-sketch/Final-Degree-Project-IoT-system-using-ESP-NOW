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

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

typedef struct mensaje {
  char m[32];
} mensaje;

mensaje datos;

// MAC del Sender 
uint8_t direccionSender[] = {0x78, 0xe3, 0x6d, 0x11, 0xbb, 0x38};
esp_now_peer_info_t peerinfo;

// Callback de envío 
void ackEnviadoCb(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  Serial.print("Estado envío ACK: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "ACK OK" : "ACK FALLÓ");
  
}

// Función callback que se ejecutará cuando se reciban los datos
void datosrecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  memcpy(&datos, incomingData, sizeof(datos));

  // Enviar respuesta (ACK) para medir rtt
  esp_err_t result = esp_now_send(direccionSender, (uint8_t*)&datos, sizeof(datos));
  if (result == ESP_OK) {
    Serial.println("Comando de ACK enviado al stack");
  } else {
    Serial.println("ERROR enviando ACK (¿Peer no registrado?)");
  }


  display.clearDisplay(); 
  display.setCursor(0,0);
  display.println("Bytes recibidos:"); 
  display.println(len);
  display.print(datos.m);
  display.display(); 


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
  //registrar callback de envío para ver si el ACK llego al hardware
  esp_now_register_send_cb(ackEnviadoCb);

  //Registrar al Sender como PEER
  memset(&peerinfo, 0, sizeof(peerinfo));
  memcpy(peerinfo.peer_addr, direccionSender, 6);
  peerinfo.channel = 0;  
  peerinfo.encrypt = false;

  if (esp_now_add_peer(&peerinfo) != ESP_OK) {
    Serial.println("Fallo al añadir al Sender como peer");
    display.println("Error Peer");
    display.display();
    return;
  }
  
  Serial.println("Receptor listo y Peer registrado.");
}

void loop() {

}