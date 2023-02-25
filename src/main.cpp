// Определяем точкy доступа
#define WIFI_ACCES_POINT esp32              // имя и пароль создаваемой wifi сети
#define WIFI_PASSWIRD 1234567812345678
#define WIFI_IS_HIDDEN 1                        // 1 -скрытая точка, 0 - видимая
#define CHANNEL 1                          // канал 1..11
#define MAX_CONNECTION 4                   // макимальное количество подключений 1..8
//#define TELEGRAM

// Подключем библиотеки
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <GyverPortal.h>
GyverPortal ui;

// Создаём объект UDP cоединения
#include <AsyncUDP.h>
AsyncUDP udp;
const uint16_t port = 12345; // Определяем порт


#define ESP32_WIFIAP // закомментировать если нужно  подключаться к существующей wifi
#ifdef ESP32_WIFIAP
bool wifiHidden = WIFI_IS_HIDDEN;
byte maxConnection = MAX_CONNECTION;
bool wifiChanel = CHANNEL;
const char* ssidAP = "esp32";
// const char* ssidAP = "esp32";
const char* passwordAP = "1234567812345678"; // пароль от 1 до 8 два раза
#else
// если не точка, то wifi client 
const char* ssid = "NGOK";
const char* password = "newgeneration";
#endif


#define LED_PIN 2 // esp32 dev module LED_BUILTIN
bool ledState = LOW;
uint8_t state  = 0; // автомат состояний 
bool sensor = 0;
uint32_t ms = 0, prevMs = 0, stateMs = 0, sec = 1;
GPtime upd_UpTime;
uint8_t uptimeHour = 0, uptimeMin = 0, uptimeSec = 0;


// Мигаем, если данные пришли
void ledBlink() {
  digitalWrite(LED_PIN, 1);
  delay(40);
  digitalWrite(LED_PIN, 0);
  delay(40);
}


// страницу web портала строим
void webPageBuild() {
  GP.BUILD_BEGIN();
  GP.THEME(GP_DARK);

  GP.UPDATE("label1,label2,hh,mm,ss");// какие поля нужно обновлять

  // обновление случайным числом
  GP.TITLE("ESP32 photo catch");
  GP.HR();

  GP.LABEL("Посл раз:");
  GP.LABEL("NAN", "label1");
  GP.LABEL("сек назад");
  GP.BREAK();
  GP.LABEL("Пропущено пакетов:");
  GP.LABEL("NAN", "label2");
  GP.BREAK();
  GP.LABEL("Аптайм:");
  GP.LABEL("hh", "hh");
  GP.LABEL("ч");
  GP.LABEL("mm", "mm");
  GP.LABEL("м");
  GP.LABEL("ss", "ss");
  GP.BREAK();

  /* examples

    // создаём блок вручную
    GP.BLOCK_TAB_BEGIN("MOTOR CONFIG");
    M_BOX(GP.LABEL("Velocity"); GP.SLIDER("vel"););
    M_BOX(GP.LABEL("Accel."); GP.SLIDER("acc"););
    M_BOX(GP.BUTTON("bkw", "◄"); GP.BUTTON("frw", "►"););
    GP.BLOCK_END();

    GP.HR();
    GP.HR();

    GP.BLOCK_BEGIN(GP_THIN, "", "My thin txt red", GP_RED);
    GP.LABEL("Block thin text red");
    GP.BOX_BEGIN(GP_JUSTIFY);
    GP.LABEL("Slider");
    GP.SLIDER("sld");

    GP.BOX_BEGIN(GP_JUSTIFY);
    GP.LABEL("Buttons");
    GP.BOX_BEGIN(GP_RIGHT);
    GP.BUTTON_MINI("b1", "Kek", "", GP_RED);
    GP.BUTTON_MINI("b1", "Puk");
    GP.BOX_END();
    GP.BOX_END();

    GP.HR();
    GP.HR();

    M_BLOCK(
      M_BOX(GP.LABEL("Some check 1"); GP.CHECK(""); );
    M_BOX(GP.LABEL("Some Switch 1"); GP.SWITCH(""); );
    M_BOX(GP.LABEL("SSID");     GP.TEXT(""); );
    M_BOX(GP.LABEL("passwordAP"); GP.TEXT(""); );
    M_BOX(GP.LABEL("Host");     GP.TEXT(""); );
    );

     */
  GP.BUILD_END();
} // webPageBuild()


// обрабатываем действия на гайвер портале
void webPageAction() {

  if (ui.update()) {
    // ui.updateTime("time", upd_UpTime);
    ui.updateInt("label1", sec);
    ui.updateInt("label2", ms);
    ui.updateInt("hh", uptimeHour);
    ui.updateInt("mm", uptimeMin);
    ui.updateInt("ss", uptimeSec);
  }
}//webPageAction()


// инициализируем гайвер портал
void webUI_Init() {
  ui.attachBuild(webPageBuild);
  ui.attach(webPageAction);
  ui.start();

}//webUI_Init()


void wifiInit() {

#ifdef ESP32_WIFIAP
  // Инициируем точку доступа WiFi

  WiFi.softAP(ssidAP, passwordAP, wifiChanel, wifiHidden, maxConnection);
  // IPAddress myIP = WiFi.softAPIP();
  Serial.print("wifi_AP IP: ");
  Serial.println(WiFi.softAPIP());
#else 
  // Подключаемся к Wi-Fi
  Serial.print("try conn to ");
  Serial.print(ssid);
  Serial.print(":");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connected. \nIP: ");
  }
  // Выводим IP ESP32
  Serial.println(WiFi.localIP());
#endif
}//wifiInit()




//распарсивание udp пакета 
void parseUdpMessage() {
  if (udp.listen(port)) {
    udp.onPacket([](AsyncUDPPacket packet) {
      /*
      // хороший сайт для работы с библиотекой AsyncUdp
      // https://community.appinventor.mit.edu/t/esp32-with-udp-send-receive-text-chat-mobile-mobile-udp-testing-extension-udp-by-ullis-ulrich-bien/72664/2

      // пример парсинга единственного первого байта данных
      String state;                       // Объект для хранения состояния светодиода в строковом формате
      const uint8_t* msg = packet.data(); // Записываем адрес начала данных в памяти
      const size_t len = packet.length(); // Записываем размер данных

      // Если адрес данных не равен нулю и размер данных больше нуля...
      if (msg != NULL && len > 0) {
        if (msg[0] != 0) {                    // Если первый байт данных содержит 0x1
          state = "Включён"; // записываем строку в объект String
          ledBlink();
        } else if (msg[0] == 0) {
          state = "OFF";
        }
        // Отправляем Обратно данные клиенту
        //packet.printf("Светодиод %s", state.c_str());
      }
      */
#ifdef UDPDEBUG
      Serial.print("UDP Packet Type: ");
      Serial.print(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");
      Serial.print(", From: ");
      Serial.print(packet.remoteIP());
      Serial.print(":");
      Serial.print(packet.remotePort());
      Serial.print(", To: ");
      Serial.print(packet.localIP());
      Serial.print(":");
      Serial.print(packet.localPort());
      Serial.print(", Length: ");
      Serial.print(packet.length());
      Serial.print(", Data: ");
      Serial.write(packet.data(), packet.length());
      Serial.println();
#endif
      /*
      // мощное преобразование входящего сообщения в String
      //взято отсюда https://stackoverflow.com/questions/58253174/saving-packet-data-to-string-with-asyncudp-on-esp32
      //String  udpMessage = (char*)(packet.data());
      */
      char* tmpStr = (char*)malloc(packet.length() + 1);
      memcpy(tmpStr, packet.data(), packet.length());
      tmpStr[packet.length()] = '\0'; // ensure null termination
      String udpMessage = String(tmpStr);
      free(tmpStr); // we can clean it now
#ifdef UDPDEBUG
      Serial.print("income udp String: ");
      Serial.print(udpMessage);
#endif
      if (udpMessage == "on") {
        //digitalWrite(2, HIGH);
        //packet.print("Received: on");
      }
      if (udpMessage == "1") {
        //Serial.println("got sensor");
        sensor = 1; // флаг 
      }
      //reply to the client
      //packet.printf("Got %u bytes of data", packet.length());
      });
  }//udp.listen()
}//parseUdpMessage()


void makePhoto(){
/* 
  #define FOCUS1 10
  #define FOCUS2 10
  #define SHOOTER 10 
  uint16_t shooterTime = 200;
  uint16_t afterSensorTime = 200;
  
  if(afterSensorTime>50) afterSensorTime -= 50;
  delay(afterSensorTime);
  digitalWrite(FOCUS1, 1);
  digitalWrite(FOCUS1, 1);
  delay(50);
  digitalWrite(SHOOTER, 1);
  delay(shooterTime);
  digitalWrite(FOCUS1, 0);
  digitalWrite(FOCUS1, 0);
  digitalWrite(SHOOTER, 0);
 */  
}// makePhoto()


void setup() {
  // Инициируем последовательный порт
  Serial.begin(115200);
  Serial.println("\n\nbulletShot_reciever\n\n");
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, ledState);

  wifiInit();
#ifdef TELEGRAM
  tgBot_Init();
#endif
  parseUdpMessage(); // слушаем входящие по udp 

  webUI_Init();

} // setup

void loop() {
  ui.tick();
  ms = millis();
  // digitalWrite(LED_PIN, ledState);
  parseUdpMessage();// ф-я вернет 1, если пришло по udp  значение "1"

  //автомат:  ловим единичку, фоткаем, отдыхаем 3 сек
  switch (state) {
  case 0:
    state = 1;
    break;
  case 1:
    if (sensor){
      makePhoto(); // фотографируем 
      digitalWrite(LED_PIN, 1);
      Serial.printf("\t%lu:%lu .%lu \t sensor => shoot\n", uptimeHour, uptimeMin, uptimeSec);
      stateMs = ms;
      state = 5;
    }
    break;
  case 5:
    // пауза в 3 секунды, чтобы игнорировать повторные сообщения
    if ((ms - stateMs) > 3000ul) {
      stateMs = ms;
      digitalWrite(LED_PIN, 0);
      sensor = 0; // обнуляем 
      state = 1; // GO на исходную
    }//ms 3000
    break;
  }//switch(state)

  // инкрементируем Аптайм
  if ((ms - prevMs) > 1000) {
    prevMs = ms;
    sec++;
    uptimeSec++;
    if (uptimeSec > 59) {
      uptimeSec = 0;
      uptimeMin++;
      if (uptimeMin > 59) {
        uptimeMin = 0;
        uptimeHour++;
        Serial.printf("%lu hours %lu mins\n", uptimeHour, uptimeMin);
      }
    }  //if sec
  }//ms 1000

} // loop()


/*
 // example of sending udp data
char buf[50];
unsigned long testID = 1716526225;
sprintf(buf, "%lu", testID);
Serial.println(buf);
udp.broadcast(buf);
 */