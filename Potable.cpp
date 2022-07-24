#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

const int ROW_NUM    = 4; // four rows
const int COLUMN_NUM = 4; // four columns

char keys[ROW_NUM][COLUMN_NUM] = 
{
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};

byte pin_rows[ROW_NUM] = {3, 4, 5, 6};      // connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {7, A3, A2, A1}; // connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows

int cursorColumn = 0;

int LED_PIN = A0;

const int buzzer = 11; //buzzer to arduino pin 11
unsigned long lastPeriodStart;
const int onDuration=1000;
const int periodDuration=2000;


String token = "";
int statusKran = 0; // 0 = tutup, 1 = buka

#define INTERVAL_MESSAGE1 500
#define INTERVAL_MESSAGE2 1000

unsigned long time_1 = 0;
unsigned long time_2 = 0;

int motorpin1 = 12;
int motorpin2 = 13;

//-------------------
byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin       = 2;

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per litre/minute of flow.
float calibrationFactor = 0.05;  //aslinya 4.5

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;
//-------------------

byte airkosong[8]=
{
  B00100, 
  B00100, 
  B01010, 
  B01010, 
  B10001, 
  B10001, 
  B10001, 
  B01110
};//icon for water droplet

byte airpenuh[8]=
{
  B00100, 
  B00100, 
  B01110, 
  B01110, 
  B11111, 
  B11111, 
  B11111, 
  B01110
};//icon for water droplet

int debitawal = 100;
 
void print_time(unsigned long time_millis);

void setup()
{
  lcd.begin(); // initialize the lcd
  lcd.backlight();
  lcd.clear();

  lcd.createChar(1, airkosong);
  lcd.createChar(2, airpenuh);
    
  Serial.begin(9600);
  
  pinMode(LED_PIN, OUTPUT);

  pinMode(buzzer, OUTPUT); // Set buzzer - pin 11 as an output

  //-------------------

  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;

  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);

  //-------------------

  pinMode(motorpin1, OUTPUT);
  pinMode(motorpin2, OUTPUT);

  digitalWrite(motorpin1, LOW);
  digitalWrite(motorpin2, LOW);

  lcd.setCursor(0,0); // baris ke 1
  lcd.print("  PDAM PRABAYAR ");
  delay(2000);
  lcd.setCursor(0,0); // baris ke 1
  lcd.print("                ");
  lcd.setCursor(0,1); // baris ke 2
  lcd.print("                ");
  
}

void loop()
{
  // lcd.setCursor(0,0); // baris ke 1
  // lcd.print("  PDAM PRABAYAR ");
 
  if(millis() >= time_1 + INTERVAL_MESSAGE1)
  {
    time_1 +=INTERVAL_MESSAGE1;
    print_time(time_1);
    // Serial.println("I'm message number one!");
    digitalWrite(LED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  }
   
  if(millis() >= time_2 + INTERVAL_MESSAGE2)
  {
    time_2 +=INTERVAL_MESSAGE2;
    print_time(time_2);
    //Serial.println("Hello, I'm the second message.");
    digitalWrite(LED_PIN, LOW);    // turn the LED off by making the voltage LOW
  }


   /*
  digitalWrite(LED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_PIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second

  // key pad ga bisa pake DELAY
  */

  char key = keypad.getKey();
  
  //kode hanya 16 digit
  if (key) 
  {
    lcd.setCursor(cursorColumn, 1); // move cursor to   (cursorColumn, 0)
    
    lcd.print(key);                 // print key at (cursorColumn, 0)

    if(key!='#')  //&'*'&'A'&'B'&'C'&'D'
    {
      token += key;
    }
   
    cursorColumn++;                 // move cursor to next position
    if(cursorColumn == 18) 
    {                               // if reaching limit, clear LCD
      lcd.clear();
      cursorColumn = 0;
      token="";
    }
  }

  //----------------------

  if(key=='#')
  {
    Serial.println(token);
   // int tokenint = token.toInt();
   // Serial.println (tokenint);

  //----------------------------------------
  //Dummy token

  /*
  Rp 10k : 2817 Liter : " 4337 1283 5612 7889 "
  Rp 25k : 7042 Liter : " 9288 3121 8264 5749 "
  Rp 50k : 13819 L : " 3172 3286 9172 3232 "
  Rp 75k : 19138 Liter : " 4521 3287 4299 7481 "
  Rp 100k : 24457 Liter : " 4317 4139 8989 1232 "
  */

  if (token == "4337128356127889")
  {
    debitawal = debitawal+2817 ;
    token="";
    lcd.setCursor(0, 1);
    lcd.print("   Voucher 10K  "); 
    delay(2000);
    lcd.setCursor(0, 1);
    lcd.print("Kuota Air 2817 L"); 
    delay(2000);
    lcd.setCursor(0, 1);
    lcd.print("                "); 
    cursorColumn = 0;
  }

  else if (token == "9288312182645749")
  {
    debitawal = debitawal+7042;
    token="";
    lcd.setCursor(0, 1);
    lcd.print("   Voucher 25K  "); 
    delay(2000);
    lcd.setCursor(0, 1);
    lcd.print("Kuota Air 7042 L"); 
    delay(2000);
    lcd.setCursor(0, 1);
    lcd.print("                "); 
    cursorColumn = 0;
  }

  else if (token == "3172328691723232")
  {
    debitawal = debitawal+13819 ;
    token="";
    lcd.setCursor(0, 1);
    lcd.print("   Voucher 50K  "); 
    delay(2000);
    lcd.setCursor(0, 1);
    lcd.print("                "); 
    cursorColumn = 0;
  }

  else if (token == "4521328742997481")
  {
    debitawal = debitawal+19138;
    token="";
    lcd.setCursor(0, 1);
    lcd.print("   Voucher 75K  "); 
    delay(2000);
    lcd.setCursor(0, 1);
    lcd.print("                "); 
    cursorColumn = 0; 
  }

  else if (token == "4317413989891232")
  {
    debitawal = debitawal + 24457 ;
    token="";
    lcd.setCursor(0, 1);
    lcd.print("  Voucher 100K  "); 
    delay(2000);
    lcd.setCursor(0, 1);
    lcd.print("                "); 
    cursorColumn = 0;
  }
  //-----------------------

  else
  {
    token="";
    lcd.setCursor(0, 1);
    lcd.print("  Voucher Salah "); 
    delay(1000);
    lcd.setCursor(0, 1);
    lcd.print("                "); 
    cursorColumn = 0;
  }
  }
  
//---------------------

  if(key=='A')
  {
    token="";
    lcd.setCursor(0, 1);
    lcd.print("                "); 
    cursorColumn = 0;
    //--------------------------------------Kran Buka
    digitalWrite(motorpin1, HIGH);
    digitalWrite(motorpin2, LOW);
    delay(8000);  //diperlukan 10 detik

    digitalWrite(motorpin1, LOW);
    digitalWrite(motorpin2, LOW);
    delay(1000);
  }

  //------------------------------------
  
  if(key=='B')
  {

   token="";
   lcd.setCursor(0, 1);
   lcd.print("                "); 
   cursorColumn = 0;
    
  //------------------------------kran tutup
  digitalWrite(motorpin1, LOW);
  digitalWrite(motorpin2, HIGH);
  delay(8000);  //diperlukan 10 detik

  digitalWrite(motorpin1, LOW);
  digitalWrite(motorpin2, LOW);
  delay(1000);
  }

//------------------------------------------------------------------

  if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(sensorInterrupt);
        
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    
    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    oldTime = millis();
    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;
    
  
    totalMilliLitres += flowMilliLitres;    // Add the millilitres passed in this second to the cumulative total
      
    unsigned int frac;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");        
    Serial.print(totalMilliLitres);
    Serial.print("mL"); 
    Serial.print("\t");       // Print tab space
    Serial.print(totalMilliLitres/1000);
    Serial.println("L");
    
    pulseCount = 0;    // Reset the pulse counter so we can start incrementing again    

    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);    // Enable the interrupt again now that we've finished sending output


  lcd.setCursor(9,0);
  lcd.print(char(1));
  lcd.setCursor(10,0); // baris ke 1
  lcd.print("     ");
  lcd.setCursor(10,0); // baris ke 1
  lcd.print(totalMilliLitres/1000);
  lcd.setCursor(15,0); // baris ke 1
  lcd.print("L");

    
 int sisa = debitawal-(totalMilliLitres/1000);

  lcd.setCursor(0,0);
  lcd.print(char(2));
  lcd.setCursor(1,0); // baris ke 1
  lcd.print("     ");
  lcd.setCursor(1,0); // baris ke 1
  lcd.print(sisa);
  lcd.setCursor(6,0); // baris ke 1
  lcd.print("L");

//-----------------------------------

 if (sisa <= 50 && sisa >= 10)
{


 if (millis()-lastPeriodStart>=periodDuration)
  {
    lastPeriodStart+=periodDuration;
    tone(buzzer,550, onDuration); // play 550 Hz tone in background for 'onDuration'
  }


/*
    if(millis() >= time_1 + INTERVAL_MESSAGE1)
  {
    time_1 +=INTERVAL_MESSAGE1;
    print_time(time_1);
    // Serial.println("I'm message number one!");
    digitalWrite(buzzer, LOW);   // turn the LED on (HIGH is the voltage level)
  }
   
  if(millis() >= time_2 + INTERVAL_MESSAGE2)
  {
    time_2 +=INTERVAL_MESSAGE2;
    print_time(time_2);
    //Serial.println("Hello, I'm the second message.");
    digitalWrite(buzzer, HIGH);    // turn the LED off by making the voltage LOW
  }
  
 
    if(millis() >= time_1 + INTERVAL_MESSAGE1)
  {
    time_1 +=INTERVAL_MESSAGE1;
    print_time(time_1);
    // Serial.println("I'm message number one!");
    digitalWrite(buzzer, LOW);   // turn the LED on (HIGH is the voltage level)
  }
   
  if(millis() >= time_2 + INTERVAL_MESSAGE2)
  {
    time_2 +=INTERVAL_MESSAGE2;
    print_time(time_2);
    //Serial.println("Hello, I'm the second message.");
    digitalWrite(buzzer, HIGH);    // turn the LED off by making the voltage LOW
  }

  
    if(millis() >= time_1 + INTERVAL_MESSAGE1)
  {
    time_1 +=INTERVAL_MESSAGE1;
    print_time(time_1);
    // Serial.println("I'm message number one!");
    digitalWrite(buzzer, LOW);   // turn the LED on (HIGH is the voltage level)
  }
   
  if(millis() >= time_2 + INTERVAL_MESSAGE2)
  {
    time_2 +=INTERVAL_MESSAGE2;
    print_time(time_2);
    //Serial.println("Hello, I'm the second message.");
    digitalWrite(buzzer, HIGH);    // turn the LED off by making the voltage LOW
  }
  */
  
  
  }


//if sisa

if (sisa > 0 && statusKran == 1)
{
  //------------------------------kran buka
  digitalWrite(motorpin1, HIGH);
  digitalWrite(motorpin2, LOW);
  delay(8000);  //diperlukan 10 detik

  digitalWrite(motorpin1, LOW);
  digitalWrite(motorpin2, LOW);
  delay(1000);
  
  statusKran = 0;
  }


if (sisa <= 0 && statusKran == 0)
{
  lcd.setCursor(0, 0);
  lcd.print("                "); 
  lcd.setCursor(0, 1);
  lcd.print("                ");

  lcd.setCursor(0, 0);
  lcd.print("   KUOTA AIR    "); 
  lcd.setCursor(0, 1);
  lcd.print("     HABIS      ");

  //------------------------------kran tutup
  digitalWrite(motorpin1, LOW);
  digitalWrite(motorpin2, HIGH);
  delay(8000);  //diperlukan 10 detik

  digitalWrite(motorpin1, LOW);
  digitalWrite(motorpin2, LOW);
  delay(1000);
  
 statusKran = 1;

   lcd.setCursor(0, 0);
  lcd.print("                "); 
  lcd.setCursor(0, 1);
  lcd.print("                ");
  
}

else
{
  
}


}

//------------------------------------------------------------------

}                                                                                   

void print_time(unsigned long time_millis)
{
//    Serial.print("Time: ");
  //  Serial.print(time_millis/1000);
 //  Serial.print("s - ");
}


void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}
