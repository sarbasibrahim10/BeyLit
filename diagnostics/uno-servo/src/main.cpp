#include <Arduino.h>
#include <Servo.h>
Servo servo;
bool active = false;
int pulse = 1500;
unsigned phase = 0;
uint32_t started = 0, stepped = 0;
void stopTest() {
    servo.detach();
    pinMode(9, OUTPUT);
    digitalWrite(9, LOW);
    active = false;
    Serial.println(F("Servo signal OFF"));
}
void setup() {
    Serial.begin(115200);
    stopTest();
    Serial.println(F("UNO SG90 TEST: unloaded servo signal on D9. Send T to test, X to stop."));
}
void loop() {
    if (Serial.available()) {
        char c = Serial.read();
        if (c == 'X' || c == 'x') stopTest();
        if ((c == 'T' || c == 't') && !active) {
            servo.attach(9, 1400, 1600);
            pulse = 1500;
            servo.writeMicroseconds(pulse);
            active = true;
            phase = 0;
            started = stepped = millis();
            Serial.println(F("TEST: centering then 1400 -> 1600 -> 1500 us; no position feedback"));
        }
    }
    if (!active) return;
    uint32_t now = millis();
    if ((uint32_t)(now - started) >= 12000) { stopTest(); return; }
    if ((uint32_t)(now - started) < 1000 || (uint32_t)(now - stepped) < 20) return;
    stepped = now;
    const int targets[] = {1400, 1600, 1500};
    int target = targets[phase];
    if (pulse < target) ++pulse;
    else if (pulse > target) --pulse;
    servo.writeMicroseconds(pulse);
    if (pulse == target) {
        Serial.print(F("Command us=")); Serial.println(pulse);
        if (++phase == 3) stopTest();
    }
}
