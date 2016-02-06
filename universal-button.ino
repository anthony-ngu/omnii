#include "application.h"
#include "SparkFunMAX17043/SparkFunMAX17043.h"
#include "SparkFunMicroOLED/SparkFunMicroOLED.h"
#include "math.h"
#include "neopixel/neopixel.h"
#include "JSMNSpark/JSMNSpark.h" // for more info on JSMN check out @ https://github.com/zserge/jsmn

SYSTEM_MODE(AUTOMATIC);

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_PIN D5
#define PIXEL_COUNT 24
#define PIXEL_TYPE WS2812B

void doEncoderA();
void doEncoderB();

// Button Globals
#define ENC_BUTTON_PIN D4 //push button switch
int buttonState;             // the current reading from the input pin
int lastButtonState = HIGH;   // the previous reading from the input pin
long lastButtonDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 100;    // the debounce time; increase if the output flickers

// Neopixel Globals
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

// Setup Globals
String jsonString = NULL; // the established JSON string
jsmntok_t jsmnTokens[100]; //JSMN Tokens (a conversion of JSON)
int i = 0;
String beginString = "begin--";
String endString = "--end";

// Rotary Encoder Globals
#define ENC_A A0
#define ENC_B A1
volatile bool A_set = false;
volatile bool B_set = false;
volatile int encoderPos = 0;
int prevPos = 0;
int value = 0;
long lastEncoderDebounceTime = 0;

// OLED Globals
MicroOLED oled;

// Battery Monitoring Globals
double voltage = 0; // Variable to keep track of LiPo voltage
double soc = 0; // Variable to keep track of LiPo state-of-charge (SOC)
bool alert; // Variable to keep track of whether alert has been triggered

void setup()
{
    bool success = Particle.function("setupStruct", setupStructure);
    
    // Encoder setup
    pinMode(ENC_A, INPUT_PULLUP);
    pinMode(ENC_B, INPUT_PULLUP);
    attachInterrupt(ENC_A, doEncoderA, CHANGE);
    attachInterrupt(ENC_B, doEncoderB, CHANGE);

    // Button setup  
    pinMode(ENC_BUTTON_PIN, INPUT_PULLUP);
    
    // Neopixel Ring Setup
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    strip.setBrightness(10); // Stops the LEDs from being blinding
    
    // Set up the MAX17043 LiPo fuel gauge:
    lipo.begin(); // Initialize the MAX17043 LiPo fuel gauge
    // Quick start restarts the MAX17043 in hopes of getting a more accurate
    // guess for the SOC.
    lipo.quickStart();
    
    // OLED Setup
    oled.begin();    // Initialize the OLED
    oled.clear(ALL); // Clear the display's internal memory
    oled.display();  // Display what's in the buffer (splashscreen)
    delay(1000);     // Delay 1000 ms
    oled.clear(PAGE); // Clear the buffer.
    
    // Info Setup
    jsonString = ""; // the established JSON string
    Particle.publish("setupInit");
}

void loop()
{
    // NeoPixel Ring
    // goIntoStandby();
    strip.setPixelColor(1, strip.Color(0, 0, 255));
    strip.setPixelColor(0, strip.Color(0, 255, 0));
    strip.setPixelColor(23, strip.Color(255, 0, 0));
    strip.show();
    
    // Encoder + Button
    if (prevPos != encoderPos) {
    //     lastEncoderDebounceTime = millis();
    //     if ((millis() - lastEncoderDebounceTime) > debounceDelay) {
            prevPos = encoderPos;
            char str[63];
            sprintf(str, "%d", encoderPos);
            Particle.publish("encoderPos",str);
        // }
    }
    int reading = digitalRead(ENC_BUTTON_PIN);
    // If the switch changed, due to noise or pressing:
    if (reading != lastButtonState) {
      // reset the debouncing timer
      lastButtonDebounceTime = millis();
    }
    if ((millis() - lastButtonDebounceTime) > debounceDelay) {
      // whatever the reading is at, it's been there for longer
      // than the debounce delay, so take it as the actual current state:
      // if the button state has changed:
      if (reading != buttonState) {
        buttonState = reading;
        if (buttonState == LOW) {
          Particle.publish("button", "pressed");
        }
      }
    }
    lastButtonState = reading;
    
    // LiPo and Battery Display
    voltage = lipo.getVoltage(); // lipo.getVoltage() returns a voltage value (e.g. 3.93)
    soc = lipo.getSOC(); // lipo.getSOC() returns the estimated state of charge (e.g. 79%)
    char str[63];
    sprintf(str, "%.2f V\n%.1f %%", voltage, soc);
    oled.setFontType(1);  // Set font to type 1
    oled.clear(PAGE);     // Clear the page
    oled.setCursor(0, 0); // Set cursor to top-left
    // Print can be used to print a string to the screen:
    oled.print(str);
    oled.display();       // Refresh the display
}

void goIntoStandby()
{
    strip.setPixelColor(0, strip.Color(0, 0, 0));
    strip.show();
}

int setupStructure(String args){
    jsmn_parser parser; // creates the parser
    jsmn_init(&parser); // initializes the parser
    int tokenArraySize = 0;
    // Particle.publish("received", args);
    strip.setPixelColor(i++, strip.Color(255, 0, 0));
    strip.show();
    
    if(args.startsWith(beginString))
    {
        jsonString = args;
    }else{
        jsonString += args;
        if(args.endsWith(endString))
        {
            String jsonStringData = jsonString.substring(beginString.length(), jsonString.length() - endString.length());
            // Particle.publish("received","endReceived");
            // Particle.publish("receivedString", jsonStringData);
            // parse it and set up the universal button as such
            
            tokenArraySize = jsmn_parse(&parser, jsonStringData, jsmnTokens, 100); // hands back the required token allocation size
            // char str[63];
            // sprintf(str, "tokenArraySize: %d", tokenArraySize);
            // Particle.publish("parser", str);
        	    
        	if (tokenArraySize >= 0)
        	{
        	    // parsed successfuly
        	    for (int i = 0; i < 100; i++) 
        	    {
                	switch(jsmnTokens[i].type){
                	    case JSMN_OBJECT:
                	        // Particle.publish("parser", "object");
                	        break;
                	    case JSMN_ARRAY:
                	        // Particle.publish("parser", "array");
                	        break;
                	    case JSMN_STRING:
                	        // Particle.publish("parser", "string");
                	        break;
                	    case JSMN_PRIMITIVE:
                	        // Particle.publish("parser", "primitive");
                	        //  't', 'f' - boolean
                            //  'n' - null
                            //  '-', '0'..'9' - integer
                	        break;
                	    default:
                	        // undefined
                	        return -1;
                	        break;
                	}
                }
            }else {
        	    // parse failed
        	    Particle.publish("parser", "failed");
        	    return -1;
        	}
        }   
    }
    return 1;
}

void doEncoderA(){
  if( digitalRead(ENC_A) != A_set ) {  // debounce once more
    A_set = !A_set;
    // adjust counter + if A leads B
    if ( A_set && !B_set ) 
      encoderPos += 1;
  }
}

// Interrupt on B changing state, same as A above
void doEncoderB(){
   if( digitalRead(ENC_B) != B_set ) {
    B_set = !B_set;
    //  adjust counter - 1 if B leads A
    if( B_set && !A_set ) 
      encoderPos -= 1;
  }
}

// Center and print a small title
// This function is quick and dirty. Only works for titles one
// line long.
void printTitle(String title, int font)
{
  int middleX = oled.getLCDWidth() / 2;
  int middleY = oled.getLCDHeight() / 2;

  oled.clear(PAGE);
  oled.setFontType(font);
  // Try to set the cursor in the middle of the screen
  oled.setCursor(middleX - (oled.getFontWidth() * (title.length()/2)),
                 middleY - (oled.getFontWidth() / 2));
  // Print the title:
  oled.print(title);
  oled.display();
  delay(1500);
  oled.clear(PAGE);
}

