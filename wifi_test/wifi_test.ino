#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32_WROVER_DASHBOARD";
const char* password = ""; 

WebServer server(80);

int led2State = 0;
int led4State = 0;

#include "page.h"

// page.h contains: const char INDEX_HTML[] PROGMEM = R"=====(...)=====" ;
// Moved to separate .h file to prevent Arduino preprocessor from injecting
// #line directives into the JavaScript code.

// ---- verify that the closing tag is also removed ----
// The HTML block ends at )====="; which is now in page.h

static_assert(sizeof(INDEX_HTML) > 100, "page.h loaded");


// ========== C++ BACKEND ==========

volatile bool scope_running = false;
volatile int shared_timebase = 10;
volatile int shared_trig_lvl = 2048;
volatile int shared_trig_edge = 1;
volatile int shared_trig_mode = 0;

volatile bool sim_enabled = false;
volatile int sim_wave_type = 0;
volatile float sim_freq = 100.0;
volatile float sim_amplitude = 1.5;

volatile bool dac_enabled = false;
volatile int dac_freq = 1000;
volatile bool dac_pending_change = false;
volatile bool dac_pending_enable = false;
volatile int dac_pending_freq = 1000;
volatile int dac_atten = 0;
volatile int dac_pending_atten = 0;

#include "driver/dac_cosine.h"
dac_cosine_handle_t global_dac_handle = NULL;

const int NUM_SAMPLES = 400;
int shared_samples[NUM_SAMPLES] = {0};
volatile bool shared_triggered = false;
volatile uint32_t shared_elapsed_us = 8000;
volatile bool shared_new_data = false;
portMUX_TYPE scopeMux = portMUX_INITIALIZER_UNLOCKED;

void scopeTask(void *pvParameters) {
  while (true) {
    if (scope_running) {
      bool is_sim = false;
      portENTER_CRITICAL(&scopeMux);
      is_sim = sim_enabled;
      portEXIT_CRITICAL(&scopeMux);

      if (is_sim) {
        int t_base, t_lvl, t_edge, t_mode;
        float s_freq, s_amp;
        int s_wave;

        portENTER_CRITICAL(&scopeMux);
        t_base = shared_timebase;
        t_lvl = shared_trig_lvl;
        t_edge = shared_trig_edge;
        t_mode = shared_trig_mode;
        s_freq = sim_freq;
        s_amp = sim_amplitude;
        s_wave = sim_wave_type;
        portEXIT_CRITICAL(&scopeMux);

        int local_samples[NUM_SAMPLES];
        float samplePeriod = (t_base + 10) / 1000000.0;

        static unsigned long simStartMs = 0;
        if (simStartMs == 0) simStartMs = millis();
        double baseTime = (millis() - simStartMs) / 1000.0;

        for (int i = 0; i < NUM_SAMPLES; i++) {
          double t = baseTime + i * samplePeriod;
          double angle = 2.0 * PI * s_freq * t;
          double V = 1.65;

          if (s_wave == 0) V = 1.65 + s_amp * sin(angle);
          else if (s_wave == 1) V = 1.65 + (sin(angle) >= 0 ? s_amp : -s_amp);
          else if (s_wave == 2) V = 1.65 + s_amp * (2.0 / PI) * asin(sin(angle));
          else if (s_wave == 3) V = 1.65 + s_amp * (2.0 * fmod(t * s_freq, 1.0) - 1.0);
          else if (s_wave == 4) {
            double ph = fmod(t * s_freq, 1.0);
            double ecg = 0.0;
            if (ph < 0.10) ecg = 0.15 * sin(ph * 10.0 * PI);
            else if (ph < 0.12) ecg = -0.2 * sin((ph - 0.1) * 50.0 * PI);
            else if (ph < 0.15) ecg = 1.0 * sin((ph - 0.12) * 33.3 * PI);
            else if (ph < 0.18) ecg = -0.3 * sin((ph - 0.15) * 33.3 * PI);
            else if (ph >= 0.22 && ph < 0.35) ecg = 0.25 * sin((ph - 0.22) * 7.7 * PI);
            V = 1.65 + s_amp * ecg;
          } else if (s_wave == 5) V = 1.65 + (((rand() % 200) - 100) / 100.0) * s_amp;

          int adc_val = (int)(V * 4095.0 / 3.3);
          if (adc_val < 0) adc_val = 0;
          if (adc_val > 4095) adc_val = 4095;
          local_samples[i] = adc_val;
        }

        uint32_t sim_elapsed = (uint32_t)((t_base + 10) * NUM_SAMPLES);
        portENTER_CRITICAL(&scopeMux);
        memcpy(shared_samples, local_samples, sizeof(shared_samples));
        shared_triggered = true;
        shared_elapsed_us = sim_elapsed;
        shared_new_data = true;
        portEXIT_CRITICAL(&scopeMux);
        vTaskDelay(pdMS_TO_TICKS(35));

      } else {
        int t_base, t_lvl, t_edge, t_mode;
        portENTER_CRITICAL(&scopeMux);
        t_base = shared_timebase;
        t_lvl = shared_trig_lvl;
        t_edge = shared_trig_edge;
        t_mode = shared_trig_mode;
        portEXIT_CRITICAL(&scopeMux);

        bool triggered = false;
        unsigned long timeout = (t_mode == 1) ? 80 : 30;
        int retries = (t_mode == 1) ? 1 : 3;

        for (int attempt = 0; attempt < retries && !triggered; attempt++) {
          unsigned long startWait = millis();
          int lastVal = analogRead(34);
          while (millis() - startWait < timeout) {
            int val = analogRead(34);
            if (t_edge == 1) { if (lastVal < t_lvl && val >= t_lvl) { triggered = true; break; } }
            else { if (lastVal > t_lvl && val <= t_lvl) { triggered = true; break; } }
            lastVal = val;
          }
        }

        uint32_t start_t = micros();
        int local_samples[NUM_SAMPLES];
        for (int i = 0; i < NUM_SAMPLES; i++) {
          local_samples[i] = analogRead(34);
          if (t_base > 0) {
            if (t_base >= 1000) vTaskDelay(pdMS_TO_TICKS(t_base / 1000));
            else delayMicroseconds(t_base);
          }
        }
        uint32_t elapsed = micros() - start_t;

        portENTER_CRITICAL(&scopeMux);
        memcpy(shared_samples, local_samples, sizeof(shared_samples));
        shared_triggered = triggered;
        shared_elapsed_us = elapsed;
        shared_new_data = true;
        portEXIT_CRITICAL(&scopeMux);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void handleRoot() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send_P(200, "text/html; charset=utf-8", (const char*)INDEX_HTML);
}

void handleScopeApi() {
  portENTER_CRITICAL(&scopeMux);
  if (server.hasArg("timebase")) shared_timebase = server.arg("timebase").toInt();
  if (server.hasArg("trig_lvl")) shared_trig_lvl = server.arg("trig_lvl").toInt();
  if (server.hasArg("trig_edge")) shared_trig_edge = server.arg("trig_edge").toInt();
  if (server.hasArg("trig_mode")) shared_trig_mode = server.arg("trig_mode").toInt();
  scope_running = true;
  portEXIT_CRITICAL(&scopeMux);

  int wait_count = 0;
  while (!shared_new_data && wait_count < 15) { delay(5); wait_count++; }

  int local_samples[NUM_SAMPLES];
  bool local_triggered;
  uint32_t local_elapsed;
  portENTER_CRITICAL(&scopeMux);
  memcpy(local_samples, shared_samples, sizeof(local_samples));
  local_triggered = shared_triggered;
  local_elapsed = shared_elapsed_us;
  shared_new_data = false;
  portEXIT_CRITICAL(&scopeMux);

  String json; json.reserve(3000);
  json = "{\"triggered\":" + String(local_triggered ? "true" : "false") + 
         ",\"elapsed_us\":" + String(local_elapsed) + 
         ",\"samples\":[";
  for (int i = 0; i < NUM_SAMPLES; i++) {
    json += String(local_samples[i]);
    if (i < NUM_SAMPLES - 1) json += ",";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void handleLedToggle() {
  if (server.hasArg("pin") && server.hasArg("state")) {
    int pin = server.arg("pin").toInt();
    int state = server.arg("state").toInt();
    if (pin == 2 || pin == 4) {
      digitalWrite(pin, state);
      if (pin == 2) led2State = state;
      if (pin == 4) led4State = state;
      server.send(200, "text/plain", "OK");
      return;
    }
  }
  server.send(400, "text/plain", "Bad Request");
}

void handleSystemApi() {
  String json = "{";
  json += "\"led2\":" + String(led2State) + ",";
  json += "\"led4\":" + String(led4State) + ",";
  json += "\"sram_total\":" + String(ESP.getHeapSize()) + ",";
  json += "\"sram_free\":" + String(ESP.getFreeHeap()) + ",";
  json += "\"psram_total\":" + String(ESP.getPsramSize()) + ",";
  json += "\"psram_free\":" + String(ESP.getFreePsram()) + ",";
  json += "\"model\":\"" + String(ESP.getChipModel()) + "\",";
  json += "\"cores\":" + String(ESP.getChipCores()) + ",";
  json += "\"cpu_freq\":" + String(ESP.getCpuFreqMHz()) + ",";
  json += "\"uptime\":" + String(millis());
  json += "}";
  server.send(200, "application/json", json);
}

void handleGeneratorApi() {
  portENTER_CRITICAL(&scopeMux);
  if (server.hasArg("sim_enable")) sim_enabled = (server.arg("sim_enable").toInt() == 1);
  if (server.hasArg("sim_wave")) sim_wave_type = server.arg("sim_wave").toInt();
  if (server.hasArg("sim_freq")) sim_freq = server.arg("sim_freq").toFloat();
  if (server.hasArg("sim_amp")) sim_amplitude = server.arg("sim_amp").toFloat();

  if (server.hasArg("dac_enable")) {
    dac_pending_enable = (server.arg("dac_enable").toInt() == 1);
    dac_pending_change = true;
  }
  if (server.hasArg("dac_freq")) {
    dac_pending_freq = server.arg("dac_freq").toInt();
    dac_pending_change = true;
  }
  if (server.hasArg("dac_atten")) {
    dac_pending_atten = server.arg("dac_atten").toInt();
    dac_pending_change = true;
  }
  portEXIT_CRITICAL(&scopeMux);

  if (dac_pending_change) {
    dac_pending_change = false;
    if (dac_pending_enable != dac_enabled || (dac_pending_enable && (dac_pending_freq != dac_freq || dac_pending_atten != dac_atten))) {
      dac_enabled = dac_pending_enable;
      dac_freq = dac_pending_freq;
      dac_atten = dac_pending_atten;

      if (dac_enabled) {
        ledcDetach(25); 
        if (global_dac_handle != NULL) {
            dac_cosine_stop(global_dac_handle);
            dac_cosine_del_channel(global_dac_handle);
            global_dac_handle = NULL;
        }
        
        dac_cosine_config_t cos_cfg = {
            .chan_id = DAC_CHAN_0, // DAC_CHAN_0 is GPIO25 on ESP32
            .freq_hz = (uint32_t)dac_freq,
            .clk_src = DAC_COSINE_CLK_SRC_DEFAULT,
            .atten = (dac_cosine_atten_t)dac_atten,
            .phase = DAC_COSINE_PHASE_0,
            .offset = 0,
            .flags = {
                .force_set_freq = false
            }
        };
        
        Serial.printf("[DAC] Initializing Cosine Wave Generator on GPIO25 (DAC_CHAN_0): Freq=%d Hz, Atten=%d\n", dac_freq, dac_atten);
        esp_err_t err = dac_cosine_new_channel(&cos_cfg, &global_dac_handle);
        if (err == ESP_OK) {
            err = dac_cosine_start(global_dac_handle);
            Serial.printf("[DAC] Started successfully, status: %d (ESP_OK)\n", err);
        } else {
            Serial.printf("[DAC] Failed to create channel, err code: 0x%X\n", err);
        }
      } else {
        Serial.println("[DAC] Disabling generator. Setting GPIO25 to INPUT (high impedance).");
        if (global_dac_handle != NULL) {
            dac_cosine_stop(global_dac_handle);
            dac_cosine_del_channel(global_dac_handle);
            global_dac_handle = NULL;
        }
        ledcDetach(25);
        pinMode(25, INPUT);
      }
    }
  }
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);

  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(2, LOW);
  digitalWrite(4, LOW);
  pinMode(34, INPUT);

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  Serial.println("\n--- ESP32 WROVER Pro Dashboard & Scope ---");

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP: "); Serial.println(ssid);
  Serial.print("URL: http://"); Serial.println(IP);

  xTaskCreatePinnedToCore(scopeTask, "ScopeTask", 8192, NULL, 5, NULL, 1);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/system", HTTP_GET, handleSystemApi);
  server.on("/api/led", HTTP_POST, handleLedToggle);
  server.on("/api/scope", HTTP_GET, handleScopeApi);
  server.on("/api/generator", handleGeneratorApi);

  server.begin();
  Serial.println("Server started!");
}

void loop() {
  server.handleClient();
}
