/*
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/get-change-esp32-esp8266-mac-address-arduino/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  Autor de las modificaciones: Enmanuel García Martínez
  Se ha usado como base el codigo de Rui Santos y Sara Santos.
*/

#include <esp_now.h>//librerias
#include <WiFi.h>

uint8_t direcciontransm2[] = {0x78, 0x21, 0x84, 0x98, 0x1b, 0x84};


typedef struct mensaje{
  float temp;
}mensaje;


mensaje datos;

esp_now_peer_info_t peerInfo2;

void datosenviados(const uint8_t *direccionMAC,esp_now_send_status_t estado){
  Serial.print("\r\nEstado de envío del último paquete:\t");
  Serial.println(estado==ESP_NOW_SEND_SUCCESS ? "Entrega exitosa" : "Entrega fallida");
}



//funcion calback que se ejecutara cuando se reciban los datos
void datosrecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len){
  memcpy(&datos, incomingData, sizeof(datos));

   // Enviamos el mensaje por ESP-NOW
    esp_err_t result2 = esp_now_send(direcciontransm2, (uint8_t*) &datos, sizeof(datos));



    if(result2 == ESP_OK){
      Serial.print("Enviado con exito: ");
    }else{
      Serial.println("ERROR: No se pudo enviar la informacion");
    }

  if(datos.temp < 30){

  Serial.print("Temperatura: "); 
  Serial.println(datos.temp);

  }else if(datos.temp < 35){

  Serial.println("Alerta, Subida de temperatura."); 
  Serial.print("Temperatura: "); 
  Serial.println(datos.temp);

  }else{

  Serial.println("Peligro, Alerta Roja."); 
  Serial.print("Temperatura: "); 
  Serial.println(datos.temp);
  }


}
void setup() {

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_recv_cb(esp_now_recv_cb_t(datosrecv));

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
