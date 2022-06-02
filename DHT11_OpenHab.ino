/* 
 * Monitoreo de Temperatura y Humedad (por MQTT) en OpenHab
 * Por: Marving
 * Fecha: 31 de mayo de 2022
 * 
 * Se utiliza el sensor DHT11 para medir la
 * temperatura y la humedad, imprimirlas en el 
 * monitor serial, enviarlas por MQTT a OpenHAB e
 * imprimirlas en OpenHAB. Se agregan Leds 
 * indicadores:
 * 
 * Led1 (amarillo): indicador de alta temperatura             (On-HIGH, Off-LOW)
 * Led2 (azul):     indicador de alta humedad relativa        (On-HIGH, Off-LOW)
 * Led3 (rojo):     indicador parpad de espera de conex WiFi  (On-HIGH, Off-LOW)
 * Led4 (blanco):   indicador de recepción de mensaje MQTT    (On-LOW,  Off-HIGH lógica inversa)
 * +++++++++++++++++
 * Conexiones:
 * 
 * ESP32      LED1(ama)      ESP32      LED2(azu)      ESP32      LED3(roj)       ESP32      DHT11
 * I04 ------ ánodo          I02 ------ ánodo          I014 ----- ánodo           I012 ----- señal (izq)
 * GND ------ cátodo         GND ------ cátodo         GND ------ cátodo          GND ------ GND   (der)
 *                                                                                5V ------- Vcc   (cen)
 * 
 */

// Bibliotecas
#include <WiFi.h>           // para el control de WiFi
#include <PubSubClient.h>   // para conexión MQTT
#include "DHT.h"            // para el sensor DHT11

// Constantes para objetos
#define DHTPIN 12
#define DHTTYPE DHT11 

// Objetos
DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;             // Este objeto maneja los datos de conexion WiFi
PubSubClient client(espClient);   // Este objeto maneja los datos de conexion al broker

// Constantes 
const int LED1 = 4;                           // Indica alta temperatura
const int LED2 = 2;                           // Indica alta humedad
const int LED3 = 14;                          // Conexión WiFi
const int LED4 = 33;                          // Recepción de mensaje MQTT
const int TEMP_H = 23;                        // Umbral de alta temperatura
const int HUME_H = 73;                        // Umbral de alta humedad relativa
const char* ssid = "Totalplay-ADA6";          // Nombre de la red WiFi
const char* password = "Tp0107018052";        // Contraseña de la red Wifi
const char* mqtt_server = "192.168.100.60";   // Red local (ifconfig en terminal) -> broker MQTT
IPAddress server(192,168,100,60);             // Red local (ifconfig en terminal) -> broker MQTT

// Variables
float t;                  // Variable de temperatura
float h;                  // Variable de humedad relativa
long timeNow5, timeLast5; // Vars de control de tiempo no-bloq en envío de mens MQTT
int wait5 = 5000;         // Espera cada 5 s para envío de mensajes MQTT

// ----------------- Función SETUP (Se ejecuta sólo una vez al energizar) ----------------- //

void setup() {// Inicio de void setup()

  Serial.begin(115200);        // Inicio de comunicación serial

  // Configuración de pines - Leds

  pinMode (LED1,OUTPUT);
  pinMode (LED2,OUTPUT);
  pinMode (LED3,OUTPUT);
  pinMode (LED4,OUTPUT);

  digitalWrite (LED1,LOW);     // Apagar LED alta temp
  digitalWrite (LED2,LOW);     // Apagar LED alta humed
  digitalWrite (LED3,LOW);     // Apagar LED conexión WiFi
  digitalWrite (LED4,HIGH);    // Apagar LED mensaje MQTT (lógica inversa)

  // Conexión a la red WiFi

  Serial.println();
  Serial.print("Conectar a ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);             // Esta es la función que realiza la conexión a WiFi
 
  while (WiFi.status() != WL_CONNECTED) { // Este bucle espera a que se realice la conexión a WiFi
    digitalWrite (LED3,LOW);              // Apagar LED de conexión WiFi
    delay(500);                             // Esperar 5 ms a la conexión (espera bloqueante)
    digitalWrite (LED3,HIGH);             // Encender LED de conexión WiFi
    Serial.print(".");                    // Indicador de progreso
    delay(500);                           // Esperar 0.5 s a la conexión (espera bloqueante)
  }
  
  // Cuando se haya logrado la conexión:
  Serial.println();
  Serial.println("WiFi conectado");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());

  // Como se logró la conexión, encender LED de conexión
  if (WiFi.status () > 0){
    digitalWrite (LED3,HIGH);       // Encender LED de conexión WiFi
  }
  
  // Conexión al Broker

  delay(1000);                      // Espera 1 s antes de iniciar la comunicación con el broker
  client.setServer(server,1883);    // Conectarse a la IP del broker en el puerto indicado
//  client.setCallback(callback);   // Activ func CallBack, permite recibir mensjs MQTT y ejec funcs a partir de ellos
  delay(1500);                      // Esta espera es preventiva, espera a la conexión para no perder información


  Serial.println(F("¡ Prueba del sensor DHT11 !"));
  dht.begin();              // Iniciar comunicación con el sensor DHT11

  timeLast5 = millis();     // Inicia el control de tiempo - envío de mensajes MQTT y para lectura de datos DTH11

}// Fin de void setup

// ----------------------- Función LOOP (Se ejecuta constantemente) ----------------------- //

void loop() {
  
  if (!client.connected()) {   // Si NO hay conexión al broker ...
    reconnect();               // Ejecuta el intento de reconexión
  }                            

  client.loop();               // Es para mantener la comunicación con el broker

  lecturaDHT();
  
  logica();

} // Fin de void loop

// ------------------------ Función de lectura del DHT11 y publicación t-h ------------------------ //

void lecturaDHT() {
  
timeNow5 = millis(); // Control de tiempo para esperas no bloqueantes

  if (timeNow5 - timeLast5 > wait5) { 
  
    t = dht.readTemperature();        // Lee la temp en ºC 
    h = dht.readHumidity();           // Lee la humed en % 

    // Revisar si la lectura falló (para intentar de nuevo)
    if ( isnan(t) || isnan(h) ) {
      Serial.println (F("¡¡ Falla en la lectura del sensor DHT !!"));  // You can pass flash-memory based strings to Serial.print() by wrapping them with F(). (NO ENTIENDO la F)
      return;                                                          // NO ENTIENDO
    }

    char tempString[8];              // Arreglo de caracteres a enviar por MQTT (long del msj: 8 caracteres)
    char humeString[8];              // Arreglo de caracteres a enviar por MQTT (long del msj: 8 caracteres)
    dtostrf(t, 1, 2, tempString);    // Función nativa de leguaje AVR; convierte un arreglo de caracs en una var String
    dtostrf(h, 1, 2, humeString);    // Función nativa de leguaje AVR; convierte un arreglo de caracs en una var String
    Serial.print("Temperatura: ");
    Serial.print(tempString);
    Serial.print("    Humedad: ");
    Serial.println(humeString);
    client.publish("codigoIoT/G6/temp", tempString);   // Envía Temperatura por MQTT, especifica el tema y el valor
    client.publish("codigoIoT/G6/humed", humeString);  // Envía Humedad Rel por MQTT, especifica el tema y el valor

    timeLast5 = timeNow5;            // Actualización de seguimiento de tiempo

  }

}

// ----------------------------- Función de la lógica ----------------------------- //

void logica() {

  // LED indicador de alta temperatura ( t > 23ºC ) 

  if ( t > TEMP_H ) {
    digitalWrite (LED1, HIGH);
  } else {
    digitalWrite (LED1, LOW);
  }
  
  // LED indicador de alta humedad relativa ( h > 73% )
  
  if ( h > HUME_H ) {
    digitalWrite (LED2, HIGH);
  } else {
    digitalWrite (LED2, LOW);
  }
}


// -------------------------------- Función para Reconectarse -------------------------------- //

void reconnect() {
  while (!client.connected()) {                  // Mientras NO haya conexión ...
    Serial.print("Tratando de conectarse...");
    if (client.connect("ESP32CAMClient")) {      // Si se logró la conexión ...
      Serial.println("Conectado");
      client.subscribe("esp32/output");          // Esta función realiza la suscripción al tema (¿se necesita aquí? creo que no)
    }                                            
    else {                                       // Si no se logró la conexión ...
      Serial.print("Conexion fallida, Error rc=");
      Serial.print(client.state());              // Muestra el código de error
      Serial.println(" Volviendo a intentar en 5 segundos");
      delay(5000);                               // Espera de 5 segundos bloqueante
      Serial.println (client.connected ());      // Muestra estatus de conexión
    }                                            
  }                                              
}                                                


//// -------------------------------- Función Callback -------------------------------- //
//
//// Esta función permite tomar acciones en caso de 
//// que se reciba un mensaje correspondiente a un 
//// tema al cual se hará una suscripción.
//// Aún no está habilitada en OpenHAB
//
//void callback(char* topic, byte* message, unsigned int length) {
//
//  // Indicar por serial que llegó un mensaje
//  Serial.print("Llegó un mensaje en el tema: ");
//  Serial.print(topic);
//
//  // Concatenar los mensajes recibidos para conformarlos como una variable String
//  String messageTemp;                 // Se declara la variable en la cual se generará el mensaje completo  
//  for (int i = 0; i < length; i++) {  // Se imprime y concatena el mensaje
//    Serial.print((char)message[i]);
//    messageTemp += (char)message[i];
//  }
//
//  // Se comprueba que el mensaje se haya concatenado correctamente
//  Serial.println();
//  Serial.print ("Mensaje concatenado en una sola variable: ");
//  Serial.println (messageTemp);
//
//  // En caso de recibir el mensaje true - false, se cambiará el 
//  // estado del LED flash soldado en la placa.
//  // El ESP32CAM está suscrito al tema esp32/output
//  if (String(topic) == "esp32/output") {  
//    if(messageTemp == "true"){
//      Serial.println("Led4 encendido");
//      digitalWrite(LED4, LOW);
//    }
//    else if(messageTemp == "false"){
//      Serial.println("Led4 apagado");
//      digitalWrite(LED4, HIGH);
//    }
//  }
//}
