/*  gatchaProj_walk_motor_test.ino
    걸음 감지시 모터 회전 테스트
    걸음 상태 : 걸음 감지 analog pin 전압이 일정 값 이상 -> T1초동안 걸음 상태 ON
    모터는 T2초만큼 회전함 (조금씩)

    2021-06-16
    Minju Park (pmz671712@gmail.com)
*/

#define PIN_IN1 2
#define PIN_IN2 3
#define PIN_IN3 4
#define PIN_IN4 5
#define PIN_WALKING_DETECTION A0  // Arduino Digital pin. for Monitoring walking.
#define PIN_ANALOG_MAGNETIC_SENSOR A1
#define PIN_DIGITAL_MAGNETIC_SENSOR 8

extern volatile unsigned long timer0_millis;

const int WALK_DETECT_THRESHOLD = 100;
const int ISWALKING_MAX_TIME = 1500;     // ms

const int MAIN_MOTOR_ON_TIME = 1000;       // ms
const int MAIN_MOTOR_REPEAT_TIME = 7000;   // ms

volatile bool isWalking = false;
volatile bool isMainMotorOn = false;
unsigned long lastWalkTime = 0;
unsigned long mainMotorStartTime = 0;
unsigned long mainMotorEndTime = 0;

void setup() {
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);
  pinMode(PIN_WALKING_DETECTION, INPUT);
  // pinMode(PIN_ANALOG_MAGNETIC_SENSOR, INPUT);
  pinMode(PIN_DIGITAL_MAGNETIC_SENSOR, INPUT);

  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);
}

void loop() {
  // 걸음 감지
  detectWalking();

  // 메인 모터가 작동중일 경우, 작동 시간이 지나면 OFF
  if ( isMainMotorOn &&
      timer0_millis - mainMotorStartTime > MAIN_MOTOR_ON_TIME )
    MainMotorOff();        

  // 걷는중 상태이고 메인모터 꺼져있는 경우 일정 시간 이상 지났으면 모터 On
  if (isWalking) {
    if ( !isMainMotorOn &&
        timer0_millis - mainMotorEndTime > MAIN_MOTOR_REPEAT_TIME )
        MainMotorOn();
  }
}

void detectWalking() {
  int readData = analogRead(PIN_WALKING_DETECTION);

  if ( isWalking ) {
    if ( PIN_WALKING_DETECTION >= WALK_DETECT_THRESHOLD )
      lastWalkTime = timer0_millis;
    else if( timer0_millis - lastWalkTime > ISWALKING_MAX_TIME )
        isWalking = false;
  }
  else {
    if ( PIN_WALKING_DETECTION >= WALK_DETECT_THRESHOLD ) {
      isWalking = true;
      lastWalkTime  = timer0_millis;
    }
  }
}

void MainMotorOn() {
  digitalWrite(PIN_IN1, HIGH);
  digitalWrite(PIN_IN2, LOW);
  mainMotorStartTime = timer0_millis;
  isMainMotorOn = true;
}

void MainMotorOff() {
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  mainMotorEndTime = timer0_millis;
  isMainMotorOn = false;
}