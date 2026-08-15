const int ledD7 = 7;
const int ledD8 = 8;
const unsigned long intervalMs = 1000;

void setup() {
	pinMode(ledD7, OUTPUT);
	pinMode(ledD8, OUTPUT);
}

void loop() {
	digitalWrite(ledD7, HIGH);
	digitalWrite(ledD8, LOW);
	delay(intervalMs);

	digitalWrite(ledD7, LOW);
	digitalWrite(ledD8, HIGH);
	delay(intervalMs);
}
