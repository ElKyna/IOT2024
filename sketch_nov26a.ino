#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Configuración del DHT11
#define DHTPIN 14         // Pin digital al que está conectado el DHT11 (GPIO 14)
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Configuración del MQ-135
#define MQ135_PIN 34        // Pin analógico del MQ-2 (GPIO 34, ADC1)

// Configuración del LED RGB
#define LED_R 25          // Canal rojo conectado al GPIO 25
#define LED_G 26          // Canal verde conectado al GPIO 26

// Configuración del Buzzer
#define BUZZER_PIN 23     // Pin digital para el buzzer (GPIO 23)

// Umbrales
#define UMBRAL_GAS 2500   // Umbral de nivel alto de gas (peligroso)

// Wi-Fi
const char* ssid = "SensorGas";  // Reemplaza con el nombre de tu red Wi-Fi
const char* password = "sensor";  // Reemplaza con la contraseña d Wi-Fi

// MQTT
const char* mqtt_server = "mqtt.eclipseprojects.io"; // Dirección del broker MQTT
const char* topic_gas = "sensor/gas";
const char* topic_temp = "sensor/temperature";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Configuración de pines
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  setColor(0, 0);

  // Conectar a Wi-Fi
  connectWiFi();

  // Configurar MQTT
  client.setServer(mqtt_server, 1883); // Puerto por defecto de MQTT
}

void loop() {
  // Asegurar que el cliente MQTT esté conectado
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // Leer datos del DHT11 y MQ-2
  float temperatura = dht.readTemperature();
  int mq2Value = analogRead(MQ2_PIN);

  // Mostrar datos en Serial Monitor
  Serial.print("Temperatura: ");
  Serial.println(temperatura);
  Serial.print("Nivel de Gas (MQ-2): ");
  Serial.println(mq2Value);

  // Publicar datos a Node-RED (MQTT)
  client.publish(topic_temp, String(temperatura).c_str());
  client.publish(topic_gas, String(mq2Value).c_str());

  // Control del LED y Buzzer según el nivel de gas
  if (mq2Value > UMBRAL_GAS) {
    setColor(255, 0); // Rojo: Nivel peligroso
    activateBuzzer();
  } else {
    setColor(0, 255); // Verde: Nivel seguro
    deactivateBuzzer();
  }

  delay(2000); // Esperar 2 segundos antes de la próxima iteración
}

// Función para establecer el color del LED RGB
void setColor(int red, int green) {
  analogWrite(LED_R, red);
  analogWrite(LED_G, green);
}

// Función para activar el buzzer
void activateBuzzer() {
  tone(BUZZER_PIN, 2000);
}

// Función para desactivar el buzzer
void deactivateBuzzer() {
  noTone(BUZZER_PIN);
}

// Función para conectar a Wi-Fi
void connectWiFi() {
  Serial.print("Conectando a Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado.");
}

// Función para reconectar al broker MQTT
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando a MQTT...");
    if (client.connect("ESP32_Client")) {
      Serial.println("Conectado.");
    } else {
      Serial.print("Error, rc=");
      Serial.print(client.state());
      Serial.println(" Intentando de nuevo en 5 segundos.");
      delay(5000);
    }
  }
}
