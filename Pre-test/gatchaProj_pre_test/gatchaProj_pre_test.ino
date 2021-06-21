/*  gatchaProj_pre_test.ino
    조립 전 테스트. 시리얼 연결 필요.
    mode
      1 : 메인 모터 회전 테스트
      2 : 가챠 모터 회전 테스트
      3 : 걸음 감지 테스트
      4 : 자석 감지 테스트

    2021-06-16
    Minju Park (pmz671712@gmail.com)
*/

#define MOTOR_A_TIME 40 // 모터 A가 한바퀴 도는데 걸리는 시간 (실제 기어비 체크해서 한바퀴 = 약 13초정도?)
#define MOTOR_B_TIME 5 // 모터 B가 한바퀴 도는데 걸리는 시간

#define PIN_IN1 2
#define PIN_IN2 3
#define PIN_IN3 4
#define PIN_IN4 5
#define PIN_WALKING_DETECTION A0  // Arduino Digital pin. for Monitoring walking.
#define PIN_ANALOG_MAGNETIC_SENSOR A1
#define PIN_DIGITAL_MAGNETIC_SENSOR 8

volatile uint8_t mode = 0x00;

extern volatile unsigned long timer0_millis;
unsigned long startTime;

volatile long input;

const int MAIN_MOTOR_ROTATE_TIME = 5000;
const int GATCHA_MOTOR_ROTATE_TIME = 5000;

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

  Serial.begin(9600);
}

void loop() {
  Serial.println("1 : 메인 모터 테스트 / 2 : 가챠 모터 테스트");
  Serial.println("3 : 걸음 감지 테스트 / 4 : 자석 감지 테스트");
  Serial.println("모드 번호를 입력하세요 : ");
  while(1) {
    if(Serial.available()){
      input = Serial.parseInt();
      break;
    }
  }

  switch(input) {
    case 1:       // 메인 모터 테스트
      digitalWrite(PIN_IN1, HIGH);
      digitalWrite(PIN_IN2, LOW);
      delay(MAIN_MOTOR_ROTATE_TIME * 1000);
      digitalWrite(PIN_IN1, LOW);
      digitalWrite(PIN_IN2, LOW);
//      input = 0;
      break;
    case 2:       // 가챠 모터 테스트
      digitalWrite(PIN_IN3, LOW);
      digitalWrite(PIN_IN4, HIGH);
      delay(1500);
//      delay(GATCHA_MOTOR_ROTATE_TIME * 1000);
      while(1) {
        if (digitalRead(8)) break;
      }
      
      digitalWrite(PIN_IN3, LOW);
      digitalWrite(PIN_IN4, LOW);
//      input = 0;
      break;
    case 3:       // 걸음 감지 테스트. 시리얼 모니터에 0을 입력하면 종료
      Serial.println("9 입력시 종료합니다.");
      while(1) {
        if (Serial.available()) input = Serial.parseInt();
        if( input == 9 ) break;
        Serial.println(analogRead(PIN_WALKING_DETECTION));
        delay(100);
      }
      break;
    case 4:       // 자석 감지 테스트. 시리얼 모니터에 0을 입력하면 종료
      Serial.println("9 입력시 종료합니다.");
      while(1) {
        if (Serial.available()) input = Serial.parseInt();
        if ( input == 9 ) break;
        Serial.println(digitalRead(PIN_DIGITAL_MAGNETIC_SENSOR));
        // Serial.println(analogRead(PIN_ANALOG_MAGNETIC_SENSOR));
      }
      break;

    default:
      delay(1000);
      break;
  }
}
