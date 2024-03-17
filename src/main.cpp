// Определяем точкy доступа
#define WIFI_ACCES_POINT esp32              // имя и пароль создаваемой wifi сети
#define WIFI_PASSWIRD 1234567812345678
#define WIFI_IS_HIDDEN 1                        // 1 -скрытая точка, 0 - видимая
#define CHANNEL 1                          // канал 1..11
#define MAX_CONNECTION 4                   // макимальное количество подключений 1..8
//#define TELEGRAM

#define PHOTOFLASH1 26 //white 
#define PHOTOFLASH2 25 // green
#define FOCUS 33 // red
#define SHOOTER 32 //black




// Подключем библиотеки
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>

#include <EEPROM.h>
#include <EEManager.h>

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
const char* ssidAP = "esp32-1";
const char* passwordAP = "1234567812345678"; // пароль от 1 до 8 два раза
#else
// если не точка, то wifi client 
const char* ssid = "NGOK";
const char* password = "newgeneration";
#endif


#define LED_PIN 2 // esp32 dev module LED_BUILTIN
bool ledState = LOW;
uint8_t state = 0; // автомат состояний 
bool sensor = 0;
uint32_t ms = 0, prevMs = 0, stateMs = 0, sec = 1;
GPtime upd_UpTime;
uint8_t uptimeHour = 0, uptimeMin = 0, uptimeSec = 0;
uint16_t totalPhotos = 0;
// структура настроек
struct Settings {
  uint16_t shooterTime;
  uint16_t afterSensorTime;
  uint16_t sld;
  uint16_t afterFlashDel;
  char str[20];
};

Settings set; // инициализация структуры типа mem

EEManager memory(set); // инициализация памяти


// Мигаем, если данные пришли
void ledBlink() {
  digitalWrite(LED_PIN, 1);
  delay(40);
  digitalWrite(LED_PIN, 0);
  delay(40);
}//ledBlink()



void makePhoto() { 
  // чтобы изменить безболезненно, перекладываем значение в другую переменную
  uint16_t afterSensorTime = set.afterSensorTime;
  if (afterSensorTime > 50) afterSensorTime -= 50;
  delay(afterSensorTime);
  digitalWrite(LED_PIN, 0); //  тушим на 50 мс чтобы было понятно что фотка пошла
  digitalWrite(FOCUS, 1);
  delay(50); // после фокусировки задержка
  digitalWrite(LED_PIN, 1); // восстанавливаем сигнальный светодиод
  digitalWrite(SHOOTER, 1); // ФОТОГРАФИРУЕМ
  delay(set.afterFlashDel); // задержка вспышки после шутера
  digitalWrite(PHOTOFLASH1, 1);  // сигнал на вспышку
  digitalWrite(PHOTOFLASH2, 1);
  delay(set.shooterTime); // задержка удержания затвора

  digitalWrite(FOCUS, 0);
  digitalWrite(SHOOTER, 0);
  digitalWrite(PHOTOFLASH1, 0);
  digitalWrite(PHOTOFLASH2, 0);
}// makePhoto()



// страницу web портала строим
void webPageBuild() {
  GP.BUILD_BEGIN();
  GP.THEME(GP_DARK);

  GP.UPDATE("label1,label2,hh,mm,ss");// какие поля нужно обновлять

  // обновление случайным числом
  GP.TITLE("ESP32 photo catch");
  GP.HR();
  GP.BUTTON("btn", "Shot");
  GP.LABEL("Всего:");
  GP.LABEL("NAN", "label1");
  GP.LABEL("фоток");
  GP.BREAK();
  GP.LABEL("Сфоткал");
  GP.LABEL("NAN", "label2");
  GP.LABEL("сек назад");
  GP.BREAK();
  GP.LABEL("Аптайм:");
  GP.LABEL("hh", "hh");
  GP.LABEL("ч");
  GP.LABEL("mm", "mm");
  GP.LABEL("м");
  GP.LABEL("ss", "ss");
  GP.BREAK();
  GP.HR();
  //GP.SLIDER("sld", set.sld);
  //GP.BREAK();
  //GP.TEXT("txt", "", set.str);

  GP.LABEL("Задержка датчика");
  GP.NUMBER("uiPhotoGap", "number", set.afterSensorTime); GP.BREAK();
  GP.LABEL(" мс");
  GP.BREAK();
  GP.HR();
  GP.LABEL("Задержка вспышки ");
  GP.NUMBER("uiFlashDel", "number", set.afterFlashDel); GP.BREAK();
  GP.LABEL(" мс");
  GP.BREAK();
  GP.HR();
  GP.LABEL("Удержание затвора");
  GP.NUMBER("uiShooterTime", "number", set.shooterTime); GP.BREAK();
  GP.LABEL("в милисекундах");
  GP.BREAK();
  GP.HR();

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
    ui.updateInt("label1", totalPhotos);
    ui.updateInt("label2", sec);
    ui.updateInt("hh", uptimeHour);
    ui.updateInt("mm", uptimeMin);
    ui.updateInt("ss", uptimeSec);
  }//update()
  if (ui.click()) {
    Serial.println("UI CLICK detected");
    // по нажатию на кнопку сфотографируем
    if (ui.click("btn")) makePhoto();
    // перезапишем в shooterTime что ввели в интерфейсе
    if (ui.clickInt("uiPhotoGap", set.afterSensorTime)) {
      Serial.print("set.afterSensorTime: ");
      Serial.println(set.shooterTime);
      memory.update(); //обновление настроек в EEPROM памяти
    }
    if (ui.clickInt("uiFlashDel", set.afterFlashDel)) {
      Serial.print("set.afterFlashDel: ");
      Serial.println(set.afterFlashDel);
      memory.update(); //обновление настроек в EEPROM памяти
    }
    if (ui.clickInt("uiShooterTime", set.shooterTime)) {
      Serial.print("set.shooterTime: ");
      Serial.println(set.shooterTime);
      memory.update(); //обновление настроек в EEPROM памяти
    }
  }//click()
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


void pinsBegin() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, 0);
  pinMode(PHOTOFLASH1, OUTPUT);
  pinMode(PHOTOFLASH2, OUTPUT);
  pinMode(FOCUS, OUTPUT);
  pinMode(SHOOTER, OUTPUT);
  digitalWrite(PHOTOFLASH1, 0);
  digitalWrite(PHOTOFLASH2, 0);
  digitalWrite(FOCUS, 0);
  digitalWrite(SHOOTER, 0);
}  //pinsBegin()


void setup() {
  // Инициируем последовательный порт
  Serial.begin(115200);
  Serial.println("\n\nbulletShot_reciever\n\n");
  EEPROM.begin(100);  // выделить память (больше или равно размеру даты)
  memory.begin(0, 'a');

  pinsBegin();
  wifiInit();
#ifdef TELEGRAM
  tgBot_Init();
#endif
  parseUdpMessage(); // слушаем входящие по udp 
  webUI_Init();
  makePhoto(); // тестовая фотка 
} // setup

void loop() {
  ui.tick();
  memory.tick();
  ms = millis();
  parseUdpMessage();// ф-я вернет 1, если пришло по udp  значение "1"

  //автомат:  ловим единичку, фоткаем, отдыхаем 3 сек
  switch (state) {
  case 0:
    state = 1;
    break;
  case 1:
    if (sensor) {
      digitalWrite(LED_PIN, 1);
      makePhoto(); // фотографируем 
      totalPhotos++;
      sec = 0; // обнуляем инфо-таймер последней фотки

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
    //Serial.print("state:");
    //Serial.println(state);
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
