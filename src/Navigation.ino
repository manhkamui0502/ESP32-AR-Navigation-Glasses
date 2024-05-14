#include "ui.h"
#include <WiFi.h>
#include <lvgl.h>
#include <Arduino.h>
#include <LittleFS.h>
#include <TFT_eSPI.h>
#include <HTTPClient.h>
#include "NimBLEDevice.h"
#include <components/lv_fs_littlefs.h>

/* DEFINES*/
#define accessToken "access_token=pk.eyJ1IjoibWFuaGthbXVpMDUwMiIsImEiOiJjbHNid2R6YWQwZTloMnZsNTBxeG16Y2VnIn0.tnwgb_-j7RK3xU5KHAELeQ&attribution=false&logo=false"
#define mapStyle "https://api.mapbox.com/styles/v1/manhkamui0502/cltgjsc1c009i01qp24nyd3or/static/url-https:%2F%2Fi.postimg.cc%2FY9yMFxKy%2Foroginal-g.png"
#define SERVICE_UUID "DD3F0AD1-6239-4E1F-81F1-91F6C9F01D86"
#define WRITE_CHAR_UUID "DD3F0AD1-6239-4E1F-81F1-91F6C9F01D86"
#define READ_CHAR_UUID "DD3F0AD1-6239-4E1F-81F1-91F6C9F01D87"
const char *ssid = "username";
const char *password = "password";
/*BUTTON*/
#define SWITCHMODE 5

#define VBAT_PIN 26
#define BATTV_MAX 4.33 // maximum voltage of battery
#define BATTV_MIN 3.2  // what we regard as an empty battery
#define BATTV_LOW 3.4  // voltage considered to be low battery

/*SCREEN RESOLUTION*/
static uint32_t screenWidth = 160;
static uint32_t screenHeight = 80;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t disp_draw_buf[160 * 80 / 10];
static lv_disp_drv_t disp_drv;

/* TFT INSTANCE */
TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight);
NimBLECharacteristic *pCharacteristic;
NimBLECharacteristic *pCharacteristic2;
NimBLEServer *pServer;

/*NAVIGATE*/
float percent, speed, remainDistance, turnDistance;
int direction, mode, turn;
bool inFullMapMode = false, inFreeDrive = false;
int h, m, s;
std::string info;

/* VARIABLE*/
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool isNaviDataUpdated = false;
bool on_route = false;

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();
  lv_disp_flush_ready(disp);
}

class MyServerCallbacks : public NimBLEServerCallbacks
{
  void onConnect(NimBLEServer *pServer)
  {
    deviceConnected = true;
  };

  void onDisconnect(NimBLEServer *pServer)
  {
    deviceConnected = false;
    pServer->getAdvertising()->start();
  }
};

class WriteCharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
  void onWrite(NimBLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();
    if (value.find("wfcr") != std::string::npos)
    {
      std::string wifi_info = value.substr(5);
      size_t split_pos = wifi_info.find("/");
      lv_disp_load_scr(ui_connectionScreen);
      lv_img_set_src(ui_deviceImage, &ui_img_wifi_png);
      lv_label_set_text(ui_cText1, "Connecting");
      lv_label_set_text(ui_label, "Wifi");
      if (split_pos != std::string::npos)
      {
        std::string username = wifi_info.substr(0, split_pos);
        std::string password = wifi_info.substr(split_pos + 1);
        WiFi.begin(username.c_str(), password.c_str());
        lv_label_set_text(ui_cText2, username.c_str());
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED)
        {
          delay(500);
          attempts++;
          if (attempts > 10)
          {
            lv_label_set_text(ui_cText1, "Not connected");
            lv_obj_set_style_text_color(ui_cText1, lv_color_hex(0xFD0000), LV_PART_MAIN | LV_STATE_DEFAULT);
            delay(3500);
            break;
          }
          if (WiFi.status() == WL_CONNECTED)
          {
            lv_label_set_text(ui_cText1, "Connected");
            lv_label_set_text(ui_cText2, "\nFullmap ready!");
            lv_obj_set_style_text_color(ui_cText1, lv_color_hex(0x00FD27), LV_PART_MAIN | LV_STATE_DEFAULT);
            delay(2000);
          }
        }
        lv_disp_load_scr(ui_Screen1);
      }
    }
    else
    {
      info = value;
    }
  };
};

void extractData()
{
  if (info.find("110x70") != std::string::npos)
  {
    if (inFullMapMode == true)
    {
      Serial.println("in full map mode");
    }
  }
  else
  {
    if (inFullMapMode == false && inFreeDrive == false)
    {
      extractNavigationInfo(String(info.c_str()));
    }
    else if (inFullMapMode == false && inFreeDrive == true)
    {
      extractFreeDrive(String(info.c_str()));
    }
  }
  // Serial.println(String(info.c_str()));
}

void extractMapImage(String url)
{
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK)
  {
    fs::File file = LittleFS.open("/map.png", "w");
    while (http.getStreamPtr()->available())
    {
      file.write(http.getStreamPtr()->read());
    }
    file.close();
  }
  http.end();
  delay(200);
}

void extractNavigationInfo(String info)
{
  char data[info.length() + 1];
  info.toCharArray(data, info.length() + 1);
  char *token = strtok(data, "$");
  mode = atoi(token);
  token = strtok(NULL, "$");
  direction = atoi(token);
  token = strtok(NULL, "$");
  speed = atof(token);
  token = strtok(NULL, "$");
  turnDistance = atof(token);
  token = strtok(NULL, "$");
  remainDistance = atof(token);
}

void extractFreeDrive(String info)
{
  char data[info.length() + 1];
  info.toCharArray(data, info.length() + 1);
  char *token = strtok(data, "$");
  mode = atoi(token);
  token = strtok(NULL, "$");
  speed = atof(token);
  token = strtok(NULL, "$");
  h = atoi(token);
  token = strtok(NULL, "$");
  m = atoi(token);
  token = strtok(NULL, "$");
  s = atoi(token);
}

void setup()
{
  Serial.begin(9600);
  tft.begin();
  tft.invertDisplay(1);
  tft.setRotation(1);
  pinMode(SWITCHMODE, INPUT);

  lv_init();
  lv_png_init();
  lv_fs_littlefs_init();
  lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, screenWidth * screenHeight / 10);
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
  ui_init();
  ui_reset();
  LittleFS.begin();

  /*BLE Init*/
  NimBLEDevice::init("Navigation Glasses");
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  NimBLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(WRITE_CHAR_UUID, NIMBLE_PROPERTY::WRITE);
  pCharacteristic2 = pService->createCharacteristic(READ_CHAR_UUID, NIMBLE_PROPERTY::NOTIFY);
  pCharacteristic->setCallbacks(new WriteCharacteristicCallbacks());
  pService->start();
  pServer->getAdvertising()->start();
  Serial.println("BLE server started");
  fs::File file = LittleFS.open("/map.png", "w");
  file.close();
}

void loop()
{
  lv_task_handler();
  lv_timer_handler();
  displayBattery();
  if (!info.empty())
  {
    extractData();
  }
  if (deviceConnected)
  {
    if (digitalRead(SWITCHMODE) == 1)
    {
      while (digitalRead(SWITCHMODE) == 1)
      {
      }
      delay(100);
      if (WiFi.status() == WL_CONNECTED)
      {
        if (inFullMapMode == false)
        {
          pCharacteristic2->setValue("fullmapmode");
          pCharacteristic2->notify();
          lv_disp_load_scr(ui_Screen2);
          // Serial.println("Switch to full map mode");
          inFullMapMode = true;
        }
        else
        {
          pCharacteristic2->setValue("simplemode");
          pCharacteristic2->notify();
          idle_ui();
          lv_disp_load_scr(ui_Screen1);
          // Serial.println("Switch to simple mode");
          inFullMapMode = false;
        }
      }
      else
      {
        // Serial.println("Connect to wifi first!");
      }
    }
    routingStatus();
    if (on_route == true && inFreeDrive == false)
    {
      if (inFullMapMode == true)
      {
        extractMapImage(mapStyle + String(info.c_str()) + accessToken);
        lv_img_set_src(ui_mapImage, "F:/map.png");
        lv_disp_load_scr(ui_Screen2);
      }
      else
      {
        displayNavigationArrow(direction);
        displaySpeed(speed);
        displayTurnDistance(turnDistance);
        displayRemainDistance(remainDistance);
      }
    }
    else if (inFreeDrive == true && on_route == false)
    {
      if (inFullMapMode == true)
      {
        extractMapImage(mapStyle + String(info.c_str()) + accessToken);
        lv_img_set_src(ui_mapImage, "F:/map.png");
        lv_disp_load_scr(ui_Screen2);
      }
      else
      {
        displaySpeed(speed);
        displayTime();
      }
    }
  }

  // Kiểm tra nếu có kết nối mới hoặc mất kết nối
  if (deviceConnected != oldDeviceConnected)
  {
    if (deviceConnected)
    {
      if (on_route == true && inFullMapMode == false)
      {
        lv_disp_load_scr(ui_Screen1);
      }
      else
      {
        lv_img_set_src(ui_deviceImage, &ui_img_bluetooth);
        lv_label_set_text(ui_cText1, "Connected");
        lv_label_set_recolor(ui_cText1, "true");
        lv_obj_set_style_text_color(ui_cText1, lv_color_hex(0x00FD27), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(ui_cText2, "\nGlasses ready!");
      }
    }
    else
    {
      lv_img_set_src(ui_deviceImage, &ui_img_bluetooth);
      lv_label_set_text(ui_cText1, "Not Connected");
      lv_label_set_recolor(ui_cText1, "true");
      lv_obj_set_style_text_color(ui_cText1, lv_color_hex(0xFD0000), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_label_set_text(ui_cText2, "Connect via \nMobile App");
      lv_disp_load_scr(ui_connectionScreen);
    }
    oldDeviceConnected = deviceConnected;
  }
}

void ui_reset()
{
  lv_label_set_text(ui_direction, "No Route");
  lv_img_set_src(ui_turnArrow, &ui_img_start_png);
  lv_label_set_text(ui_speed, "0");
  lv_label_set_text(ui_distance, "0 m");
  lv_label_set_text(ui_remainDistance, "0 km");
}

void idle_ui()
{
  lv_label_set_text(ui_direction, "IDLE");
  lv_img_set_src(ui_turnArrow, &ui_img_start_png);
  lv_label_set_text(ui_distance, "0 m");
  lv_label_set_text(ui_remainDistance, "0 km");
}

void arrival_ui()
{
  lv_label_set_text(ui_direction, "Arrived");
  lv_img_set_src(ui_turnArrow, &ui_img_arrival_png);
  lv_label_set_text(ui_distance, "0 m");
  lv_label_set_text(ui_remainDistance, "0 km");
}

void displaySpeed(float speed)
{
  lv_label_set_text(ui_speed, String(speed, 1).c_str());
}

void displayTurnDistance(float distance)
{
  if (distance > 1000)
  {
    distance = distance / 1000;
    lv_label_set_text(ui_distance, (String(distance, 1) + " km").c_str());
  }
  else if (distance > 99)
  {
    lv_label_set_text(ui_distance, (String(distance, 0) + " m").c_str());
  }
  else
  {
    lv_label_set_text(ui_distance, (String(distance, 1) + " m").c_str());
  }
}

void displayRemainDistance(float distance)
{
  if (distance > 1000)
  {
    distance = distance / 1000;
    lv_label_set_text(ui_remainDistance, (String(distance, 1) + " km").c_str());
  }
  else if (distance > 99)
  {
    lv_label_set_text(ui_remainDistance, (String(distance, 0) + " m").c_str());
  }
  else
  {
    lv_label_set_text(ui_remainDistance, (String(distance, 1) + " m").c_str());
  }
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void displayBattery()
{
  float battValue = analogRead(VBAT_PIN);
  float voltage = (((battValue * 3.3) / 4095) * 2) + 0.34;
  percent = mapfloat(voltage, 3.3, 4.25, 0, 100);
  if (percent > 80)
  {
    lv_img_set_src(ui_batteryImage, &ui_img_battery_indicator_100_png);
    lv_img_set_src(ui_batteryImage2, &ui_img_battery_indicator_100_png);
  }
  else if (percent <= 80 && percent > 60)
  {
    lv_img_set_src(ui_batteryImage, &ui_img_battery_indicator_80_png);
    lv_img_set_src(ui_batteryImage2, &ui_img_battery_indicator_80_png);
  }
  else if (percent <= 60 && percent > 40)
  {
    lv_img_set_src(ui_batteryImage, &ui_img_battery_indicator_60_png);
    lv_img_set_src(ui_batteryImage2, &ui_img_battery_indicator_60_png);
  }
  else if (percent <= 40 && percent > 20)
  {
    lv_img_set_src(ui_batteryImage, &ui_img_battery_indicator_40_png);
    lv_img_set_src(ui_batteryImage2, &ui_img_battery_indicator_40_png);
  }
  else if (percent <= 20 && percent > 0)
  {
    lv_img_set_src(ui_batteryImage, &ui_img_battery_indicator_20_png);
    lv_img_set_src(ui_batteryImage2, &ui_img_battery_indicator_20_png);
  }

  /*Serial.print("Analog Value = ");
  Serial.print(sensorValue);
  Serial.print("\t Output Voltage = ");
  Serial.print(voltage);
  Serial.print("\t Battery Percentage = ");
  Serial.println(percent);*/
}

void routingStatus()
{
  if (mode == 1 && on_route == false)
  {
    lv_disp_load_scr(ui_Screen1);
    on_route = true;
    inFreeDrive = false;
  }
  else if (mode == 2 && inFreeDrive == false)
  {
    lv_disp_load_scr(ui_Screen1);
    inFreeDrive = true;
  }
  else if (mode == 3)
  {
  }
  else if (mode == 4)
  {
    lv_disp_load_scr(ui_Screen1);
    on_route == false;
    inFreeDrive = false;
  }
  else if (mode == 0)
  {
    inFreeDrive = false;
    on_route = false;
    idle_ui();
  }
}

void displayNavigationArrow(int d)
{
  switch (d)
  {
  case 1:
    lv_label_set_text(ui_direction, "Go Straight");
    lv_img_set_src(ui_turnArrow, &ui_img_straight_png);
    break;

  case 2:
    lv_label_set_text(ui_direction, "Turn Left");
    lv_img_set_src(ui_turnArrow, &ui_img_left_end_png);
    break;

  case 3:
    lv_label_set_text(ui_direction, "Turn Left");
    lv_img_set_src(ui_turnArrow, &ui_img_slight_left_png);
    break;

  case 4:
    lv_label_set_text(ui_direction, "Turn Left");
    lv_img_set_src(ui_turnArrow, &ui_img_continue_left_png);
    break;

  case 5:
    lv_label_set_text(ui_direction, "Turn Left");
    lv_img_set_src(ui_turnArrow, &ui_img_off_ramp_slight_left_png);
    break;

  case 6:
    lv_label_set_text(ui_direction, "Turn Right");
    lv_img_set_src(ui_turnArrow, &ui_img_right_end_png);
    break;

  case 7:
    lv_label_set_text(ui_direction, "Turn Right");
    lv_img_set_src(ui_turnArrow, &ui_img_slight_right_png);
    break;

  case 8:
    lv_label_set_text(ui_direction, "Turn Right");
    lv_img_set_src(ui_turnArrow, &ui_img_continue_right_png);
    break;

  case 9:
    lv_label_set_text(ui_direction, "Turn Right");
    lv_img_set_src(ui_turnArrow, &ui_img_off_ramp_slight_right_png);
    break;

  case 10:
    lv_label_set_text(ui_direction, "UTurn");
    lv_img_set_src(ui_turnArrow, &ui_img_left_uturn_png);
    break;

  case 11:
    lv_label_set_text(ui_direction, "UTurn");
    lv_img_set_src(ui_turnArrow, &ui_img_right_uturn_png);
    break;

  case 12:
    lv_label_set_text(ui_direction, "Go Straight");
    lv_img_set_src(ui_turnArrow, &ui_img_round_straight_180_png);
    break;

  case 13:
    lv_label_set_text(ui_direction, "Turn Left");
    lv_img_set_src(ui_turnArrow, &ui_img_round_left_270_png);
    break;

  case 14:
    lv_label_set_text(ui_direction, "Turn Right");
    lv_img_set_src(ui_turnArrow, &ui_img_round_right_90_png);
    break;

  case 15:
    lv_label_set_text(ui_direction, "Arrived");
    lv_img_set_src(ui_turnArrow, &ui_img_arrival_png);
    on_route = false;
    arrival_ui();
    break;

  case 16:
    lv_label_set_text(ui_direction, "Starting");
    lv_img_set_src(ui_turnArrow, &ui_img_start_png);
    on_route = false;
    break;

  case 17:
    break;

  default:
    idle_ui();
    on_route = false;
    break;
  }
}

void displayTime()
{
  char str[20];
  sprintf(str, "%d:%d:%d", h, m, s);
  lv_label_set_text(ui_direction, String(str).c_str());
  lv_img_set_src(ui_turnArrow, &ui_img_start_png);
  lv_label_set_text(ui_distance, "- - -");
  lv_label_set_text(ui_remainDistance, "- - -");
}