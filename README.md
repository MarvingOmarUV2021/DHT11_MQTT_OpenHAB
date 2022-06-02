# DHT11_MQTT_OpenHAB

## Programa realizado en la IDE de Arduino para lectura de temperatura y humedad con el DHT11, envío de datos por MQTT y publicación de datos en OpeHAB

El programa fue realizado en la *IDE* de *Arduino*.

El **objetivo principal del programa** se descibe en los siguientes 4 puntos:
 * Leer la temperatura ambiental (ºC) y la humedad relativa (%) con el sensor *DHT11*.
 * Los datos de temperatura y humedad se envían al *uC ESP32CAM*.
 * El *uC* está conectado a una red *WiFi*, y envía los datos de temperatura y humedad a un *broker* mediante protocolo MQTT.
 * El *broker* envía los datos de temperatura y humedad a *OpenHAB*.

El **objetivo secundario del programa** se descibe en los siguientes 4 puntos:
 * Si la temperatura rebasa un cierto umbral, se enciende un *LED* amarillo colocado junto al *uC*.
 * Si la humedad rebasa un cierto umbral, se enciende un *LED* azul colocado junto al *uC*.
 * Existe un *LED* rojo junto al *uC* que se enciende cuando éste tiene conexión con la red *WiFi*.
 * El *LED* (blanco) del *uC* enciende cada que el *uC* envía un mensaje al *broker*.

A continuación se muestra el circuito realizado:
[Figura del circuito]

A continuación se muestra la lectura de datos en el monitor serial de la *IDE* de *Arduino*:
[Figura del monitor serial]

A continuación se muestra la lectura de la temperatura en *OpenHAB*:
[Figura de la temp en OH]

A continuación se muestra la lectura de la humedad relativa en *OpenHAB*:
[Figura de la temp en OH]

A continuación se muestra una liga con información del sensor DHT11: [Sensor DHT11](https://naylampmechatronics.com/sensores-temperatura-y-humedad/57-sensor-de-temperatura-y-humedad-relativa-dht11.html)




