#define LED_PIN 14 // Куда подключен светодиод
#define DELAY_TIME 500 // Сколько ждать, в мс (пол-секунды)

void setup() {
  pinMode(LED_PIN, OUTPUT); // Указываем, что мы будем использовать пин как выход
}

void loop() { // Главный цикл 
  digitalWrite(LED_PIN, HIGH);
  delay(DELAY_TIME);
  digitalWrite(LED_PIN, LOW);
  delay(DELAY_TIME);
}