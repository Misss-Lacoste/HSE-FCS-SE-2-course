/*Как работает модуль для измерения расстояния простыми словами:

Мы посылаем короткий импульс (например, 10 микросекунд) на TRIG
Датчик посылает ультразвук и ждёт, пока он отразится от объекта
Как только возвращается эхо, на ECHO появляется сигнал HIGH
Чем дольше он держится, тем дальше объект
Мы измеряем время, а затем переводим его в расстояние*/

#define PIN_TRIG 16
#define PIN_ECHO 4
#define PIN_BUZZER 13

void setup() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
}

void loop() {
  // Запускаем измерение:
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  // Считываем результат:
  int duration = pulseIn(PIN_ECHO, HIGH);
  int distanceCM = duration / 58;

  if (distanceCM <= 100) {
    int frequency;
    if (distanceCM > 75) frequency = 400;
    else if (distanceCM > 50) frequency = 600;
    else if (distanceCM > 25) frequency = 800;
    else frequency = 1000;
    tone(PIN_BUZZER, frequency);
  } else {
    noTone(PIN_BUZZER);
  }

  delay(200);
}