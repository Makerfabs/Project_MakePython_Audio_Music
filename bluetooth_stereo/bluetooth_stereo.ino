#include <btAudio.h>
//#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "time.h"
#include "makerfabs_pin.h"
#include <WiFi.h>

//SSD1306
#define MAKEPYTHON_ESP32_SDA 4
#define MAKEPYTHON_ESP32_SCL 5
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Button
const int Pin_vol_up = 39;
const int Pin_vol_down = 36;
const int Pin_mute = 35;

const int Pin_previous = 15;
const int Pin_pause = 33;
const int Pin_next = 2;

//WIFI
const char *ssid = "Tenda_195";
const char *password = "xbw941024";

//NTP time
//const char* ntpServer = "pool.ntp.org";
const char *ntpServer = "120.25.108.11";
const long gmtOffset_sec = 8 * 60 * 60; //China+8
const int daylightOffset_sec = 0;

/*
String clock_time = "20:14:0";
String clock_time2 = "20:16:0";
*/

String clock_time = "07:45:0";
String clock_time2 = "08:00:0";


struct tm timeinfo;

// Sets the name of the audio device
btAudio audio = btAudio("ESP_Speaker");

void setup()
{

    //IO mode init
    pinMode(Pin_vol_up, INPUT_PULLUP);
    pinMode(Pin_vol_down, INPUT_PULLUP);
    pinMode(Pin_mute, INPUT_PULLUP);
    pinMode(Pin_previous, INPUT_PULLUP);
    pinMode(Pin_pause, INPUT_PULLUP);
    pinMode(Pin_next, INPUT_PULLUP);

    Serial.begin(115200);

    Wire.begin(MAKEPYTHON_ESP32_SDA, MAKEPYTHON_ESP32_SCL);
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    { // Address 0x3D for 128x64
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }

    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    display.display();
    display.setRotation(2);
    logoshow();

    //connect to WiFi
    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" CONNECTED");
    lcd_text("Wifi OK");

    //init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.println("Alread get npt time.");
    printLocalTime();
    lcd_text("NPT FLESH");

    //disconnect WiFi as it's no longer needed
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    // streams audio data to the ESP32
    audio.begin();

    //  outputs the received data to an I2S DAC https://www.adafruit.com/product/3678
    int bck = MP_AUDIO_I2S_BCLK;
    int ws = MP_AUDIO_I2S_LRC;
    int dout = MP_AUDIO_I2S_DOUT;
    audio.I2S(bck, dout, ws);
}

unsigned long button_time = 0;
unsigned long run_time = 0;
uint alarm_flag = 0;
int16_t fy[2] = {10000, 60000};
size_t i2s_bytes_write = 0;

void loop()
{
    if (millis() - button_time > 1200)
    {
        printLocalTime();
        if (alarm_flag == 0)
        {
            if (showtime() != 0)
            {
                run_time = millis();
                alarm_flag = 1;
                display.setCursor(0, 24); // Start at top-left corner
                display.println("ALARM!!!!!");
                display.display();
                delay(1000);
                button_time = millis();
            }
        }
    }
    
    while (alarm_flag)
    {
        for (int i = 0; i < 100; i++)
        {
            i2s_write(I2S_NUM_0, fy, 1, &i2s_bytes_write, 100);
            fy[0]++;
            fy[1]++;
        }
        Serial.println("alarm");
        //Button logic
        if (digitalRead(Pin_mute) == 0)
        {
            Serial.println("Pin_mute");
            alarm_flag = 0;
            button_time = millis();
        }
        if((millis() - run_time) > 60000)
        {
          Serial.println("Alarm over");
          alarm_flag = 0;
        }
    }

    if (digitalRead(Pin_pause) == 0)
    {
        showalarm();
    }
}

void logoshow()
{
    display.clearDisplay();

    display.setTextSize(2);              // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 0);             // Start at top-left corner
    display.println("MakePython");
    display.setCursor(0, 20); // Start at top-left corner
    display.println("WAKEMASTER");
    display.display();
    delay(2000);
}

void lcd_text(String text)
{
    display.clearDisplay();
    display.setTextSize(2);  
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 0);             // Start at top-left corner
    display.println(text);
    display.display();
    delay(500);
}

void showalarm()
{
    display.clearDisplay();
    display.setTextSize(3);  
    display.setCursor(0, 0); // Start at top-left corner
    display.println(clock_time); 
    display.println(clock_time2); 
    display.display();
    delay(5000);
}

int showtime()
{
    int hour = timeinfo.tm_hour;
    int min = timeinfo.tm_min;
    int sec = timeinfo.tm_sec;
    char time_str[10];
    sprintf(time_str, "%02d:%02d:%02d", hour, min, sec);
    display.clearDisplay();
    display.setTextSize(4);  
    display.setCursor(0, 0); // Start at top-left corner
    display.println(time_str);
    display.display();
    String temp = (String)time_str;
    if (temp.startsWith(clock_time))
    {
        return 1;
    }
    if (temp.startsWith(clock_time2))
    {
        return 2;
    }
    return 0;
}

void printLocalTime()
{

    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        display.clearDisplay();
        display.setTextSize(4);  
        display.setCursor(0, 0); // Start at top-left corner
        display.println("PLEASE RESTART");
        display.display();
        while(1);
        return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}
