// 자석이 감지되면 회전을 멈추는 프로그램
// 최초에 감지되는 자석은 무시 (1.5초동안)

#define PIN_IN1 2
#define PIN_IN2 3
#define PIN_IN3 4
#define PIN_IN4 5
#define PIN_GATCHA_MAGNET_SENSOR 8

extern volatile unsigned long timer0_millis;
unsigned long startTime;

const int MAGNET_IGNORE_TIME = 2000;

volatile int gatchaMode = 0;  // 0은 초기단계. 1은 도는중. 2는 완료상태.
int readData = 0;

void setup() {
//  Serial.begin(9600);
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);
  pinMode(PIN_GATCHA_MAGNET_SENSOR, INPUT);
  

  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);

  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, HIGH);

  startTime = timer0_millis;
}

void loop() {
  readData = digitalRead(PIN_GATCHA_MAGNET_SENSOR);
//  Serial.println(readData);
  switch(gatchaMode) {
    case 0:
      if (timer0_millis - startTime >= MAGNET_IGNORE_TIME)
        gatchaMode++;
    break;

    case 1:
//      if (digitalRead(PIN_GATCHA_MAGNET_SENSOR))
        if(readData)
          gatchaMode++;
    break;
    
    case 2:
      digitalWrite(PIN_IN3, LOW);
      digitalWrite(PIN_IN4, LOW);
    break;
    default:
      delay(100000);
    break;
  }
}
