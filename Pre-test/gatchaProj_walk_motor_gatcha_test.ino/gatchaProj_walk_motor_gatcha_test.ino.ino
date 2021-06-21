/*  gatchaProj_walk_motor_gatcha_test.ino
    한바퀴 다 돌고 원점이 감지되면 가챠 투하 테스트

    2021-06-16
    Minju Park (pmz671712@gmail.com)
*/

// 아두이노 PIN
#define PIN_IN1 2                      // 메인모터 PIN
#define PIN_IN2 3                      // 메인모터 PIN
#define PIN_IN3 4                      // 가챠모터 PIN
#define PIN_IN4 5                      // 가챠모터 PIN
#define PIN_WALKING_DETECTION A0       // 워킹패드 모터 전압 감지 Analog PIN
#define PIN_ANALOG_MAGNETIC_SENSOR A1  // 리니어 홀센서 Analog PIN
#define PIN_DIGITAL_MAGNETIC_SENSOR 8  // 리니어 홀센서 Digital PIN

// timer0 외부변수 정의
extern volatile unsigned long timer0_millis;

// 구동을 위한 상수 정의
const int WALK_DETECT_THRESHOLD = 100;   // 모터회전 전압을 걷는중 상태로 감지하기 위한 임계값
const int ISWALKING_MAX_TIME = 1500;     // (ms) 걸음 감지가 안됐을경우 걷지 않음으로 변경하기 위한 시간값
const int MAIN_MOTOR_ON_TIME = 1000;     // (ms) 메인모터가 한번 작동하는 시간값
const int MAIN_MOTOR_REPEAT_TIME = 7000; // (ms) 지속적으로 걸을시 메인모터 작동 반복 딜레이 시간값
const int MAX_MAGNET_WAIT_TIME = 1000;   // (ms) 자석 감지가 안됐을 경우 원점 벗어남으로 변경하기 위한 시간값
const int AFTER_GATCHA_DELAY_TIME = 10000; // (ms) 가챠 투하 후 기기 휴식시간값
const int GATCHA_MOTOR_ON_TIME = 1000;   // (ms) 가챠 투하시 모터 작동 시간값

// 동작 상태 관련 변수 (boolean)
volatile bool isWalking = false;        // true = 걷는중 / false = 걷지않는중
volatile bool isMainMotorOn = false;    // true = 메인모터 작동중 / false = 메인모터 작동X
volatile bool isOrigin = false;         // true = 현재 원점(자석감지o) / false = 원점X(자석감지X)

// 시간 변수 (unsigned long)
unsigned long lastWalkTime = 0;         // 마지막으로 걸음 간지된 시간
unsigned long mainMotorOnTime = 0;      // 마지막으로 메인모터 작동 시작 시간
unsigned long mainMotorOffTime = 0;     // 마지막으로 메인모터 작동 종료 시간
unsigned long lastOriginTime = 0;       // 마지막으로 원점에 도착한 시간
unsigned long gatchaMotorOnTime = 0;    // 마지막으로 가챠모터 작동한 시간 ( 한바퀴 완료 시간 )

// Count 변수
unsigned int total_complete = 0;

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
  // 걸음 감지, 자기장(원점) 감지
  detectWalking();
  detectMagnet();

  // 메인 모터가 작동중일 경우, 작동 시간이 지나면 OFF
  if ( isMainMotorOn &&
      timer0_millis - mainMotorOnTime > MAIN_MOTOR_ON_TIME )
    mainMotorOff();        

  // 걷는중 상태이고 메인모터 꺼져있는 경우 일정 시간 이상 지났으면 모터 On
  if (isWalking) {
    if ( !isMainMotorOn &&
        timer0_millis - mainMotorOffTime > MAIN_MOTOR_REPEAT_TIME )
        mainMotorOn();
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

void detectMagnet() {
  int readData = digitalRead(PIN_DIGITAL_MAGNETIC_SENSOR);

  if (isOrigin) {
    if ( readData )
      lastOriginTime = timer0_millis;
    else if ( timer0_millis - lastOriginTime > MAX_MAGNET_WAIT_TIME )
      isOrigin = false;
  }
  else {
    if ( readData ) {
      // 홀센서 값이 튀는 경우에는 count 변수 추가해서 몇 이상이면 상태 바꾸도록 하기
      isOrigin = true;
      lastOriginTime = timer0_millis;
    }
  }
}

void mainMotorOn() {
  digitalWrite(PIN_IN1, HIGH);
  digitalWrite(PIN_IN2, LOW);
  mainMotorOnTime = timer0_millis;
  isMainMotorOn = true;
}

void mainMotorOff() {
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  mainMotorOffTime = timer0_millis;
  isMainMotorOn = false;
}

void completeMode() {
  digitalWrite(PIN_IN3, HIGH);
  digitalWrite(PIN_IN4, LOW);
  delay(GATCHA_MOTOR_ON_TIME);
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);

  total_complete++;
  delay(AFTER_GATCHA_DELAY_TIME);
}