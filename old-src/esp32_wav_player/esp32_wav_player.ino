#include "I2S.h"
#include <FS.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define MAKEPYTHON_ESP32_SDA 4
#define MAKEPYTHON_ESP32_SCL 5

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int Pin_up = 39;
const int Pin_down = 36;
const int Pin_pause = 35;
const int Pin_mute = 34;

const int offset = 0x2C;
char data[800];
char null_data[4] = {0, 0, 0, 0};
long i = 0;
String music_list[30];
int music_num = 0;
int music_index = 0;

void setup()
{
  pinMode(Pin_down, INPUT_PULLUP);
  pinMode(Pin_up, INPUT_PULLUP);
  pinMode(Pin_pause, INPUT_PULLUP);
  pinMode(Pin_mute, OUTPUT);
  digitalWrite(Pin_mute, LOW);

  Serial.begin(115200);
  Wire.begin(MAKEPYTHON_ESP32_SDA, MAKEPYTHON_ESP32_SCL);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  display.clearDisplay();
  logoshow();

  SPI.begin(18, 19, 23, 5);
  if (!SD.begin(22, SPI))
  {
    Serial.println("Card Mount Failed");
    return;
  }
  //listDir(SD, "/", 0);
  music_num = get_wav_list(SD, "/", 0, music_list);
  Serial.print("Wav count:");
  Serial.println(music_num);

  music_num--;
  Serial.println("All wav:");
  for (int i = 0; i < music_num; i++)
  {
    Serial.println(music_list[i]);
  }
}

void loop()
{
  Serial.print("Wav index:");
  Serial.println(music_index);
  String wav_name = music_list[music_index];
  File file = SD.open(wav_name); // 44100Hz, 16bit, linear PCM

  file.seek(22);
  int ch = 2;
  long int music_time = file.size() / 160000;

  Serial.print(wav_name + "  ");
  Serial.println("OPEN FILE");

  display.clearDisplay();

  display.setTextSize(1);              // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);             // Start at top-left corner
  display.println(wav_name);
  display.setCursor(0, 12);
  display.println("44100Hz");
  display.setCursor(0, 24);
  display.print(ch);
  display.println(" Channel");
  display.setCursor(0, 36);
  display.print(music_time);
  display.println(" seconds");
  display.display();
  delay(2000);

  file.seek(offset);

  I2S_Init();
  Serial.println("I2S");

  while (file.readBytes(data, sizeof(data)))
  {

    if (digitalRead(Pin_down) == 0)
    {
      Serial.println("Pin_down");
      if (music_index < music_num)
        music_index++;
      else
        music_index = 0;
      delay(100);
      break;
    }
    if (digitalRead(Pin_up) == 0)
    {
      Serial.println("Pin_up");
      if (music_index == 0)
        music_index = music_num;
      else
        music_index--;
      delay(100);
      break;
    }
    if (digitalRead(Pin_pause) == 0)
    {
      Serial.println("Pin_pause");
      digitalWrite(Pin_mute, HIGH);
      for (int i = 0; i < sizeof(data); ++i)
        data[i] = 0; // to prevent buzzing
      for (int i = 0; i < 5; ++i)
        I2S_Write(data, sizeof(data));
      delay(1000);
      while (1)
      {
        if (digitalRead(Pin_pause) == 0)
        {
          Serial.println("Pin_pause_play");
          delay(1000);
          digitalWrite(Pin_mute, LOW);
          break;
        }
      }
    }
    I2S_Write(data, sizeof(data));
  }
  file.close();
  for (int i = 0; i < sizeof(data); ++i)
    data[i] = 0; // to prevent buzzing
  for (int i = 0; i < 5; ++i)
    I2S_Write(data, sizeof(data));

  Serial.println("over");
}

int get_wav_list(fs::FS &fs, const char *dirname, uint8_t levels, String wavlist[30])
{
  Serial.printf("Listing directory: %s\n", dirname);
  int i = 0;

  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("Failed to open directory");
    return i;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return i;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
    }
    else
    {
      String temp = file.name();
      if (temp.endsWith(".wav"))
      {
        wavlist[i] = temp;
        i++;
      }
    }
    file = root.openNextFile();
  }
  return i;
}

void mydisplay(int wav_index)
{
  display.clearDisplay();

  display.setTextSize(1);              // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);             // Start at top-left corner
  display.println(music_list[wav_index]);
  display.setCursor(0, 12);
  display.println("44100Hz");
  display.setCursor(0, 24);
  display.println("2 Channel");
  display.display();
  delay(2000);
}

void logoshow(void)
{
  display.clearDisplay();

  display.setTextSize(2);              // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);             // Start at top-left corner
  display.println(F("MakePython"));
  display.setCursor(0, 20); // Start at top-left corner
  display.println(F("Audio"));
  display.setCursor(0, 40); // Start at top-left corner
  display.println(F("WAVPLAYER"));
  display.display();
  delay(2000);
}