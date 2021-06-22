const int PIN_ANALOG = A0;

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_ANALOG, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(analogRead(PIN_ANALOG));
  delay(100);
}
