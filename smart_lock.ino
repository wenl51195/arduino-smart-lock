#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Keypad.h>

// Wi-Fi & IFTTT
const String WIFI_SSID = "your_ssid";
const String WIFI_PASSWORD = "your_password";
const String IFTTT_HOST = "maker.ifttt.com";
const String IFTTT_URI = "/trigger/msg/with/key/your_webhook_key";

// pin
const int LOCK_PIN = 2;
const int LED_PIN = 13;
const int BUZZER_PIN = 3;
const int LDR_PIN = A0;

// keypad
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
    {'1','2','3'},
    {'4','5','6'},
    {'7','8','9'},
    {'*','0','#'}
};
byte rowPins[ROWS] = {12, 11, 10, 9};
byte colPins[COLS] = {8, 7, 6};

// (RX, TX): 設定 ESP 的 RX 接到 Arduino 的 5 號腳位, TX 接到 4 號腳位
SoftwareSerial esp(5, 4);  // ESP_TX=5, ESP_RX=4
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(0x3F, 16, 2);

String currentPasscode = "8888";  // init password
int loginAttempts = 0;
const int MAX_ATTEMPTS = 3;
const int LDR_THRESHOLD = 100;


void setup() {
    esp.begin(115200);
    Serial.begin(9600);

    lcd.begin(16, 2);
    lcd.init();
    lcd.backlight();

    pinMode(LOCK_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LDR_PIN, INPUT_PULLUP);

    digitalWrite(LOCK_PIN, LOW);
    digitalWrite(LED_PIN, LOW);

    connectWiFi();
}

// Connect Wi-Fi
void connectWiFi() {
    esp.println("AT+CWMODE=1");
    delay(1000);

    String cmd = "AT+CWJAP=\"" + WIFI_SSID + "\",\"" + WIFI_PASSWORD + "\"";
    esp.println(cmd);

    while (!esp.find("OK")) {
        Serial.println("Wi-Fi Connect Fail");
        delay(1000);
    }
    Serial.println("Wi-Fi Connected !!");
}

String keyInput(bool showPassword) {  // true: show password, false: hide passwword
    String input = "";
    String displayInput = "";

    lcd.setCursor(0, 1);
    lcd.print("                ");

    while (true) {
        char key = keypad.getKey();
        if (key != NO_KEY) {
            lcd.setCursor(0, 1);
            lcd.print("                ");
            
            if (key == '#') {
                break;
            } else if (key == '*') {
                input = "";
            } else {
                if (input.length() < 10) {  // limit password length
                    input += key;
                }
            }
            
            lcd.setCursor(0, 1);
            if (showPassword) {
                lcd.print(input);
            } else {
                lcd.print(String(input.length(), '*'));
            }
        }
    }

    Serial.println("Your input: " + input);
    lcd.clear();
    return input;
}

void loop() {
    digitalWrite(LOCK_PIN, LOW);

    lcd.setCursor(0, 0);
    lcd.print("Mode:");
    lcd.setCursor(0, 1);
    lcd.print("1:Reset 2:Input");

    String mode = keyInput(true);

    if (mode == "1") {  // reset mode
        lcd.setCursor(0, 0);
        lcd.print("Enter Current");

        if (keyInput(false) == currentPasscode) {
            lcd.setCursor(0, 0);
            lcd.print("Enter New Code");
            
            currentPasscode = keyInput(false);

            lcd.setCursor(0, 0);
            lcd.print("Code Updated!");
            delay(1000);
        }
    } else if (mode == "2") {  // unlock mode
        sendNotification();
        while (loginAttempts < MAX_ATTEMPTS) {
            lcd.setCursor(0, 0);
            lcd.print("Enter Password");
            
            if (keyInput(false) == currentPasscode) {
                lcd.setCursor(0, 0);
                lcd.print("Access!");
                while (true) {
                    int sensorValue = analogRead(LDR_PIN);
                    digitalWrite(LED_PIN, sensorValue >= LDR_THRESHOLD);
                    
                    char key = keypad.getKey();
                    if (key == '#') break;
                    digitalWrite(LOCK_PIN, HIGH);
                }
                loginAttempts = 0;
                break;
            } else {
                lcd.setCursor(0, 0);
                lcd.print("Access Denied!");
                delay(1000);
                loginAttempts++;
            }
        }

        // alert
        if (loginAttempts == MAX_ATTEMPTS) {
            for (int i = 0; i < 10; i++) {
                tone(BUZZER_PIN, 650, 230);
                delay(230);
                noTone(BUZZER_PIN);
                tone(BUZZER_PIN, 1550, 100);
                delay(100);
                noTone(BUZZER_PIN);
            }
            loginAttempts = 0;
        }
    }
}

void sendNotification() {
    esp.println("AT+CIPSTART=\"TCP\",\"" + IFTTT_HOST + "\",80");
    delay(1000);

    String requestBody = 
        "GET " + IFTTT_URI + " HTTP/1.1\r\n" +
        "Host: " + IFTTT_HOST + "\r\n" +
        "Accept: */*\r\n" +
        "Content-Type: application/x-www-form-urlencoded\r\n" +
        "Connection: close\r\n\r\n";

    esp.println("AT+CIPSEND=" + String(requestBody.length()));
    delay(100);

    esp.println(requestBody);
    delay(100);

    esp.println("AT+CIPCLOSE");
}