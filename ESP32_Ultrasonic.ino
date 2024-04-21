#define BLYNK_TEMPLATE_ID "TMPL6lqyOceVE"
#define BLYNK_TEMPLATE_NAME "345"
#define BLYNK_AUTH_TOKEN "F8i78OY5kmrr_o92uO7E4zaMf1lbYsyx"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#define TRIG_PIN 5
#define ECHO_PIN 4
#define buzzerpin 14
#define BOTTLE_HEIGHT_CM 400

char ssid[] = "Wokwi-GUEST"; 
char pass[] = "";

bool buzzer_triggered = false; 
bool buzzer_triggered2 = false;

unsigned long previousMillis = 0;
const long interval = 15000;

BlynkTimer timer;

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(buzzerpin, OUTPUT);

  Serial.begin(9600);
  Serial.print("DISTANT : ");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(1000L, sendSensorData); // ตั้งค่าการส่ง Data ทุกๆ 1 วินาที
}

void sendSensorData() {
  float dist = distance();
  float percentage = mapToPercentage(dist);
  Serial.print("Distance: ");
  Serial.print(dist);
  Serial.print(" cm, Percentage: ");
  Serial.print(percentage);
  Serial.println("%");

  if (percentageChanged(percentage)) {
    previousMillis = millis(); // รีเซ็ต timer 
  }

  if (millis() - previousMillis >= interval) {
    if (!buzzer_triggered) {
    tone(buzzerpin, 1000);
    delay(1000);
    noTone(buzzerpin);
    buzzer_triggered = true;
  }; // แจ้งเตือนการดื่มน้ำทุกๆ 15 วินาที
  } else {
    buzzer_triggered = false;
  }

  Blynk.virtualWrite(V1, percentage); // ส่งค่า Percent ไปที่ Blynk
  Blynk.virtualWrite(V2, (millis() - previousMillis) / 1000); // ส่งค่า timer ไปที่ Blynk
}

bool percentageChanged(float newPercentage) { // ตรวจเช็คการเปลี่ยนแปลงของระดับน้ำเพิ่อใช้ในการแจ้งเตือนการดื่มน้ำ
  static float oldPercentage = -1;
  if (newPercentage != oldPercentage) {
    oldPercentage = newPercentage;
    return true;
  }
  return false;
}

float distance() { // ฟั่งชั่น distance ทำงานกับ sensor ultrasonic วัดระยะทางจากตัว sensor กับผิวน้ำ
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);

  float distance_cm = duration * 0.034 / 2; // นำค่าที่วัดได้จาก sensor มาเข้าสูตรการคำนวณเพื่อเปลี่ยนให้เป็นระยะทางแบบเซนติเมตร
  return distance_cm;
}

float mapToPercentage(float distance_cm) { // นำค่าระยะทางแบบเซนติเมตรในฟังชั่น distance มาคำนวณให้เป็น Percent
  float percentage = (1 - (distance_cm / BOTTLE_HEIGHT_CM)) * 100;
  if (percentage < 0) {
    return 0;
  } else if (percentage > 100) {
    return 100;
  } else {
    return percentage;
  }
}

void buzzer(float percentage) { // ฟั่งชั่น Buzzer
  if (percentage < 20 && !buzzer_triggered2) { // เมื่อปริมาณน้ำต่ำกว่า 20% ของขวดให้ทำการส่งเสียงแจ้งเตือนและตั้งค่าให้ปิดการแจ้งเตื่อน
    tone(buzzerpin, 1000);
    delay(1000);
    noTone(buzzerpin);
    buzzer_triggered2 = true;
  }
  if (percentage > 25 && buzzer_triggered2) { // เมื่อปริมาณน้ำมากกว่า 20% ทำการตั้งค่าให้เปิดการแจ้งเตื่อน
    buzzer_triggered2 = false;
  }
}

void loop() {
  float dist = distance();
  float percentage = mapToPercentage(dist);
  Serial.print("Distance: ");
  Serial.print(dist);
  Serial.print(" cm, Percentage: ");
  Serial.print(percentage);
  Serial.println("%");
  buzzer(percentage); // เรียกใช้ฟังชั่น Buzzer
  delay(1000);
  Blynk.run();
  timer.run(); // เริ่มการทำงานของ timer
}
