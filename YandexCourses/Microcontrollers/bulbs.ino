#define LED_R 13
#define LED_G 12
#define LED_B 14

#define BNT_R 18
#define BNT_G 19
#define BNT_B 21

int lastButtonsState[3] = {HIGH, HIGH, HIGH};
int ledPins[3] = {LED_R, LED_G, LED_B};
bool ledsState[3] = {false, false, false};

void setup()
{
    pinMode(BNT_R, INPUT_PULLUP);
    pinMode(BNT_G, INPUT_PULLUP);
    pinMode(BNT_B, INPUT_PULLUP);

    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
}

void loop()
{

    int currentButtonsState[3] = {
        digitalRead(BNT_R),
        digitalRead(BNT_G),
        digitalRead(BNT_B)};

    for (uint8_t index = 0; index < 3; index++)
    {

        if (lastButtonsState[index] == HIGH && currentButtonsState[index] == LOW)
        {
            ledsState[index] = !ledsState[index];
            digitalWrite(ledPins[index], ledsState[index]);
        }
        lastButtonsState[index] = currentButtonsState[index];
    }

    delay(100);
}