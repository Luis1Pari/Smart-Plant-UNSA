int sensorHumedad = A0;
int bomba = 5;
int lectura = 0;
int humedad = 0;
void setup() {
  Serial.begin(9600); 
  pinMode(bomba, OUTPUT);
}

void loop() {
 lectura = analogRead(sensorHumedad); 
 humedad = map(lectura, 0, 880, 0, 100);
 if (humedad <90) {
   digitalWrite(bomba, HIGH);
 } else {
   digitalWrite(bomba, LOW);
 }
  Serial.println(sensorHumedad);
  delay(500);
}
