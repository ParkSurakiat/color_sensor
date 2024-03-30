 #include <Arduino.h>
#include "concount.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <math.h>
#include <ModbusIP_ESP8266.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// https://github.com/birdtechstep/modbus-esp.git
/*
  ESP32 TCS3200 Color sensor
  http:://www.electronicwings.com
*/

const char *apSSID = "ESP32-Access-Point";
const char *apPassword = "password";

String ssid = "";
String password = "";


// const char* ssid = "AUTOMATION_IOT_2";
// const char* password = "admin12345";

AsyncWebServer server(80);

ModbusIP mb;

Eloquent::ML::Port::LogisticRegression clft;

#define S2 0          /*Define S2 Pin Number of ESP32*/
#define S3 4          /*Define S3 Pin Number of ESP32*/
#define sensorOut 16  /*Define Sensor Output Pin Number of ESP32*/
#define Num_Arry 5  // เก็บค่าตัว Sample ของแต่ละสี (จำนวน Sample)

/*Enter the Minimum and Maximum Values which getting from Calibration Code*/
unsigned long previousMillis = 0;
const long interval = 3000;  // 500 milliseconds delay

int R_Min = 13;  /*Red minimum value*/
int R_Max = 315; /*Red maximum value*/
int G_Min = 14;  /*Green minimum value*/
int G_Max = 341; /*Green maximum value*/
int B_Min = 12;  /*Blue minimum value*/
int B_Max = 313; /*Blue maximum value*/
int C_Min = 4;
int C_Max = 106;
int ClearT =0;

int Find = 0;

/*Define int variables*/
int Red = 0;
int Green = 0;
int Blue = 0;
int Clear = 0;

int R[15];
int G[15];
int B[15];
int C[15];

int redValue;
int greenValue;
int blueValue;
int clearValue;
int Frequency;
int color_num=0;
float rgb_Value[] = {redValue,greenValue,blueValue,clearValue};

int c_white;
int c_black;
int c_brown;
int c_yellow;
int c_green;
int c_blue;
int c_red;
int c_orange;
int c_pink;
int c_waste;
int c_unknown;
int c_purple;

int total_all=0;
int total_white=0;
int total_black=0;
int total_brown=0;
int total_yellow=0;
int total_green=0;
int total_blue=0;
int total_red=0;
int total_orange=0;
int total_pink=0;
int total_waste=0;
int total_purple=0;
int before_color=0;
int recent_color=0;

String string_total_all = String(total_all);
String string_total_white = String(total_white);
String string_total_black = String(total_black);
String string_total_brown = String(total_brown);
String string_total_yellow = String(total_yellow);
String string_total_green = String(total_green);
String string_total_blue = String(total_blue);
String string_total_red = String(total_red);
String string_total_orange = String(total_orange);
String string_total_pink = String(total_pink);
String string_total_waste = String(total_waste);
String string_total_purple = String(total_purple);
String string_before_color = String(before_color);

int Wifi_status = 0;

const int total_all_MOD=12;
const int total_white_MOD=1;
const int total_black_MOD=2;
const int total_brown_MOD=3;
const int total_yellow_MOD=4;
const int total_green_MOD=5;
const int total_blue_MOD=7;
const int total_purple_MOD=8;
const int total_red_MOD=6;
const int total_orange_MOD=9;
const int total_pink_MOD=10;
// const int total_purple_MOD=13;
const int before_color_MOD=11;
// const int recent_color_MOD=14;

// กำหนดขนาดสำหรับ JSON buffer
const size_t capacity = JSON_OBJECT_SIZE(14);

const char* mqtt_server = "192.168.1.105"; // กำหนดที่อยู่ IP ของ MQTT broker
const int mqtt_port = 1883; // กำหนดพอร์ตของ MQTT broker (โดยทั่วไปเป็น 1883)
const char* mqtt_user = "Admin"; // กำหนดชื่อผู้ใช้ MQTT (ถ้ามี)
const char* mqtt_password = "Admin"; // กำหนดรหัสผ่าน MQTT (ถ้ามี)

WiFiClient espClient;
PubSubClient client(espClient);

void reconnect() {
  // รอการเชื่อมต่อกับ MQTT broker
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // พยายามเชื่อมต่อ
    if (client.connect("ESP32Client")) { // ตั้งชื่อของ MQTT client
      Serial.println("connected");
      // เมื่อเชื่อมต่อสำเร็จ ทำการส่งข้อมูลหรือตั้งค่าต่างๆ ตามต้องการ
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // รอ 5 วินาทีก่อนที่จะลองเชื่อมต่ออีกครั้ง
      delay(5000);
    }
  }
}

void MOD_SENT(){
  mb.task();

   //Attach switchPin to SWITCH_ISTS register
  mb.Hreg(total_all_MOD, total_all);
  mb.Hreg(total_white_MOD, total_white);
  mb.Hreg(total_black_MOD, total_black);
  mb.Hreg(total_brown_MOD, total_brown);
  mb.Hreg(total_yellow_MOD, total_yellow);
  mb.Hreg(total_green_MOD, total_green);
  mb.Hreg(total_blue_MOD, total_blue);
  mb.Hreg(total_red_MOD, total_red);
  mb.Hreg(total_orange_MOD, total_orange);
  mb.Hreg(total_pink_MOD, total_pink);
  mb.Hreg(total_purple_MOD, total_purple);
  mb.Hreg(before_color_MOD, before_color);
  // mb.Hreg(recent_color_MOD, recent_color);
}

void Webbrowser(){
  WiFi.softAP(apSSID, apPassword);

  // แสดง IP ของ Access Point ใน Serial Monitor
  Serial.print("Access Point IP: ");
  Serial.println(WiFi.softAPIP());

  // กำหนด route สำหรับหน้าเว็บในการกรอกข้อมูล WiFi
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><body>";
    html += "<h1>ESP32 WiFi Configuration</h1>";
    html += "<form action='/connect' method='post'>";
    html += "<label for='ssid'>SSID:</label>";
    html += "<input type='text' id='ssid' name='ssid' required><br>";
    html += "<label for='password'>Password:</label>";
    html += "<input type='password' id='password' name='password' required><br>";
    html += "<input type='submit' value='Connect'>";
    html += "</form></body></html>";
    request->send(200, "text/html", html);
  });

  // กำหนด route สำหรับการเชื่อมต่อ WiFi
  server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request){
    // รับค่าที่กรอกจากฟอร์ม
    ssid = request->arg("ssid");
    password = request->arg("password");

    // เชื่อมต่อ WiFi
    WiFi.begin(ssid.c_str(), password.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("กำลังเชื่อมต่อ WiFi...");
    }
    Wifi_status=1;
    // แสดง IP address หลังจากเชื่อมต่อ WiFi สำเร็จ
    String response = "<html><body>";
    response += "<h1>WiFi Connected!</h1>";
    response += "<p>IP Address: " + WiFi.localIP().toString() + "</p>";
    // เพิ่มค่าเพิ่มเติม 12 ค่า
    response += "<p>total_all : " + String(total_all) + "</p>";
    response += "<p>recent_color : " + String(before_color) + "</p>";
    response += "<p>total_white : " + String(total_white) + "</p>";
    response += "<p>total_black : " + String(total_black) + "</p>";
    response += "<p>total_brown : " + String(total_brown) + "</p>";
    response += "<p>total_yellow : " + String(total_yellow) + "</p>";
    response += "<p>total_green : " + String(total_green) + "</p>";
    response += "<p>total_blue : " + String(total_blue) + "</p>";
    response += "<p>total_red : " + String(total_red) + "</p>";
    response += "<p>total_orange : " + String(total_orange) + "</p>";
    response += "<p>total_pink : " + String(total_pink) + "</p>";
    response += "<p>total_waste : " + String(total_waste) + "</p>";
    response += "<p>total_purple : " + String(total_purple) + "</p>";
    response += "</body></html>";
    request->send(200, "text/html", response);
  });

  // เริ่มต้น server
  server.begin();
}

void start_Sensor_color(){
  pinMode(S2, OUTPUT);        /*Define S2 Pin as a OUTPUT*/
  pinMode(S3, OUTPUT);        /*Define S3 Pin as a OUTPUT*/
  pinMode(sensorOut, INPUT);
  Serial.println("Starting....");
}


int getRed() {
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  Frequency = pulseIn(sensorOut, LOW); /*Get the Red Color Frequency*/
  // Serial.print("FR = ");
  // Serial.println(Frequency);
  return Frequency;
}

int getGreen() {
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  Frequency = pulseIn(sensorOut, LOW); /*Get the Green Color Frequency*/
  // Serial.print("FG = ");
  // Serial.println(Frequency);
  return Frequency;
}

int getBlue() {
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  Frequency = pulseIn(sensorOut, LOW); /*Get the Blue Color Frequency*/
  // Serial.print("FB = ");
  // Serial.println(Frequency);
  return Frequency;
}

int getClear() {
  digitalWrite(S2,HIGH);
  digitalWrite(S3,LOW);
  Frequency = pulseIn(sensorOut, LOW); /*Get the Red Color Frequency*/
  // Serial.print("FC = ");
  // Serial.println(Frequency);
  // Serial.println(" ");
  return Frequency;
}

void Sensor_Out(){
  // Red = getRed();
  // redValue = map(Red, R_Min,R_Max,255,0); /*Map the Red Color Value*/
  // // delay(200);
 
  // Green = getGreen();
  // greenValue = map(Green, G_Min,G_Max,255,0); /*Map the Green Color Value*/
  // // delay(200);
 
  // Blue = getBlue();
  // blueValue = map(Blue, B_Min,B_Max,255,0);   /*Map the Blue Color Value*/
  // // delay(200);
  
  // Clear = getClear();
  // clearValue = map(Clear, C_Min,C_Max,255,0);   /*Map the Blue Color Value*/
  // // delay(200);


  // rgb_Value[0] = redValue;
  // rgb_Value[1] = greenValue;
  // rgb_Value[2] = blueValue;
  // rgb_Value[3] = clearValue;

  rgb_Value[0] = getRed();
  rgb_Value[1] = getGreen();
  rgb_Value[2] = getBlue();
  rgb_Value[3] = getClear();
  color_num = clft.predict(rgb_Value);
  // Serial.println(ClearT);
  // Serial.print("[");
  // Serial.print(rgb_Value[0]);
  // Serial.print(",");
  // Serial.print(rgb_Value[1]);
  // Serial.print(",");
  // Serial.print(rgb_Value[2]);
  // Serial.print(",");
  // Serial.print(rgb_Value[3]);
  // Serial.println("],");
  // Serial.print("color : ");
  // Serial.println(color_num);
  
  if(color_num == 1){
    // Serial.print("color : ");
    // Serial.println("White");
    c_white++;
  }
  else if(color_num == 2){
    // Serial.print("color : ");
    // Serial.println("Black");
    c_black++;
  }
  else if(color_num == 3){
    // Serial.print("color : ");
    // Serial.println("Brown");
    c_brown++;
  }
  else if(color_num == 4){
    // Serial.print("color : ");
    // Serial.println("Yellow");
    c_yellow++;
  }
  else if(color_num == 5){
    // Serial.print("color : ");
    // Serial.println("Green");
    c_green++;
  }
  else if(color_num == 6){
    // Serial.print("color : ");
    // Serial.println("Red");
    c_red++;
  }
  else if(color_num == 7){
    // Serial.print("color : ");
    // Serial.println("Blue");
    c_blue++;
  }
  else if(color_num == 8){
    // Serial.print("color : ");
    // Serial.println("Purple");
    c_purple++;
  }
  else if(color_num == 9){
    // Serial.print("color : ");
    // Serial.println("Orange");
    c_orange++;
  }
  else if(color_num == 10){
    // Serial.print("color : ");
    // Serial.println("Pink");
    c_pink++;
  }

  else if (color_num == 0){
    //   // Serial.print("color : ");
    // Serial.println("Unkown");
    c_white = 0;
    c_black = 0;
    c_brown = 0;
    c_yellow = 0;
    c_green = 0;
    c_blue = 0;
    c_red = 0;
    c_orange = 0;
    c_pink = 0;
    c_waste = 0;
    c_purple = 0;
  }
    // recent_color=0;
    // Serial.println(c_white);
  // Serial.print("Find: ");
  // Serial.println(Find);
  // delay(1000);
}

void MQTT_PUB(){

    // สร้าง JSON buffer
  DynamicJsonDocument doc(capacity);

  // เพิ่มข้อมูลลงใน JSON object
  doc["total_all"] = total_all;
  doc["total_white"] = total_white;
  doc["total_black"] = total_black;
  doc["total_brown"] = total_brown;
  doc["total_yellow"] = total_yellow;
  doc["total_green"] = total_green;
  doc["total_blue"] = total_blue;
  doc["total_red"] = total_red;
  doc["total_orange"] = total_orange;
  doc["total_pink"] = total_pink;
  doc["total_waste"] = total_waste;
  doc["total_purple"] = total_purple;
  doc["recent_color"] = before_color;

  // สร้างสตริง JSON จาก JSON object
  String json_string;
  serializeJson(doc, json_string);

  // ส่งข้อมูลผ่าน MQTT
  client.publish("StatusSen", json_string.c_str());

}

void tick_object(){
  if(c_white == 90){
    Serial.print("color : ");
    Serial.println("White");
    // Serial.println(c_white);
    c_black = 0;
    total_all++;
    total_white++;
    recent_color=1;
    before_color=recent_color;
    Find=1;
     // แปลงค่าใหม่เป็นสตริง
    string_total_all = String(total_all);
    string_total_white = String(total_white);
    string_before_color = String(before_color);
    // ส่งค่าผ่าน MQTT
    MQTT_PUB();
    Serial.print("total : ");
    Serial.println(total_all);
  }
  else if(c_black == 80){
    Serial.print("color : ");
    Serial.println("Black");
    // Serial.println(c_black);
    total_all++;
    total_black++;
    recent_color=2;
    before_color=recent_color;
    Find=1;
     // แปลงค่าใหม่เป็นสตริง
    string_total_all = String(total_all);
    string_total_black= String(total_black);
    string_before_color = String(before_color);
    // ส่งค่าผ่าน MQTT
    MQTT_PUB();
    Serial.print("total : ");
    Serial.println(total_all);
  }
  else if(c_brown == 120){
    Serial.print("color : ");
    Serial.println("Brown");
    // Serial.println(c_brown);
    total_all++;
    total_brown++;
    recent_color=3;
    before_color=recent_color;
    Find=1;
     // แปลงค่าใหม่เป็นสตริง
    string_total_all = String(total_all);
    string_total_brown = String(total_brown);
    string_before_color = String(before_color);
    // ส่งค่าผ่าน MQTT
    MQTT_PUB();
    Serial.print("total : ");
    Serial.println(total_all);
  }
  else if(c_yellow == 180){
    Serial.print("color : ");
    Serial.println("Yellow");
    // Serial.println(c_yellow);
    total_all++;
    total_yellow++;
    recent_color=4;
    before_color=recent_color;
    Find=1;
     // แปลงค่าใหม่เป็นสตริง
    string_total_all = String(total_all);
    string_total_yellow = String(total_yellow);
    string_before_color = String(before_color);
    // ส่งค่าผ่าน MQTT
    MQTT_PUB();
    Serial.print("total : ");
    Serial.println(total_all);
  }
  else if(c_green == 130){
    Serial.print("color : ");
    Serial.println("Green");
    // Serial.println(c_green);
    total_all++;
    total_green++;
    recent_color=5;
    before_color=recent_color;
    Find=1;
     // แปลงค่าใหม่เป็นสตริง
    string_total_all = String(total_all);
    string_total_green = String(total_green);
    string_before_color = String(before_color);
    // ส่งค่าผ่าน MQTT
    MQTT_PUB();
    Serial.print("total : ");
    Serial.println(total_all);
  }
  else if(c_red ==130){
    Serial.print("color : ");
    Serial.println("Red");  
    // Serial.println(c_red);
    total_all++;
    total_red++;
    recent_color=6;
    before_color=recent_color;
    Find=1;
     // แปลงค่าใหม่เป็นสตริง
    string_total_all = String(total_all);
    string_total_red = String(total_red);
    string_before_color = String(before_color);
    // ส่งค่าผ่าน MQTT
    MQTT_PUB();
    Serial.print("total : ");
    Serial.println(total_all);
  }
  else if(c_blue == 80){
    Serial.print("color : ");
    Serial.println("Blue");
    // Serial.println(c_blue);
    total_all++;
    total_blue++;
    recent_color=7;
    before_color=recent_color;
    Find=1;
     // แปลงค่าใหม่เป็นสตริง
    string_total_all = String(total_all);
    string_total_blue = String(total_blue);
    string_before_color = String(before_color);
    // ส่งค่าผ่าน MQTT
    MQTT_PUB();
    Serial.print("total : ");
    Serial.println(total_all);
  }
  else if(c_purple == 120){
    Serial.print("color : ");
    Serial.println("Purple");
    // Serial.println(c_purple);
    total_all++;
    total_purple++;
    recent_color=8;
    before_color=recent_color;
    Find=1;
     // แปลงค่าใหม่เป็นสตริง
    string_total_all = String(total_all);
    string_total_purple = String(total_purple);
    string_before_color = String(before_color);
    // ส่งค่าผ่าน MQTT
    MQTT_PUB();
    Serial.print("total : ");
    Serial.println(total_all);
  }
  else if(c_orange == 140){
    Serial.print("color : ");
    Serial.println("Orange");
    // Serial.println(c_orange);
    total_all++;
    total_orange++;
    recent_color=9;
    before_color=recent_color;
    Find=1;
     // แปลงค่าใหม่เป็นสตริง
    string_total_all = String(total_all);
    string_total_orange = String(total_orange);
    string_before_color = String(before_color);
    // ส่งค่าผ่าน MQTT
    MQTT_PUB();
    Serial.print("total : ");
    Serial.println(total_all);
  }
  else if(c_pink == 180){
    Serial.print("color : ");
    Serial.println("Pink");
    // Serial.println(c_pink);
    total_all++;
    total_pink++;
    recent_color=10;
    before_color=recent_color;
    Find=1;
     // แปลงค่าใหม่เป็นสตริง
    string_total_all = String(total_all);
    string_total_pink = String(total_pink);
    string_before_color = String(before_color);
    // ส่งค่าผ่าน MQTT
    MQTT_PUB();
    Serial.print("total : ");
    Serial.println(total_all);
  }
}


void setup() {
  Serial.begin(115200);/*Set the baudrate to 115200*/
  start_Sensor_color();
  Webbrowser();

  // while (!Serial);

  //   Serial.println("กำลังเชื่อมต่อ WiFi...");
  //   WiFi.begin(ssid, password);

  //   while (WiFi.status() != WL_CONNECTED) {
  //     delay(500);
  //     Serial.print(".");
  //   }

  //   Serial.println("");
  //   Serial.println("เชื่อมต่อ WiFi เรียบร้อย");
  //   Serial.print("IP address: ");
  //   Serial.println(WiFi.localIP());

  // เชื่อมต่อ MQTT
  client.setServer(mqtt_server, mqtt_port);
  mb.server();
  mb.addCoil(0, false, 10);
  mb.addIsts(0, false, 10);
  mb.addHreg(0, 0, 50);
  mb.addIreg(0, 0, 50);
}

void loop() {
  unsigned long currentMillis = millis();
  Sensor_Out();
  color_num=0;
  if(Find==0){
    tick_object();
  }
  else if(Find==1){
    if (currentMillis - previousMillis >= interval) {
    // Save the last time you blinked the LED
    previousMillis = currentMillis;
    Find = 0;
    // Serial.println("sura");
    }
  }

  MOD_SENT();
  MQTT_PUB();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // delay(1000);
  // Serial.print("c_color:");
  // Serial.println(c_purple);
}
