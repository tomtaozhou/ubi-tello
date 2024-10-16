#define LOAD_FONT2
#define LOAD_FONT4
#include <M5StickC.h>
#include <WiFi.h>
#include <WiFiUdp.h>

const char* TELLO_IP = "192.168.10.1";
// const char* TELLO_SSID = "TELLO-CCD85D";const char* TELLO_No = "01";
// const char* TELLO_SSID = "TELLO-CCDA6F";const char* TELLO_No = "02";
// const char* TELLO_SSID = "TELLO-CCDBB1";const char* TELLO_No = "03";
// const char* TELLO_SSID = "TELLO-CCEFC3";const char* TELLO_No = "04";
// const char* TELLO_SSID = "TELLO-CCE724";const char* TELLO_No = "05";
// const char* TELLO_SSID = "TELLO-CCEF77";const char* TELLO_No = "06";

// ドローンに対応する番号の前にある「//」を削除すること。

const int PORT = 8889;

WiFiUDP Udp;
char packetBuffer[255];
String message = "";

float x;
float y;

char msgx[6];
char msgy[6];

float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;
float accX_sum = 0.0F;
float accY_sum = 0.0F;
float accX_diff = 0.0F;
float accY_diff = 0.0F;
int count;

void setup() {
  M5.begin();
  M5.IMU.Init();
  
  // 标题为当前无人机的IP地址
  M5.Lcd.fillRect(0, 0, 80, 20, TFT_RED);  // 背景颜色为红色
  M5.Lcd.drawCentreString(TELLO_No, 40, 2, 1);  // 显示IP地址
  M5.Lcd.drawCentreString("Controller", 40, 10, 1);
  
  // 方向文字背景
  M5.Lcd.fillTriangle(20, 50, 40, 30, 60, 50, TFT_WHITE);
  M5.Lcd.fillTriangle(20, 50, 40, 70, 60, 50, TFT_WHITE);
  
  // 方向文字
  M5.Lcd.setTextColor(TFT_BLACK, TFT_YELLOW);
  M5.Lcd.drawCentreString("F", 40, 20, 4);
  M5.Lcd.drawCentreString("B", 40, 60, 4);
  M5.Lcd.drawCentreString("L", 0, 40, 4);
  M5.Lcd.drawCentreString("R", 70, 40, 4);
  
  // ---X的显示
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.drawCentreString("accX: ", 20, 90, 1);
  sprintf(msgx, "%-2.2f", x);
  M5.Lcd.drawCentreString(msgx, 60, 90, 1);
  
  // ---Y值的显示
  M5.Lcd.drawCentreString("accY: ", 20, 100, 1);
  sprintf(msgy, "%-2.2f", y);
  M5.Lcd.drawCentreString(msgy, 60, 100, 1);

  // 补正值的值获取
  for (count = 1; count <= 10; count++) {
    delay(20);
    M5.IMU.getAccelData(&accX, &accY, &accZ);
  }
  
  for (count = 1; count <= 10; count++) {
    M5.IMU.getAccelData(&accX, &accY, &accZ);
    delay(20);
    accX_sum += accX;
    accY_sum += accY;
  }
  
  // 补正值
  accX_diff = accX_sum / 10;
  accY_diff = accY_sum / 10;

  // 初始化WiFi
  WiFi.begin(TELLO_SSID, "");
  while (WiFi.status() != WL_CONNECTED) {
    print_msg("Now, WiFi Connecting..");
    delay(50);
  }
  
  print_msg("WiFi Connected.");
  Udp.begin(PORT);
  tello_command_exec("command");
  delay(50);

  // 连接后将标题颜色改为绿色
  M5.Lcd.fillRect(0, 0, 80, 20, TFT_GREEN);
  M5.Lcd.drawCentreString(TELLO_No, 40, 2, 1);
  M5.Lcd.drawCentreString("Controller", 40, 10, 1);
}

void loop() {
  M5.IMU.getAccelData(&accX, &accY, &accZ);
  x = accX - accX_diff;
  y = accY - accY_diff;

  // ---X的显示
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.drawCentreString("accX: ", 20, 90, 1);
  sprintf(msgx, "%-2.2f", x);
  M5.Lcd.drawCentreString("        ", 60, 90, 1);
  M5.Lcd.drawCentreString(msgx, 60, 90, 1);
  
  // ---Y值的显示
  M5.Lcd.drawCentreString("accY: ", 20, 100, 1);
  sprintf(msgy, "%-2.2f", y);
  M5.Lcd.drawCentreString("        ", 60, 100, 1);
  M5.Lcd.drawCentreString(msgy, 60, 100, 1);

  // 调用更新方向显示函数
  updateDirectionDisplay(x, y);

  // 单按B键起飞
  if (M5.BtnB.wasPressed()) {
    print_msg("TAKE OFF");
    tello_command_exec("takeoff");
    delay(500);
  }

  // 长按B键降落
  if (M5.BtnB.pressedFor(300)) {
    print_msg("LAND");
    tello_command_exec("land");
    delay(500);
  }

  // 按A键处理
  if (M5.BtnA.wasPressed()) {
    if (fabs(y) > 0.5) {
      if (y > 0) {
        tello_command_exec("up 50");
      } else {
        print_msg("DOWN");
        tello_command_exec("down 50");
      }
    } else {
      print_msg("CW");
      tello_command_exec("cw 45");
    }
  }

 
  //tello移動
  if (fabs(x)> 0.4){
      //左移動
      if (x > 0){
        print_msg("LEFT");
        tello_command_exec("left 30");
      }
      //右移動
      if (x < -0){
        print_msg("RIGHT");
        tello_command_exec("right 30");
      }
  }
  if (fabs(y)> 0.4){
      //後退
      if (y > 0){
        print_msg("BACK");
        tello_command_exec("back 30");
      }
      //前進
      if (y < 0){
        print_msg("FRONT");
        tello_command_exec("forward 30");
      }
  }
  delay(500); 
  M5.update(); 
}


// 更新方向显示的函数
void updateDirectionDisplay(float x, float y) {
  // 恢复默认的黄色背景
  M5.Lcd.setTextColor(TFT_BLACK, TFT_YELLOW);
  M5.Lcd.drawCentreString("F", 40, 20, 4);
  M5.Lcd.drawCentreString("B", 40, 60, 4);
  M5.Lcd.drawCentreString("L", 0, 40, 4);
  M5.Lcd.drawCentreString("R", 70, 40, 4);
  
  // 根据x和y的值动态改变背景颜色
  if (y < -0.3) {
    // 前进方向(F)变为蓝色
    M5.Lcd.setTextColor(TFT_BLACK, TFT_BLUE);
    M5.Lcd.drawCentreString("F", 40, 20, 4);
  } else if (y > 0.3) {
    // 后退方向(B)变为蓝色
    M5.Lcd.setTextColor(TFT_BLACK, TFT_BLUE);
    M5.Lcd.drawCentreString("B", 40, 60, 4);
  }

  if (x < -0.3) {
    // 向右方向(R)变为蓝色
    M5.Lcd.setTextColor(TFT_BLACK, TFT_BLUE);
    M5.Lcd.drawCentreString("R", 70, 40, 4);
  } else if (x > 0.3) {
    // 向左方向(L)变为蓝色
    M5.Lcd.setTextColor(TFT_BLACK, TFT_BLUE);
    M5.Lcd.drawCentreString("L", 0, 40, 4);
  }
}

// 显示消息
void print_msg(String status_msg) {
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.drawString("              ", 2, 140, 1);
  M5.Lcd.drawString(status_msg, 2, 140, 1);
}

// 执行Tello指令
void tello_command_exec(char* tello_command) {
  Udp.beginPacket(TELLO_IP, PORT);
  Udp.printf(tello_command);
  Udp.endPacket();
  message = listenMessage();
  delay(50);
}

// 监听Tello消息
String listenMessage() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    IPAddress remoteIp = Udp.remoteIP();
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
  }
  return (char*) packetBuffer;
}
