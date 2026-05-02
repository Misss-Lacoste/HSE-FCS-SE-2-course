#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

String inputBuffer = ""; // Буфер для ввода

void displayText(String text)
{
    lcd.clear();
    lcd.setCursor(0, 0);

    // Разбиваем строку на 2 строки по 16 символов
    String line1 = text.substring(0, 16);
    String line2 = text.length() > 16 ? text.substring(16, 32) : "";

    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
}

void setup()
{
    lcd.init();
    lcd.backlight();
    displayText("   I LOVE C++");
}

void loop(){}