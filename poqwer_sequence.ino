#include <HardwareSerial.h>
#include <map>
// 定義各種功能的狀態
#define OFF 0
#define ON 1

// 設定各個腳位
#define RED_LED_PIN 15
#define ORANGE_LED_PIN 2
#define GREEN_LED_PIN 4
#define BUZZER_PIN 13
//#define EMO_RELAY_PIN 12
#define AC_MAIN_FRAME 12
#define AC_MOTOR 14
#define AC_XY_TABLE 27
#define AC_WAFER_ROBOT 33

// 定義命令標識
#define STR_SET "SET"
#define STR_GET "GET"

#define SET_TOWER_CMD "TOWER"
#define SET_EMO_CMD "EMO"
#define SET_POWER_CMD "POWER"


#define STR_RED "R"
#define STR_ORANGE "O"
#define STR_GREEN "G"
#define STR_BUZZER "BUZZER"

#define STR_AC_MAIN_FRAME "AC_MAIN_FRAME"
#define STR_AC_MOTOR "AC_MOTOR"
#define STR_AC_XY_TABLE "AC_XY_TABLE"
#define STR_AC_WAFER_ROBOT "AC_WAFER_ROBOT"



#define BUFFER_SIZE 64 // 設置字元緩衝區大小
char buffer[BUFFER_SIZE]; // 定義字元緩衝區
int bufferIndex  = 0; // 字元緩衝區的索引

HardwareSerial SerialPort(2); // use UART2

uint8_t gbuf[4]= {0xAF, 0xFA, 0xAF, 0xFA};

uint8_t readbuffer[64]; // 创建一个大小为 64 的字符数组用于存储读取的数据

unsigned long previousMillis = 0;  // 上次任务执行的时间
const long interval = 1000;        // 间隔时间，单位毫秒


const int MAX_SIZE = 4;
String parsedStrings[MAX_SIZE]= { "", "", "", "" };

static std::map<String, int> listTower = {
    {STR_RED, RED_LED_PIN},
    {STR_ORANGE, ORANGE_LED_PIN},
    {STR_GREEN, GREEN_LED_PIN},
    {STR_BUZZER, BUZZER_PIN},
};

static std::map<String, int> listPower = {
    {STR_AC_MAIN_FRAME, AC_MAIN_FRAME},
    {STR_AC_MOTOR, AC_MOTOR},
    {STR_AC_XY_TABLE, AC_XY_TABLE},
    {STR_AC_WAFER_ROBOT, AC_WAFER_ROBOT},
};

void log(HardwareSerial &sp, const char *format, ...) {
  char buffer[256]; // 假設最大字串長度不超過 256 字元
  va_list args;
  va_start(args, format);
  
  snprintf(buffer, sizeof(buffer), "[Log]");
  int len = strlen(buffer);
  vsnprintf(buffer + len, sizeof(buffer) - len, format, args);
  
  va_end(args);
  sp.println(buffer);
}
void response(HardwareSerial &sp,String type, String command,String arg1, int ret, int data=0) {
  char buffer[256]; // 假設最大字串長度不超過 256 字元

  snprintf(buffer, sizeof(buffer), "[Response]%s,%s,%s,%d,%d",type.c_str(), command.c_str(),arg1.c_str(), ret, data);

  sp.println(buffer);
}





void setup() {
  // 初始化串口通訊
  Serial.begin(9600);
  
  SerialPort.begin(9600, SERIAL_8N1, 16, 17); 
  
  
  // 設置輸出腳位
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(ORANGE_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  //pinMode(EMO_RELAY_PIN, INPUT);
  pinMode(AC_MAIN_FRAME, OUTPUT);
  pinMode(AC_MOTOR, OUTPUT);
  pinMode(AC_XY_TABLE, OUTPUT);
  pinMode(AC_WAFER_ROBOT, OUTPUT);
   
}

void loop() {
  
  if (Serial.available() > 0) {
    char receivedChar = Serial.read(); // 讀取一個字元
    buffer[bufferIndex++] = receivedChar; // 將字元存入字元緩衝區
    if (receivedChar == '\n') { // 如果收到換行符，表示命令接收完畢
      buffer[bufferIndex] = '\0'; // 在命令結尾添加空字符，以便轉換為字串
      processCommand(buffer); // 處理命令
      bufferIndex = 0; // 重置字元緩衝區索引
    }
    if (bufferIndex >= BUFFER_SIZE) { // 如果字元緩衝區已滿，則清空緩衝區
      bufferIndex = 0;
    }
  }  


//  if (SerialPort.available() > 0)
//     {
//       //Serial.println("111");
//       Serial.println(SerialPort.read());
      
//     }        

  // //檢查EMO狀態
  // unsigned long currentMillis = millis();    
  // if (currentMillis - previousMillis >= interval) {
    
  //   previousMillis = currentMillis;    
  //   check_loopback();
  // }
  
}


void clearSerialBuffer() {
  while (SerialPort.available() > 0) {
    char c = SerialPort.read(); // 读取一个字节但不做任何处理
  }
  SerialPort.flush();
}
bool compareBuffers(uint8_t buffer1[], uint8_t buffer2[], int length) {
  for (int i = 0; i < length; ++i) {
    if (buffer1[i] != buffer2[i]) {
      return false; // 如果发现任何一个元素不相等，立即返回 false
    }
  }
  return true; // 如果所有元素都相等，则返回 true
}

void check_loopback() {
  
  clearSerialBuffer();

  SerialPort.write(gbuf, sizeof(gbuf));
    
  delay(10);

  readSerialPort();  
}
void readSerialPort() {
  
  int bytesRead = 0; // 记录已经读取的字节数
  
  // 等待串口接收到 64 个字节数据
  while (bytesRead < 64) {
    // 如果串口有数据可用
    // if (SerialPort.available() <= 0)
    // {
    //   Serial.println("COMPORT is not ready");
    //   delay(200);    
    // }        
    readbuffer[bytesRead] = SerialPort.read();      
    
    bytesRead++;  
  
  }

  // Serial.print("Read size=");
  // Serial.println(bytesRead);
  // // 打印读取的数据
  // for (int i = 0; i < bytesRead; i++) {
  //   Serial.print(readbuffer[i],HEX);
  // }

  bool res = compareBuffers(gbuf,readbuffer,4);
  setTower(STR_RED, res == 1?0:1);

}

void processCommand(char* str) {
  // 在這裡解析並處理接收到的命令

  // 去除空白字符
  for (int i = 0; str[i]; i++) {
    if (isspace(str[i])) {
      str[i] = '\0';
    }
  }
  log(Serial, "type=%s",str );
  String type, command, argment1="xx1", argment2="xx2";


  int index = 0;
  char *token = strtok(str, ",");
  while (token != NULL && index < MAX_SIZE) {
    parsedStrings[index] = String(token);
    index++;
    token = strtok(NULL, ",");
  }

  if(index >= 0) type = String(parsedStrings[0]);
  if(index >= 1) command = String(parsedStrings[1]);
  if(index >= 2) argment1 = String(parsedStrings[2]);
  if(index >= 3) argment2 = String(parsedStrings[3]);

  for (int i = 0; i < index; i++) {
    parsedStrings[i] = "";
  }

  log(Serial, "type=%s,Cmd=%s,arg1=%s,arg2=%s",type,command,argment1,argment2 );

  int ret =-1;
  if(STR_SET == type)
  {

    int state = argment2.toInt();

    //0 is pass
    //-1: is fail

  // 解析命令並執行相應的功能
    if (command.equals(SET_TOWER_CMD)) {
      String color =  argment1;        
      ret = setTower(color, state);
      log(Serial, "Command=%s,Set_Tower_pin=%s,state=%d,ret=%d",command.c_str(), color.c_str(),state,ret);
      
      response(Serial,type,command, argment1,ret);

    } 
    else if (command.equals(SET_EMO_CMD)) {

      ret = setEmo(state);
      log(Serial, "Set_EMO_state=%d,ret=",state,ret);    
      response(Serial,type,command, argment1,ret);

    } 
    else if (command.equals(SET_POWER_CMD)) {
      String psNumber = argment1;
      ret = setPowerSequence(psNumber, state);        
      log(Serial, "Set_Power_item=%s,state=%d,ret=%d", psNumber.c_str(), state, ret);

      response(Serial,type,command, argment1,ret);
    }
    else
    {
      log(Serial,"Error!Invalid command.");
      response(Serial,type,command, argment1,ret);        
    }
  }
  else if(STR_GET == type)
  {
    //GET,POWER,AC_MAIN_FRAME    

    int data = 0;
    ret = queryStatus(command,argment1,data);
    log(Serial, "Command=%s,argment1=%s,data=%d,ret=%d", command.c_str(), argment1.c_str(),data,ret);

    response(Serial,type,command, argment1,ret,data);
    

  }
  else
  {
    log(Serial,"Error!Invalid type.");
    response(Serial,type,command, argment1,ret);    
  }
    
  
}

  
// 設定 TOWER 腳位
int setTower(String color, int state) {
  
    auto it = listTower.find(color);
    // 如果找到了對應的值，返回該值；否則返回 -1 或其他適當的錯誤碼
    if (it != listTower.end()) {
        // 找到了，返回對應的 pingname
        digitalWrite(it->second, state);
        
    } else {
        // 沒有找到，返回錯誤碼
        return -1; // 或者其他適當的錯誤碼
    } 

    return 0; 
    
}

// 設定 EMO 迴路開關
int setEmo(int state) {
  //digitalWrite(EMO_RELAY_PIN, state);
  if(state == 1)
  {
    setTower(STR_RED, 1);
    setTower(STR_BUZZER, 1);
  }
  else
  {
    setTower(STR_RED, 0);
    setTower(STR_BUZZER, 0);
  }
  return 0;
}

// 設定 POWER SEQUENCE
int setPowerSequence(String psNumber, int state) {

    auto it = listPower.find(psNumber);
    // 如果找到了對應的值，返回該值；否則返回 -1 或其他適當的錯誤碼
    if (it != listTower.end()) {
        // 找到了，返回對應的 pingname
        digitalWrite(it->second, state);
        
    } else {
        // 沒有找到，返回錯誤碼
        return -1; // 或者其他適當的錯誤碼
    } 
    return 0;


  return 0;
}

// 查詢狀態
int queryStatus(String queryItem, String pingname, int& data) {

  if (queryItem.equals("TOWER")) {

    auto it = listTower.find(pingname);
    // 如果找到了對應的值，返回該值；否則返回 -1 或其他適當的錯誤碼
    if (it != listTower.end()) {
        // 找到了，返回對應的 pingname
        data = digitalRead(listTower[pingname]);
        return 0;
    } else {
        // 沒有找到，返回錯誤碼
        return -1; // 或者其他適當的錯誤碼
    } 
        

  } else if (queryItem.equals("EMO")) {
    
    check_loopback();

    return digitalRead(RED_LED_PIN);

  } else if (queryItem.equals("POWER")) {

    auto it = listPower.find(pingname);
    // 如果找到了對應的值，返回該值；否則返回 -1 或其他適當的錯誤碼
    if (it != listTower.end()) {
        // 找到了，返回對應的 pingname
        data =  digitalRead(listPower[pingname]);
        return 0;
        
    } else {
        // 沒有找到，返回錯誤碼
        return -1; // 或者其他適當的錯誤碼
    }     

  } 


  return -1;
}
