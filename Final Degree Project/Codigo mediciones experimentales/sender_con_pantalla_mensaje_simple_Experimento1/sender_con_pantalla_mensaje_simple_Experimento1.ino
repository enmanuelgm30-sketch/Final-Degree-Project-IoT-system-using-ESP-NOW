/*
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/get-change-esp32-esp8266-mac-address-arduino/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  Autor de las modificaciones: Enmanuel García Martínez
  Se ha usado como base el codigo de Rui Santos y Sara Santos.
*/

#include <esp_now.h>
#include <esp_timer.h>
#include <WiFi.h>

// MAC del Receptor
uint8_t direcciontransm[] = {0x78, 0xe3, 0x6d, 0x11, 0xb7, 0xd4};

typedef struct mensaje {
  char m[32];
} mensaje;

mensaje datos;
esp_now_peer_info_t peerinfo;

// Variables para medir RTT
volatile uint64_t tiempo_envio = 0;
volatile bool esperando_respuesta = false;
//Variables para medir perdida de paquetes
int contador = 0;
int enviados = 0;
int recibidos = 0;

// Callback de envio para el Sender
void datosenviados(const wifi_tx_info_t *tx_info, esp_now_send_status_t estado) {
  Serial.print("\r\nEstado de envío: ");
  Serial.println(estado == ESP_NOW_SEND_SUCCESS ? "Entrega exitosa" : "Entrega fallida");
}
// Callback de recepción 
void datosrecibidos(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  uint64_t tiempo_recepcion = esp_timer_get_time();

  if (esperando_respuesta) {
    recibidos++;
    uint64_t RTT = tiempo_recepcion - tiempo_envio;
    uint64_t Latencia = RTT/2;
    float tasa_exito = ((float)recibidos / enviados) * 100;
    
    Serial.println("\n----- RESPUESTA RECIBIDA -----");
    Serial.print("RTT: ");
    Serial.print(RTT);
    Serial.println(" microsegundos");

    Serial.print("RTT: ");
    Serial.print((double)RTT / 1000.0, 3);
    Serial.println(" ms");
    Serial.print("Latencia: ");
    Serial.print((double)Latencia / 1000, 3);
    Serial.println(" ms");
    

    Serial.print("Enviados: ");
    Serial.println(enviados);

    Serial.print("Recibidos: ");
    Serial.println(recibidos);

    Serial.print("Tasa de éxito: ");
    Serial.print(tasa_exito);
    Serial.println("%");
    Serial.println("------------------------------");

    esperando_respuesta = false;
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);


  if (esp_now_init() != ESP_OK) {
    Serial.println("ERROR: No se pudo inicializar ESP-NOW");
    return;
  }

  // Registrar callbacks de forma limpia
  esp_now_register_send_cb(datosenviados);
  esp_now_register_recv_cb(datosrecibidos);

  // Registrar al Peer (Receptor)
  memset(&peerinfo, 0, sizeof(peerinfo));
  memcpy(peerinfo.peer_addr, direcciontransm, 6);
  peerinfo.channel = 0;  
  peerinfo.encrypt = false;

  if (esp_now_add_peer(&peerinfo) != ESP_OK) {
    Serial.println("Fallo al añadir peer");
    return;
  }

  Serial.println("Sender listo y esperando..."); 
}

void loop() {
  sprintf(datos.m, "Mensaje %d", contador);
  // Guardar instante de envío ANTES de mandar
  tiempo_envio = esp_timer_get_time();
  esperando_respuesta = true;

  esp_err_t result = esp_now_send(direcciontransm, (uint8_t*) &datos, sizeof(datos));

  if (result == ESP_OK) {
    Serial.print("Enviado con éxito: ");
    Serial.println(datos.m);
    enviados++;
  } else {
    Serial.println("ERROR: No se pudo enviar la información");
    esperando_respuesta = false; // Cancelamos la espera si falló el hardware
  }

  contador++;
  delay(2000);  // Espera de 2 segundos
}