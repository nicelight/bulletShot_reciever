
// хранение настроек в EEPROM памяти

#define AP_SSID ""
#define AP_PASS ""

// структура настроек
struct Settings {
  int sld;
  bool sw;
  char str[20];
};

Settings memory;

// используем менеджер памяти для удобства
#include <EEManager.h>
EEManager memory(memory);

#include <GyverPortal.h>
GyverPortal ui;

void setup() {
  startup();
  EEPROM.begin(100);  // выделить память (больше или равно размеру даты)
  memory.begin(0, 'a');

  ui.start();
  ui.attachBuild(build);
  ui.attach(action);
}

void build() {
  GP.BUILD_BEGIN(GP_DARK);
  // выводим на страницу из переменных
  GP.SWITCH("sw", memory.sw);
  GP.BREAK();
  GP.SLIDER("sld", memory.sld);
  GP.BREAK();
  GP.TEXT("txt", "", memory.str);
  GP.BUILD_END();
}

void action() {
  if (ui.click()) {
    // по клику переписать пришедшие данные в переменные
    ui.clickInt("sld", memory.sld);
    ui.clickBool("sw", memory.sw);
    ui.clickStr("txt", memory.str);

    // запланировать обновление настроек в памяти
    memory.update();
  }
}

void loop() {
  ui.tick();
  memory.tick();
}

void startup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(AP_SSID, AP_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());
}