#include "Arduino.h"
//#include "WiFiMulti.h"
#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//SD Card
#define SD_CS 22
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18

//Digital I/O used  //Makerfabs Audio V2.0
#define I2S_DOUT 27
#define I2S_BCLK 26
#define I2S_LRC 25

//SSD1306
#define MAKEPYTHON_ESP32_SDA 4
#define MAKEPYTHON_ESP32_SCL 5
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Button
const int Pin_vol_up = 39;
const int Pin_vol_down = 36;
const int Pin_mute = 35;

const int Pin_previous = 15;
const int Pin_pause = 33;
const int Pin_next = 2;

Audio audio;

/*
WiFiMulti wifiMulti;
String ssid = "Makerfabs";
String password = "20160704";
*/

struct Music_info
{
    String name;
    int length;
    int runtime;
    int volume;
    int status;
    int mute_volume;
} music_info = {"", 0, 0, 0, 0, 0};

String file_list[20];
int file_num = 0;
int file_index = 0;

void setup()
{
    //IO mode init
    pinMode(Pin_vol_up, INPUT_PULLUP);
    pinMode(Pin_vol_down, INPUT_PULLUP);
    pinMode(Pin_mute, INPUT_PULLUP);
    pinMode(Pin_previous, INPUT_PULLUP);
    pinMode(Pin_pause, INPUT_PULLUP);
    pinMode(Pin_next, INPUT_PULLUP);

    //Serial
    Serial.begin(115200);

    //LCD
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

    //SD(SPI)
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    SPI.setFrequency(1000000);
    if (!SD.begin(SD_CS, SPI))
    {
        Serial.println("Card Mount Failed");
        lcd_text("SD ERR");
        while (1)
            ;
    }
    else
    {
        lcd_text("SD OK");
    }

    //Read SD
    file_num = get_music_list(SD, "/", 0, file_list);
    Serial.print("Music file count:");
    Serial.println(file_num);
    Serial.println("All music:");
    for (int i = 0; i < file_num; i++)
    {
        Serial.println(file_list[i]);
    }

    //WiFi
    /*
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(ssid.c_str(), password.c_str());
    wifiMulti.run();
    if (WiFi.status() != WL_CONNECTED)
    {
        WiFi.disconnect(true);
        wifiMulti.run();
    }
    */

    //Audio(I2S)
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(21); // 0...21

    //audio.connecttoFS(SD, "/MoonlightBay.mp3"); //ChildhoodMemory.mp3  //MoonRiver.mp3 //320k_test.mp3
    //file_list[0] = "MoonlightBay.mp3";
    open_new_song(file_list[file_index]);
    print_song_time();
}

uint run_time = 0;
uint button_time = 0;

void loop()
{
    audio.loop();
    if (millis() - run_time > 1000)
    {
        run_time = millis();
        print_song_time();
        display_music();
    }

    if (millis() - button_time > 300)
    {
        //Button logic
        if (digitalRead(Pin_next) == 0)
        {
            Serial.println("Pin_next");
            if (file_index < file_num - 1)
                file_index++;
            else
                file_index = 0;
            open_new_song(file_list[file_index]);
            print_song_time();
            button_time = millis();
        }
        if (digitalRead(Pin_previous) == 0)
        {
            Serial.println("Pin_previous");
            if (file_index > 0)
                file_index--;
            else
                file_index = file_num - 1;
            open_new_song(file_list[file_index]);
            print_song_time();
            button_time = millis();
        }
        if (digitalRead(Pin_vol_up) == 0)
        {
            Serial.println("Pin_vol_up");
            if (music_info.volume < 21)
                music_info.volume++;
            audio.setVolume(music_info.volume);
            button_time = millis();
        }
        if (digitalRead(Pin_vol_down) == 0)
        {
            Serial.println("Pin_vol_down");
            if (music_info.volume > 0)
                music_info.volume--;
            audio.setVolume(music_info.volume);
            button_time = millis();
        }
        if (digitalRead(Pin_mute) == 0)
        {
            Serial.println("Pin_mute");
            if (music_info.volume != 0)
            {
                music_info.mute_volume = music_info.volume;
                music_info.volume = 0;
            }
            else
            {
                music_info.volume = music_info.mute_volume;
            }
            audio.setVolume(music_info.volume);
            button_time = millis();
        }
        if (digitalRead(Pin_pause) == 0)
        {
            Serial.println("Pin_pause");
            audio.pauseResume();
            button_time = millis();
        }
    }

    //串口控制切歌，音量
    if (Serial.available())
    {
        String r = Serial.readString();
        r.trim();
        if (r.length() > 5)
        {
            audio.stopSong();
            open_new_song(file_list[0]);
            print_song_info();
        }
        else
        {
            audio.setVolume(r.toInt());
        }
    }
}

void open_new_song(String filename)
{
    //去掉文件名的根目录"/"和文件后缀".mp3",".wav"
    music_info.name = filename.substring(1, filename.indexOf("."));
    audio.connecttoFS(SD, filename);
    music_info.runtime = audio.getAudioCurrentTime();
    music_info.length = audio.getAudioFileDuration();
    music_info.volume = audio.getVolume();
    music_info.status = 1;
    Serial.println("**********start a new sound************");
}

void display_music()
{
    int line_step = 24;
    int line = 0;
    char buff[20];
    ;
    sprintf(buff, "%d:%d", music_info.runtime, music_info.length);

    display.clearDisplay();

    display.setTextSize(2);              // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text

    display.setCursor(0, line); // Start at top-left corner
    display.println(music_info.name);
    line += line_step;

    display.setCursor(0, line);
    display.println(buff);
    line += line_step;

    sprintf(buff, "V:%d",music_info.volume);

    display.setCursor(0, line);
    display.println(buff);
    line += line_step;

    display.setCursor(0, line);
    display.println(music_info.status);
    line += line_step;

    display.display();
}

void logoshow(void)
{
    display.clearDisplay();

    display.setTextSize(2);              // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 0);             // Start at top-left corner
    display.println(F("MakePython"));
    display.setCursor(0, 20); // Start at top-left corner
    display.println(F("MUSIC"));
    display.setCursor(0, 40); // Start at top-left corner
    display.println(F("PLAYER V2"));
    display.display();
    delay(2000);
}

void lcd_text(String text)
{
    display.clearDisplay();

    display.setTextSize(2);              // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 0);             // Start at top-left corner
    display.println(text);
    display.display();
    delay(500);
}

void print_song_info()
{
    Serial.println("***********************************");
    Serial.println(audio.getFileSize());
    Serial.println(audio.getFilePos());
    Serial.println(audio.getSampleRate());
    Serial.println(audio.getBitsPerSample());
    Serial.println(audio.getChannels());
    Serial.println(audio.getVolume());
    Serial.println("***********************************");
}

//刷新歌曲时间
void print_song_time()
{
    //Serial.println(audio.getAudioCurrentTime());
    //Serial.println(audio.getAudioFileDuration());
    music_info.runtime = audio.getAudioCurrentTime();
    music_info.length = audio.getAudioFileDuration();
    music_info.volume = audio.getVolume();
}

int get_music_list(fs::FS &fs, const char *dirname, uint8_t levels, String wavlist[30])
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
            else if (temp.endsWith(".mp3"))
            {
                wavlist[i] = temp;
                i++;
            }
        }
        file = root.openNextFile();
    }
    return i;
}

//**********************************************
// optional
void audio_info(const char *info)
{
    Serial.print("info        ");
    Serial.println(info);
}
void audio_id3data(const char *info)
{ //id3 metadata
    Serial.print("id3data     ");
    Serial.println(info);
}

//歌曲结束逻辑
void audio_eof_mp3(const char *info)
{ //end of file
    Serial.print("eof_mp3     ");
    Serial.println(info);
    file_index++;
    if (file_index >= file_num)
    {
        file_index = 0;
    }
    open_new_song(file_list[file_index]);
}
void audio_showstation(const char *info)
{
    Serial.print("station     ");
    Serial.println(info);
}
void audio_showstreaminfo(const char *info)
{
    Serial.print("streaminfo  ");
    Serial.println(info);
}
void audio_showstreamtitle(const char *info)
{
    Serial.print("streamtitle ");
    Serial.println(info);
}
void audio_bitrate(const char *info)
{
    Serial.print("bitrate     ");
    Serial.println(info);
}
void audio_commercial(const char *info)
{ //duration in sec
    Serial.print("commercial  ");
    Serial.println(info);
}
void audio_icyurl(const char *info)
{ //homepage
    Serial.print("icyurl      ");
    Serial.println(info);
}
void audio_lasthost(const char *info)
{ //stream URL played
    Serial.print("lasthost    ");
    Serial.println(info);
}
void audio_eof_speech(const char *info)
{
    Serial.print("eof_speech  ");
    Serial.println(info);
}