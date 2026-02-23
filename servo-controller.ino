#include <BluetoothSerial.h>
#include <ESP32Servo.h>

BluetoothSerial SerialBT;
Servo myServo;

// Pins
#define SERVO_PIN 18
#define SERVO_PWR 19
#define BUZZER_PIN 5
#define BUTTON_PIN 25

// Parameters
int maxAngle = 90;   // default
int sweepSpeed = 10; // delay in ms per degree (lower = faster)

// States
bool sweeping = false;
unsigned long lastButtonTime = 0; // debounce

void beep(int duration = 80)
{
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration);
    digitalWrite(BUZZER_PIN, LOW);
}

void setup()
{
    Serial.begin(115200);
    SerialBT.begin("ESP32-SERVO-CONFIG");

    pinMode(BUTTON_PIN, INPUT); // External pull-down as you said
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(SERVO_PWR, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(SERVO_PWR, LOW);

    myServo.attach(SERVO_PIN);
    myServo.write(0);

    beep(); // boot beep
    Serial.println("ESP32 Servo System Ready");
}

void sweepOnce()
{
    digitalWrite(SERVO_PWR, HIGH);
    sweeping = true;
    beep(); // start indication
    for (int pos = 0; pos <= maxAngle; pos++)
    {
        myServo.write(pos);
        delay(sweepSpeed);
    }

    beep(); // end indication
    sweeping = false;
    digitalWrite(SERVO_PWR, LOW);
}

void processBTCommand(String cmd)
{
    cmd.trim();
    cmd.toUpperCase();

    if (cmd.startsWith("SET ANGLE"))
    {
        int val = cmd.substring(10).toInt();
        if (val >= 0 && val <= 180)
        {
            maxAngle = val;
            SerialBT.println("OK: Max angle set to " + String(maxAngle));
            beep(40);
        }
        else
        {
            SerialBT.println("ERR: Angle must be 0–180");
        }
    }

    else if (cmd.startsWith("SET SPEED"))
    {
        int val = cmd.substring(10).toInt();
        if (val > 0 && val <= 100)
        {
            sweepSpeed = val;
            SerialBT.println("OK: Speed set to delay=" + String(sweepSpeed) + "ms/degree");
            beep(40);
        }
        else
        {
            SerialBT.println("ERR: Speed must be 1–100");
        }
    }

    else if (cmd == "GET")
    {
        SerialBT.println("------ Current Config ------");
        SerialBT.println("Max Angle : " + String(maxAngle));
        SerialBT.println("Speed     : " + String(sweepSpeed) + " ms/deg");
        SerialBT.println("----------------------------");
    }

    else
    {
        SerialBT.println("Unknown command");
    }
}

void loop()
{

    // --- Bluetooth Commands ---
    if (SerialBT.available())
    {
        String cmd = SerialBT.readStringUntil('\n');
        processBTCommand(cmd);
    }

    // --- Button Handling ---
    if (digitalRead(BUTTON_PIN) == LOW && !sweeping)
    {
        if (millis() - lastButtonTime > 300)
        { // debounce
            lastButtonTime = millis();
            sweepOnce();
        }
    }
}
