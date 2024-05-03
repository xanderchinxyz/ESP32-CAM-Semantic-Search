#include <WiFi.h>               // WiFi library
#include <AsyncTCP.h>           // 
#include <ESPAsyncWebServer.h>  // WebServer library
#include "esp_camera.h"         // ESP32-CAM library
#include "FS.h"                 // SD Card ESP32
#include "SD_MMC.h"             // SD Card ESP32
#include <HTTPClient.h>         // library for POST requests
#include <ESP32Time.h>          // Time libraries
#include "time.h"

ESP32Time rtc;

// WiFi ssid and passwords
const char *ssid = "FourJARS_2.4";
const char *password = "jinnxanderrachelsong";

// loop variables
uint32_t lastCapture = 0;
bool canCapture = false;

AsyncWebServer server(80);

//Definitions:
#define NUM_FLOATS 512          // number of dimensions in the vector returned by CLIP
#define CAPTURE_INTERVAL 10000  // capture interval in milliseconds
const String backendServer = "http://192.168.1.139:8000";

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void initSDCard() {
  if(!SD_MMC.begin("/sdcard", true)){
    Serial.println("SD Card Mount Failed");
    return;
  }
  
  uint8_t cardType = SD_MMC.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

void initWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
}

void initCamera() {
  // OV2640 camera module
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if(psramFound()) {
    Serial.println("psram found");
    config.frame_size = FRAMESIZE_XGA;
    config.jpeg_quality = 10;
    config.fb_count = 3;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }

  //change camera settings
  sensor_t * s = esp_camera_sensor_get();
  s->set_brightness(s, 0);     // -2 to 2
  s->set_contrast(s, 2);       // -2 to 2
  s->set_saturation(s, -2);     // -2 to 2
  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
  s->set_aec2(s, 0);           // 0 = disable , 1 = enable
  s->set_ae_level(s, 0);       // -2 to 2
  s->set_aec_value(s, 300);    // 0 to 1200
  s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  s->set_agc_gain(s, 0);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
  s->set_bpc(s, 0);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  s->set_vflip(s, 0);          // 0 = disable , 1 = enable
  s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
}

void takePicture() {
  //get time
  uint32_t currentTime = rtc.getEpoch();
  
  //take picture
  camera_fb_t * fb = NULL; // pointer
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
  }
  delay(1000);  //safety delay
  
  //send image to backend server
  HTTPClient http;
  http.begin(backendServer + "/calculate_image/");  // Specify the destination server
  uint16_t httpResponseCode = http.POST(fb->buf, fb->len); // Send the POST request
  
  // Check for errors and get response
  String response;
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    response = http.getString(); // Get the response payload
  } else {
    Serial.print("Error in POST request. HTTP Response code: ");
    Serial.println(httpResponseCode);
  }
  http.end(); // Close connection

  //split the response into a list of floats (it works but maybe I should do this in proper C?)
  uint16_t len = response.length();
  response = response.substring(1, len-1);
  float floatList[NUM_FLOATS];
  for(uint16_t x = 0; x < NUM_FLOATS-1; x++) {
    uint8_t nextComma = response.indexOf(',');
    floatList[x] = response.substring(0, nextComma).toFloat();
    response = response.substring(nextComma+1);
  }
  floatList[NUM_FLOATS-1] = response.toFloat();

  //Storing list as floats (32 bits) in a binary file - could store as 16 bit float if low on space but no need yet
  File file = SD_MMC.open("/vectors.bin", FILE_APPEND);
  if (file) { //Check if the file opened successfully
    file.write((uint8_t*)&currentTime, sizeof(uint32_t));
    file.write((uint8_t*)floatList, sizeof(floatList));   // Write the floats as binary data to the file
    file.close();
    Serial.println("Float list saved to binary file.");
  } else {
    Serial.println("Error opening file.");
  }

  //save picture to file
  file = SD_MMC.open("/Pictures/" + String(currentTime) + ".jpg", FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file in writing mode");
  } else {
    file.write(fb->buf, fb->len); // payload (image), payload length
  }
  file.close();
  esp_camera_fb_return(fb);
}

void setup() {
  Serial.begin(115200);
  initWiFi();
  initSDCard();
  initCamera();

  //Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    canCapture = false;
    request->send(SD_MMC, "/index.html", "text/html");
  });

  //load any non-form POST request body and url
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    Serial.println(request->url());
    if (request->url() == "/capture-on") {
      canCapture = true;
    } else if(request->url() == "/capture-off") {
      canCapture = false;
    } else if(request->url() == "/delete-pictures") {
      //clear vectors.bin
      File file = SD_MMC.open("/vectors.bin", FILE_WRITE);
      file.print("");

      //delete all pictures in /Pictures
      File root = SD_MMC.open("/Pictures");
      file = root.openNextFile();
      while(file){
          SD_MMC.remove("/Pictures/" + String(file.name()));
          file = root.openNextFile();
      }
    } else if(request->url() == "/send-time") {
      //set time on esp32 rtc
      String s = "";
      for(size_t i=0; i<len; i++) {
        s += char(data[i]);
      }

      uint16_t timeDate[6];
      for(uint8_t x = 0; x < 5; x++) {
        uint8_t nextComma = s.indexOf(',');
        timeDate[x] = s.substring(0, nextComma).toInt();
        s = s.substring(nextComma+1);
      }
      timeDate[5] = s.toInt();
      rtc.setTime(timeDate[0], timeDate[1], timeDate[2], timeDate[3], timeDate[4], timeDate[5]);
      Serial.println(rtc.getTimeDate());
    }
  });

  server.serveStatic("/", SD_MMC, "/");
  server.begin();
}

void loop() {
  if(canCapture && millis() - lastCapture > CAPTURE_INTERVAL) {
    lastCapture = millis();
    takePicture();
  }
}

void readFloatListFromFile(const char* fileName) {
  // Open the file for reading
  File file = SD_MMC.open(fileName, FILE_READ);

  // Check if the file opened successfully
  if (!file) {
    Serial.println("Error opening file.");
    return;
  }

  // Create a buffer to store the binary data
  uint8_t buff[sizeof(float)];
  uint8_t timeBuff[sizeof(uint32_t)];

  //read the currentTime off of the file first
  file.read(timeBuff, sizeof(uint32_t)); 
  uint32_t currentTime;
  memcpy(&currentTime, timeBuff, sizeof(uint32_t));
  Serial.println("Time: " + currentTime);
  // Read and interpret each float value
  for (size_t i = 0; i < NUM_FLOATS; i++) {
    // Read a float value from the file
    file.read(buff, sizeof(float));

    // Interpret the binary data as a float
    float floatValue;
    memcpy(&floatValue, buff, sizeof(float));

    // Print the float value
//    Serial.print("Float value ");
//    Serial.print(i);
//    Serial.print(": ");
//    Serial.println(floatValue, 6);
  }

  // Close the file
  file.close();
}
