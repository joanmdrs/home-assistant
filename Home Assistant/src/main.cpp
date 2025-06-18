#include <WiFi.h>
#include <ArduinoHA.h>
#include "DHTesp.h"

#define PUBLISH_PERIOD   10000  // em msec
#define DHT_PIN          13
#define LED_BLUE_PIN     18
#define LED_GREEN_PIN    19
#define LED_RED_PIN      21
#define BROKER_ADDR      IPAddress(192,168,3,125)
#define BROKER_USERNAME   "usermqtt" 
#define BROKER_PASSWORD   "passmqtt"
#define WIFI_SSID        "Wokwi-GUEST"
#define WIFI_PASSWORD    ""

unsigned long lastTime = 0;
DHTesp dhtSensor;
WiFiClient client;

HADevice device("ESP32_Wokwi_01"); // unique_id do dispositivo
HAMqtt mqtt(client, device);
HASwitch led_red("esp32wokwi01_led_red");    // unique_id de cada componente
HASwitch led_green("esp32wokwi01_led_green");
HASwitch led_blue("esp32wokwi01_led_blue");
HASensor dhtSensorTemp("esp32wokwi01_temperature");
HASensor dhtSensorHumi("esp32wokwi01_humidity");

void onRedSwitchCommand(bool state, HASwitch* sender)
{
	Serial.print("LED red turning ");
	Serial.println((state ? "ON" : "OFF"));
	digitalWrite(LED_RED_PIN, (state ? HIGH : LOW));
	sender->setState(state);
}

void onGreenSwitchCommand(bool state, HASwitch* sender)
{
	Serial.print("LED green turning ");
	Serial.println((state ? "ON" : "OFF"));
	digitalWrite(LED_GREEN_PIN, (state ? HIGH : LOW));
	sender->setState(state); 
}

void onBlueSwitchCommand(bool state, HASwitch* sender)
{
	Serial.print("LED blue turning ");
	Serial.println((state ? "ON" : "OFF"));
	digitalWrite(LED_BLUE_PIN, (state ? HIGH : LOW));
	sender->setState(state); 
}

void setup() {
	Serial.begin(115200);
	Serial.print("Starting ");
	dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

	pinMode(LED_RED_PIN, OUTPUT);
	digitalWrite(LED_RED_PIN, LOW);
	pinMode(LED_GREEN_PIN, OUTPUT);
	digitalWrite(LED_GREEN_PIN, LOW);
	pinMode(LED_BLUE_PIN, OUTPUT);
	digitalWrite(LED_BLUE_PIN, LOW);

	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(500); // esperando por conexão
	}
	Serial.println();
	Serial.println("Connected to the network");
	Serial.print("Device ID: "); 
	Serial.println(device.getUniqueId());
	Serial.println("---");

	// detalhes do dispositivo
	device.setName("ESP32-Wokwi-01");
	device.setManufacturer("Do It Yourself (DIY)");
	device.setModel("ESP32-Wokwi");
	device.setSoftwareVersion("1.0.0");
	// disponibilidade do dispositivo depende da conexão MQTT
	device.enableSharedAvailability();
	device.setAvailability(true); 
	device.enableLastWill();

	// funções de callback para os estados dos LEDs
	led_red.onCommand(onRedSwitchCommand);
	led_red.setName("LED Vermelho");
	led_green.onCommand(onGreenSwitchCommand);
	led_green.setName("LED Verde");
	led_blue.onCommand(onBlueSwitchCommand);
	led_blue.setName("LED Azul");

	// define os atributos do componente Temperatura
	dhtSensorTemp.setName("Temperatura");
	dhtSensorTemp.setDeviceClass("temperature");
	dhtSensorTemp.setUnitOfMeasurement("°C");
	dhtSensorTemp.setIcon("mdi:temperature-celsius");
  
	// define os atributos do componente Umidade
	dhtSensorHumi.setName("Umidade");
	dhtSensorHumi.setDeviceClass("humidity");
	dhtSensorHumi.setUnitOfMeasurement("%");
	dhtSensorHumi.setIcon("mdi:water-percent");

	mqtt.begin(BROKER_ADDR, BROKER_USERNAME, BROKER_PASSWORD);
}

void loop() {
	mqtt.loop();

	if(millis() - lastTime > PUBLISH_PERIOD)  // A cada PUBLISH_PERIOD msec
	{
		lastTime = millis();

		TempAndHumidity  data = dhtSensor.getTempAndHumidity();
		String temp = String(data.temperature, 1);
		String humi = String(data.humidity, 1);
		Serial.println("Publishing DHT22 sensor data:");
		Serial.println("Temperature: " + temp + "°C");
		Serial.println("Humidity: " + humi + "%");
		Serial.println("---");
		dhtSensorTemp.setValue(temp.c_str());
		dhtSensorHumi.setValue(humi.c_str());
	}
}
