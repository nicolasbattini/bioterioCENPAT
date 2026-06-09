#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>   // <-- Importante para HTTPS
#include <OneWire.h>
#include <DallasTemperature.h>

// ---------- WiFi credentials ----------
// const char* ssid = "Martina"; # Prueba en casa 
// const char* password = "Zimba123";
const char* ssid = "OCTOPUS";
const char* password = "patagonico";

// ---------- Google Sheets Web App URL (https://...) ----------
const char* googleScriptURL = "https://script.google.com/macros/s/AKfycbw_nOGbdNBlFVBm_gEECXBjiBi8zrlCnnvc2A_g5pyiW0lpEExVXR5cNiU851JhnUKa/exec";

// ---------- Sensores ----------
OneWire ourWire(2);               // Pin D4 (GPIO2)
DallasTemperature sensors(&ourWire);

// ---------- Cliente seguro para HTTPS ----------
WiFiClientSecure secureClient;
HTTPClient http;

// ---------- Intervalo de muestreo ----------
const unsigned long interval = 10000;
unsigned long previousMillis = 0;

// ---------- Función para verificar/reconectar WiFi ----------
void mantenerWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("WiFi perdido. Reconectando...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 20) {
      delay(500);
      Serial.print(".");
      intentos++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi reconectado. IP: " + WiFi.localIP().toString());
    } else {
      Serial.println("\nNo se pudo reconectar WiFi.");
    }
  }
}

// ---------- Envío de datos a Google Sheets ----------
void sendToGoogleSheets(float temp) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi no disponible. Reintentando en la próxima ronda.");
    return;
  }

  // --- Configuración específica para manejar la redirección 302 ---
  // 1. Habilitar el seguimiento automático de redirecciones
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  // 2. Forzar el uso de una nueva conexión para evitar conflictos
  http.setReuse(false);
  // -------------------------------------------------------------

  // Configurar cliente seguro
  secureClient.setInsecure();
  secureClient.setTimeout(5000);

  // Iniciar la conexión
  bool httpBeginOk = http.begin(secureClient, googleScriptURL);
  if (!httpBeginOk) {
    Serial.println("Error al iniciar HTTP (URL inválida o falta de memoria).");
    return;
  }

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String postData = "temp=" + String(temp);

  int httpCode = http.POST(postData);
  Serial.print("Heap libre: ");
  Serial.println(ESP.getFreeHeap());

  // Verificar el código de resultado final después de la/s redirección/es
  if (httpCode == 200) {
    Serial.println("Envío exitoso de datos.");
    // Puedes ver la respuesta del servidor si tu script de Google Apps Script la envía
    // String response = http.getString();
    // Serial.println("Respuesta: " + response);
  } else {
    // Si aún falla, puede ser por otras razones (URL incorrecta, script mal configurado, etc.)
    Serial.printf("[HTTP] POST falló, código: %d, error: %s\n", httpCode, http.errorToString(httpCode).c_str());
  }

  http.end();
}

// ---------- Setup ----------
void setup() {
  Serial.begin(9600);
  delay(1000);

  // Configurar WiFi
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  WiFi.begin(ssid, password);

  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado. IP: " + WiFi.localIP().toString());

  // Inicializar sensor
  sensors.begin();
  Serial.println("Sensores listos.");
}

// ---------- Loop ----------
void loop() {
  mantenerWiFi();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);

    if (isnan(temp) || temp == -127.0) {
      Serial.println("Error en la lectura del sensor.");
      temp = -999;
    }

    Serial.println("\n--- Leyendo sensores ---");
    Serial.print("DS18B20: ");
    Serial.println(temp);

    sendToGoogleSheets(temp);
  }

  delay(10);
}