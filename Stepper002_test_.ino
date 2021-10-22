#include <LiquidCrystal.h>  
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
#define pul 3 //Pulse
#define dir 2 //Direction
#define x 3200 //Steps per revolution
float f, r, i ; int keypad_pin = A0;  int Keypad_value = 0; int Keypad_value_old = 0;   char Btn_push;    int Btn = 49;   int Btnval = 0;
int volume = 1; int volumeOld = 1;    int MenuPage = 1; int MenuPageOld = 1;    int times = 1;  int timesold = 1;
unsigned long t ;    int clicks = 0;   int okein = 0;  unsigned long timePress = 0;  unsigned long timePressLimit = 0;
long previousMillis = 0;  unsigned long currentMillis ;   long interval = 1000;   unsigned long multiplier = 1000; //value to change based on calibration

void setup(){lcd.begin(16, 2);  analogWrite(10, 20);  lcd.setCursor(2, 0);  lcd.print("Syringe Pump");  lcd.setCursor(6, 1);  lcd.print("Test");  delay(300);  
              MenuPage = 1;   MenuDisplay(MenuPage);  pinMode(Btn, INPUT);  pinMode(dir, OUTPUT);       pinMode(pul, OUTPUT);}

void loop() {  Btn_push = ReadKeypad();   Btnval = digitalRead(Btn); MenuPage = 1; if(MenuPage != MenuPageOld){MenuDisplay(MenuPage);  MenuPageOld = MenuPage;}
  if (MenuPage == 1)    {if (Btn_push == 'U' && volume < 10) volume += 1;     if (Btn_push == 'D' && volume >  0 ) volume -= 1;
                         if (volumeOld != volume) {lcd.setCursor(8, 0);  lcd.print(volume);    volumeOld = volume;}
                         if (Btn_push == 'R' && times < 100) times += 1;       if (Btn_push == 'L' && times >  0 ) times -= 1;
                         if (timesold != times) {lcd.setCursor(8, 1);  lcd.print(times);    timesold = times;}
  if (Btn_push == 'S' ) {Refill(volume,times); MenuDisplay(MenuPage);}  
  if (Btnval   == LOW ) {Inject(volume,times); MenuDisplay(MenuPage);}}  delay(500);}//loop
  
void MenuDisplay(int page) {switch(page){case 1:lcd.clear();  lcd.setCursor(0, 0);lcd.print("Volume: ");      lcd.setCursor(8, 0);lcd.print(volume);lcd.print(" ml");      
                                                              lcd.setCursor(0, 1);lcd.print("Waktu:  ");      lcd.setCursor(8, 1);lcd.print(times); lcd.print(" sec"); break;}}
void Inject(unsigned long ml, unsigned int sec){previousMillis = millis();  currentMillis = previousMillis;  interval = ml *multiplier;
  f = ml*1000;                  r = (f/(4*2*166.8));                          t = (sec*1000/(x*r))/2*1000; //f = mL to uL //r = rotasi //t = wkt untuk delayMicrosecond
  lcd.setCursor(0, 1);  lcd.print("                ");  digitalWrite(dir, HIGH);      
  for (i = 0; i < x*r; i++){digitalWrite(pul, HIGH);      delayMicroseconds(t);       digitalWrite(pul, LOW) ;      delayMicroseconds(t);}
  lcd.setCursor(3, 1);      lcd.print("Inject Done");   delay(1000);}
void Refill(unsigned long ml, unsigned int sec) {  previousMillis = millis();  currentMillis = previousMillis;  interval = ml *multiplier;
  f = ml*1000;                  r = (f/(4*2*166.8));                          t = (sec*1000/(x*r))/2*1000; 
  lcd.setCursor(0, 1);  lcd.print("                ");   digitalWrite(dir, LOW);       
  for (i = 0; i < x*r; i++){digitalWrite(pul, HIGH);      delayMicroseconds(t);      digitalWrite(pul, LOW) ;      delayMicroseconds(t);  }
  lcd.setCursor(3, 1);  lcd.print("Refill Done");   delay(1000);}
char ReadKeypad() {Keypad_value = analogRead(keypad_pin);if(Keypad_value < 100)return 'R';  else if(Keypad_value < 200)return 'U';  else if(Keypad_value < 400)return 'D';  
                                                         else if (Keypad_value < 600) return 'L';  else if (Keypad_value < 800) return 'S'; else return 0;}
