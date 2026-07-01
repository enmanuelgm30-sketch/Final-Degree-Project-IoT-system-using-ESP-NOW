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



typedef struct mensaje{
  float temp;
}mensaje;


mensaje datos;

//funcion calback que se ejecutara cuando se reciban los datos
void datosrecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len){
  memcpy(&datos, incomingData, sizeof(datos));
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
  
}
 





void loop() {

}
