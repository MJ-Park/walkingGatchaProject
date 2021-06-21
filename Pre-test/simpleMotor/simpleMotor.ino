// 가챠 모터 회전 계속 하는 프로그램

#define PIN_IN1 2
#define PIN_IN2 3
#define PIN_IN3 4
#define PIN_IN4 5

extern volatile unsigned long timer0_millis;
unsigned long startTime;

const int MOTOR_ROTATE_TIME = 5000;

void setup() {
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);

  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);

  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, HIGH);
//  delay(MOTOR_ROTATE_TIME);
//  digitalWrite(PIN_IN3, LOW);
//  digitalWrite(PIN_IN4, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
}
