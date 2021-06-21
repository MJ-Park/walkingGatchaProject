/*  gatchaProj_main_v0.3.ino

    v0.3 변경점 : 각 모드를 while문으로 묶고, 특정 조건에서 break 하도록 변경
    2021-06-19
    Minju Park (pmz671712@gmail.com)
*/

// 아두이노 PIN
#define PIN_IN1 2                      // 메인모터 PIN
#define PIN_IN2 3                      // 메인모터 PIN
#define PIN_IN3 4                      // 가챠모터 PIN
#define PIN_IN4 5                      // 가챠모터 PIN
#define PIN_WALKING_DETECTION A0       // 워킹패드 모터 전압 감지 Analog PIN
#define PIN_GATCHA_MAGNET_SENSOR 8  // 리니어 홀센서 Digital PIN
#define PIN_MAIN_MAGNET_SENSOR 9

#define MODE_INIT 0x00
#define MODE_WAITING 0x01
#define MODE_WALKING 0x02
#define MODE_GATCHA 0x03
#define MODE_RESTORE 0x04

// timer0 외부변수 정의
extern volatile unsigned long timer0_millis;

// 구동을 위한 상수 정의
const int WALK_DETECT_THRESHOLD = 25;   // 모터회전 전압을 걷는중 상태로 감지하기 위한 임계값
const int MAX_ISWALKING_TIME = 1500;     // (ms) 걸음 감지가 안됐을경우 걷지 않음으로 변경하기 위한 시간값
const int MAX_WALK_MODE_TIME = 180000;   // (ms) 3분동안 걸었는데 원점 감지가 안되는 경우를 위한 시간값
const int MAX_MAGNET_WAIT_TIME = 1000;   // (ms) 자석 감지가 안됐을 경우 원점 벗어남으로 변경하기 위한 시간값
const int AFTER_GATCHA_DELAY_TIME = 10000; // (ms) 가챠 투하 후 기기 휴식시간값
const int MIN_GATCHA_MOTOR_ON_TIME = 1500;  // (ms) 시작하자마자 가챠 투하모드 끝나는거 방지하기 위한 최소 작동 시간
const int MAX_GATCHA_MOTOR_ON_TIME = 5000;   // (ms) 가챠 투하모드 최대 지속시간
const int MIN_WALK_MODE_TIME = 5000; // (ms) 걷기모드 최소 지속시간

// 동작 상태 관련 변수 (boolean)
volatile bool isWalking = false;        // true = 걷는중 / false = 걷지않는중
volatile bool isMainMotorOn = false;    // true = 메인모터 작동중 / false = 메인모터 작동X
volatile bool isGatchaMotorOn = false;
volatile bool isOrigin = false;         // true = 현재 원점(자석감지o) / false = 원점X(자석감지X)

// 시간 변수 (unsigned long)
unsigned long lastWalkTime = 0;         // 마지막으로 걸음 간지된 시간
unsigned long mainMotorOnTime = 0;      // 마지막으로 메인모터 작동 시작 시간
unsigned long lastOriginTime = 0;       // 마지막으로 원점에 도착한 시간
unsigned long gatchaMotorOnTime = 0;    // 마지막으로 가챠모터 작동 시작 시간

// Count 변수
unsigned int total_complete = 0;

// 모드 변수
// 0 : 부팅 / 1 : 대기 / 2 : 걷는중 / 3 : 완료
// 4 : 원점복구 / 5 : 오류
volatile uint8_t mode = MODE_INIT;

void setup() {
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);
  pinMode(PIN_WALKING_DETECTION, INPUT);
  pinMode(PIN_GATCHA_MAGNET_SENSOR, INPUT);
  pinMode(PIN_MAIN_MAGNET_SENSOR, INPUT);

  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);

  // 자석이 원점에 있는지 확인
  while(1) {
    detectMainOrigin();
    // 원점에 있으면 바로 대기 모드로 전환
    if (isOrigin){
      mode = MODE_WAITING;
      break;
    }
    // 1초동안 기다렸는데도 자석 감지 안되면 원점 복구모드
    else if(timer0_millis > 1000){
      mode = MODE_RESTORE;
      break;
    }
  }
}

void loop() {
  // 오류를 최소화하기 위해 4가지 모드와 에러 상태 사용
  switch(mode) {
    case MODE_WAITING:  // 대기 모드
      while(1) {
        detectWalking();
        if (isWalking) {
          mode = MODE_WALKING; // 걷는중 모드로 전환
          // 메인모터 켜기
          digitalWrite(PIN_IN1, HIGH);
          digitalWrite(PIN_IN2, LOW);
          mainMotorOnTime = timer0_millis;
          isMainMotorOn = true;
          delay(MIN_WALK_MODE_TIME);
          break;
        }
      }
    break;

    case MODE_WALKING:  // 걷는중 모드
      while(1) {
        detectWalking();
        detectMainOrigin();

        if(!isWalking) {
          // 중간에 움직임 멈추면 모터 종료 후 복구 모드로 전환
          digitalWrite(PIN_IN1,LOW);
          digitalWrite(PIN_IN2,LOW);
          isMainMotorOn = false;

          mode = MODE_RESTORE;
          delay(500);
          break;
        }
        else if(isOrigin) {
          // 움직임 멈춤 없이 원점까지 되돌아오면 모터 종료 후 가챠모드로 전환
          digitalWrite(PIN_IN1,LOW);
          digitalWrite(PIN_IN2,LOW);
          isMainMotorOn = false;

          mode = MODE_GATCHA;
          delay(500);
          break;
        }

        if(timer0_millis - mainMotorOnTime >= MAX_WALK_MODE_TIME) {
          // 3분 이상 걸었는데 원점 감지가 안된 경우
          // 가챠 일단 하나 주고 복구모드로 전환
          gatchaMotorOn();
          delay(MIN_GATCHA_MOTOR_ON_TIME);
          while(1) {
            if (digitalRead(PIN_GATCHA_MAGNET_SENSOR))  break;  // 정상적으로 자석 감지해서 종료
            else if(timer0_millis - gatchaMotorOnTime >= MAX_GATCHA_MOTOR_ON_TIME) break; // 만약 정상적으로 자석 감지하지 못하더라도 일정 시간 지나면 종료
          }
          gatchaMotorOff();
          delay(5000);
          mode = MODE_RESTORE;
          break;
        }
      }
    break;

    case MODE_GATCHA:  // 완료 모드 (가챠 투하 모드).
      gatchaMotorOn();
      delay(MIN_GATCHA_MOTOR_ON_TIME);  // 시작하자마자 자석 감지해서 끝나는거 방지
      while(1) {
        if (digitalRead(PIN_GATCHA_MAGNET_SENSOR))  break;  // 정상적으로 자석 감지해서 종료
        else if(timer0_millis - gatchaMotorOnTime >= MAX_GATCHA_MOTOR_ON_TIME) break; // 만약 정상적으로 자석 감지하지 못하더라도 일정 시간 지나면 종료
      }
      gatchaMotorOff();

      total_complete++;
      mode = MODE_WAITING;  // 대기 모드로 전환
      delay(AFTER_GATCHA_DELAY_TIME); // 가챠 한번 떨어지면 일정 시간 후에 대기 모드로 전환 (연속 작동 x)
    break;

    case MODE_RESTORE:  // 원점 복구 모드
      while(1) {
        detectMainOrigin();
        // 메인모터 거꾸로 돌릴수 있으면 거꾸로 돌리는게 나을듯
        if(isOrigin) {
          digitalWrite(PIN_IN1,LOW);
          digitalWrite(PIN_IN2,LOW);
          isMainMotorOn = false;
          mode = MODE_WAITING;    // 대기 모드로 전환
          delay(500); 
          break;
        }
        else if(!isMainMotorOn) {
          digitalWrite(PIN_IN1, LOW);
          digitalWrite(PIN_IN2, HIGH);
          isMainMotorOn = true;
        }
      }

    break;

    default:  // Error state
    //
    break;
  }
}


void detectWalking() {
  int readData = analogRead(PIN_WALKING_DETECTION);
  // int readData = 1023;

  if ( isWalking ) {
    if ( readData >= WALK_DETECT_THRESHOLD )
      lastWalkTime = timer0_millis;
    else if( timer0_millis - lastWalkTime > MAX_ISWALKING_TIME )
        isWalking = false;
  }
  else {
    if ( readData >= WALK_DETECT_THRESHOLD ) {
      isWalking = true;
      lastWalkTime  = timer0_millis;
    }
  }
}

void detectMainOrigin() {
  int readData = digitalRead(PIN_MAIN_MAGNET_SENSOR);

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

}

void mainMotorOff() {
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  isMainMotorOn = false;
}

void gatchaMotorOn() {
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, HIGH);
  gatchaMotorOnTime = timer0_millis;
  isGatchaMotorOn = true;
}

void gatchaMotorOff() {
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);
  isGatchaMotorOn = false;
}
