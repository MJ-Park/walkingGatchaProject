/*  gatchaProj_main_v0.2.ino

    2021-06-19
    Minju Park (pmz671712@gmail.com)
*/

// 아두이노 PIN
#define PIN_IN1 2                      // 메인모터 PIN
#define PIN_IN2 3                      // 메인모터 PIN
#define PIN_IN3 4                      // 가챠모터 PIN
#define PIN_IN4 5                      // 가챠모터 PIN
#define PIN_WALKING_DETECTION A0       // 워킹패드 모터 전압 감지 Analog PIN
#define PIN_ANALOG_MAGNETIC_SENSOR A1  // 리니어 홀센서 Analog PIN
#define PIN_GATCHA_MAGNET_SENSOR 8  // 리니어 홀센서 Digital PIN
#define PIN_MAIN_MAGNET_SENSOR 9

// timer0 외부변수 정의
extern volatile unsigned long timer0_millis;

// 구동을 위한 상수 정의
const int WALK_DETECT_THRESHOLD = 25;   // 모터회전 전압을 걷는중 상태로 감지하기 위한 임계값
const int MAX_ISWALKING_TIME = 1500;     // (ms) 걸음 감지가 안됐을경우 걷지 않음으로 변경하기 위한 시간값
const int MAX_MAGNET_WAIT_TIME = 1000;   // (ms) 자석 감지가 안됐을 경우 원점 벗어남으로 변경하기 위한 시간값
const int AFTER_GATCHA_DELAY_TIME = 10000; // (ms) 가챠 투하 후 기기 휴식시간값
const int MIN_GATCHA_MOTOR_ON_TIME = 1500;  // (ms) 시작하자마자 가챠 투하모드 끝나는거 방지하기 위한 최소 작동 시간
const int MAX_GATCHA_MOTOR_ON_TIME = 5000;   // (ms) 가챠 투하모드 최대 지속시간
const int MIN_WALK_MODE_DURATION_TIME = 5000; // (ms) 걷기모드 최소 지속시간

// 동작 상태 관련 변수 (boolean)
volatile bool isWalking = false;        // true = 걷는중 / false = 걷지않는중
volatile bool isMainMotorOn = false;    // true = 메인모터 작동중 / false = 메인모터 작동X
volatile bool isGatchaMotorOn = false;
volatile bool isOrigin = false;         // true = 현재 원점(자석감지o) / false = 원점X(자석감지X)

// 시간 변수 (unsigned long)
unsigned long lastWalkTime = 0;         // 마지막으로 걸음 간지된 시간
unsigned long mainMotorOnTime = 0;      // 마지막으로 메인모터 작동 시작 시간
unsigned long mainMotorOffTime = 0;     // 마지막으로 메인모터 작동 종료 시간
unsigned long lastOriginTime = 0;       // 마지막으로 원점에 도착한 시간
unsigned long gatchaMotorOnTime = 0;    // 마지막으로 가챠모터 작동 시작 시간
unsigned long gatchaMotorOffTime = 0;   // 마지막으로 가챠모터 작동 종료 시간
unsigned long walkModeStartTime = 0;    // 걷는중 모드 시작된 시간 (한바퀴 돌기 시작한 시간 감지 위해)

// Count 변수
unsigned int total_complete = 0;

// 모드 변수
// 0 : 부팅 / 1 : 대기 / 2 : 걷는중 / 3 : 완료
// 4 : 원점복구 / 5 : 오류
volatile uint8_t mode = 0x00;

void setup() {
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);
  pinMode(PIN_WALKING_DETECTION, INPUT);
  // pinMode(PIN_ANALOG_MAGNETIC_SENSOR, INPUT);
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
      mode = 0x01;
      break;
    }
    // 1초동안 기다렸는데도 자석 감지 안되면 원점 복구모드
    else if(timer0_millis > 1000){
      if ( !isMainMotorOn ) {
        mainMotorOn();
      }
      // mode = 0x04;  // 원점 복구모드로 전환
      // break;
    }
  }
}

void loop() {
  // 오류를 최소화하기 위해 4가지 모드와 에러 상태 사용
  switch(mode) {
    case 0x01:  // 대기 모드
      detectWalking();
//      detectMainOrigin();
      if (isWalking) {
        walkModeStartTime = timer0_millis;
        mode++; // 걷는중 모드로 전환
      }
    break;

    case 0x02:  // 걷는중 모드
      detectWalking();
      detectMainOrigin();

      if (isWalking) {
        //걷는중 + 메인모터 꺼져있으면 메인모터 켜고 5초 딜레이
        if(!isMainMotorOn) {
          mainMotorOn();
          delay(MIN_WALK_MODE_DURATION_TIME);
        }
        // 걷는중 + 메인모터 켜져있음 + 원점도달하면 모터 끄고 완료 모드로 전환
        else if(isOrigin) {
          mainMotorOff();
          mode++;   // 완료 모드로 전환
          delay(1000);
        }
      }
      else {
        if(isMainMotorOn) {
          // if(timer0_millis - mainMotorOnTime > MIN_WALK_MODE_DURATION_TIME ) {
            mainMotorOff();
            mode = 0x04;  // 중간에 이탈한 경우 복구 모드로 전환
          // }
        }
      
      }
    break;

    case 0x03:  // 완료 모드 (가챠 투하 모드).
      gatchaMotorOn();
      delay(MIN_GATCHA_MOTOR_ON_TIME);  // 시작하자마자 자석 감지해서 끝나는거 방지
      while(1) {
        if (digitalRead(PIN_GATCHA_MAGNET_SENSOR))  break;  // 정상적으로 자석 감지해서 종료
        else if(timer0_millis - gatchaMotorOnTime >= MAX_GATCHA_MOTOR_ON_TIME) break; // 만약 정상적으로 자석 감지하지 못하더라도 일정 시간 지나면 종료
      }
      gatchaMotorOff();

      total_complete++;
      mode = 0x01;  // 대기 모드로 전환
      delay(AFTER_GATCHA_DELAY_TIME); // 가챠 한번 떨어지면 일정 시간 후에 대기 모드로 전환 (연속 작동 x)
    break;

    case 0x04:  // 원점 복구 모드
      detectMainOrigin();
      if (isOrigin) {
        mainMotorOff();
        mode = 0x01;  //대기 모드로 전환
      }
      else if (!isMainMotorOn) mainMotorOn();
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

void gatchaMotorOn() {
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, HIGH);
  gatchaMotorOnTime = timer0_millis;
  isGatchaMotorOn = true;
}

void gatchaMotorOff() {
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);
  gatchaMotorOffTime = timer0_millis;
  isGatchaMotorOn = false;
}


    // case 0x02:  // 걷는중 모드
    //   detectWalking();
    //   detectMainOrigin();

    //   // // 조금씩 회전하기 위해서 메인모터 일정 시간 이상 작동하면 꺼짐 
    //   // if( isMainMotorOn && timer0_millis - mainMotorOnTime > MAIN_MOTOR_ON_TIME )
    //   //   mainMotorOff();


    //   // 한바퀴 다 돌았는지 감지. 메인모터 끄기
    //   if( isOrigin ) {
    //     if ( timer0_millis - walkModeStartTime > MIN_WALK_MODE_DURATION_TIME) {
    //       mainMotorOff();
    //       mode++;   // 완료 모드로 전환
    //     }
    //   }

    //   // 중간에 이탈했는지 감지
    //   if (!isWalking) {
    //     mainMotorOff();
    //     mode = 0x04;  // 원점 복구 모드로 전환
    //   }

    //   // // 메인모터가 마지막으로 작동한지 일정 시간 이상 됐으면 작동
    //   // if( !isMainMotorOn && timer0_millis - mainMotorOffTime > MAIN_MOTOR_REPEAT_TIME )
    //   //   mainMotorOn();

    // break;
