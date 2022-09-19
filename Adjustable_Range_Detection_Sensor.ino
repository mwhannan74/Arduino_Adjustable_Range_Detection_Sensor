// This code is specific design for an Adafruit Metro M4 board that has a built in NeoPixel.
// Other than the NeoPixel, this code should easily be adaptable to any other Adruino board.
#include <Adafruit_NeoPixel.h>


//============================================================================================
// GLOBALS
//============================================================================================
static const int trigPin = 12;
static const int echoPin = 11;

static const int potPin = A0;

static const int ledPin = 13; // this is same pin
bool ledOn = false;

static const int neoPixPin = 40;
static const int neoPixCnt = 1;
Adafruit_NeoPixel strip(neoPixCnt, neoPixPin, NEO_GRB + NEO_KHZ800);

static const float CM2IN = 0.393701;
static const float IN2CM = 2.54;

static const float minRange_in = 10.0*CM2IN; // 2cm (0.787") stated --> 10cm (3.937") practical
static const float maxRange_in = 250.0*CM2IN; // 400cm (157.5") stated --> 250cm (98.5" ~= 8') practical


//============================================================================================
// SETUP
//============================================================================================
void setup() 
{
  Serial.begin (115200);
  
  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);
  pinMode(echoPin, INPUT);
  
  pinMode(ledPin, OUTPUT);

  // NeoPixel
  strip.begin();
  strip.clear();// Initialize all pixels to 'off'
  strip.show(); 
  float brightness = 0.1; // between 0-1
  strip.setBrightness( brightness*255 ); // this is not intended to be used in the loop() method
}

//============================================================================================
// LOOP
//============================================================================================
unsigned long blinkPeriod_ms = 1000;
unsigned long printPeriod_ms = 200;

uint32_t time_ms = millis();
uint32_t timeLED_ms = millis();
uint32_t timePrint_ms = millis();

float rangeFiltered_in = 0.0;
float Kn_bias = 0.2;
float Ko_bias = 1.0 - Kn_bias;

float potVal_scale_filtered = 0.0;
float Kn_pot = 0.1;
float Ko_pot = 1.0 - Kn_bias;

//float rangeTrigger_in = 12.0;

void loop() 
{  
  // get the current time in msec for this loop
  // Needs to go after the etime calc
  time_ms = millis();


  //------------------------------------------------------------
  // Read potentiometer to determine range
  int potVal_bits = analogRead(potPin); // 10 bit = 1024
  float potVal_scale = (float)potVal_bits / 1024; // (0.0-1.0)
  float potVal_volt = 3.3 * potVal_scale; 
  
  potVal_scale_filtered = Ko_pot*potVal_scale_filtered + Kn_pot*potVal_scale;
  float rangeTrigger_in = potVal_scale_filtered * maxRange_in;

//  Serial.print("potVal_bits:"); Serial.print(potVal_bits); Serial.print(", ");
//  Serial.print("potVal_scale:"); Serial.print(potVal_scale); Serial.print(", ");
//  Serial.print("potVal_volt:"); Serial.print(potVal_volt); Serial.print(", ");
//  Serial.print("potVal_scale_filtered:"); Serial.print(potVal_scale_filtered); Serial.print(", ");
//  Serial.print("rangeTrigger_in:"); Serial.println(rangeTrigger_in);
  

  //------------------------------------------------------------
  // Setting the Trig pin to high for 10µs causes the sensor to initiate an ultrasonic burst.
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the time the pulse is high using built in hardware capability
  unsigned long duration_us = pulseIn(echoPin, HIGH);

  // Calc distance based on duration of high pulse
  float range_in = (float)duration_us / 148.0;

  // Check limits
  int isRangeLimit = 0;
  if( range_in < minRange_in ) 
  {  
    isRangeLimit = -1;
    range_in = minRange_in;
  }
  else if( range_in > maxRange_in ) 
  {    
    isRangeLimit = 1;
    range_in = maxRange_in;
  }

  // simple averaging filter
  rangeFiltered_in = Ko_bias*rangeFiltered_in + Kn_bias*range_in;


  //------------------------------------------------------------
  bool isTriggered = false;
  if( rangeFiltered_in < rangeTrigger_in )
  {
    isTriggered = true;
    
    int red = 255;
    int green = 0;
    int blue = 0;
    strip.setPixelColor(0, red, green, blue);
    strip.show();
  }
  else if( rangeFiltered_in < 2.0*rangeTrigger_in )
  {
    int red = 255;
    int green = 255;
    int blue = 0;
    strip.setPixelColor(0, red, green, blue);
    strip.show();
  }
  else
  {
    strip.clear();
    strip.show();
  }


  //------------------------------------------------------------
  // Print to console
  if( time_ms - timePrint_ms > printPeriod_ms ) 
  {
    timePrint_ms = time_ms;
    //Serial.print("duration_us:"); Serial.print(duration_us); Serial.print(", ");     
    Serial.print("isRangeLimit:"); Serial.print(isRangeLimit); Serial.print(", ");
    Serial.print("range_in:"); Serial.print(range_in); Serial.print(", ");    
    Serial.print("rangeFiltered_in:"); Serial.print(rangeFiltered_in); Serial.print(", ");
    Serial.print("rangeTrigger_in:"); Serial.print(rangeTrigger_in); Serial.print(", ");
    Serial.print("isTriggered:"); Serial.println(isTriggered);
  }


  //------------------------------------------------------------
  // Blink LED
  if( time_ms - timeLED_ms > blinkPeriod_ms ) 
  {
    timeLED_ms = time_ms;
    ledOn = !ledOn;
    digitalWrite(ledPin, ledOn);
  }

  // Max rate of ultrasonic sensor is 40Hz (25ms)
  delay(50); // 50ms -> 20Hz
} 
