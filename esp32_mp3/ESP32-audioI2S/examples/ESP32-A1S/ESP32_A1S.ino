// found this sketch on https://www.mikrocontroller.net/topic/474383
// thanks to Michael U. (amiga)

#include "Arduino.h"
#include "WiFi.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include "Wire.h"
#include "AC101.h" //https://github.com/Yveaux/AC101
#include "Audio.h" //https://github.com/schreibfaul1/ESP32-audioI2S


// SPI GPIOs
#define SD_CS         13
#define SPI_MOSI      15
#define SPI_MISO       2
#define SPI_SCK       14

// I2S GPIOs, the names refer on AC101
#define I2S_DSIN      25
#define I2S_BCLK      27
#define I2S_LRC       26
#define I2S_MCLK       0
#define I2S_DOUT      35

// I2C GPIOs
#define IIC_CLK       32
#define IIC_DATA      33

// Tasten
// #define BUTTON_2_PIN 13             // shared mit SPI_CS
#define BUTTON_3_PIN  19
#define BUTTON_4_PIN  23
#define BUTTON_5_PIN  18               // Stop
#define BUTTON_6_PIN   5               // Play

// VerstÃ¤rker enable
#define GPIO_PA_EN    21

//Switch S1: 1-OFF, 2-ON, 3-ON, 4-OFF, 5-OFF

String ssid =     "xxxxxxxxx";
String password = "xxxxxxxxx";

static AC101 ac;
int volume = 40;                                  // 0...100

Audio audio;

//#####################################################################

void setup()
{
  Serial.begin(115200);
  Serial.println("\r\nReset");
  Serial.printf_P(PSTR("Free mem=%d\n"), ESP.getFreeHeap());

  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(1000000);

  SD.begin(SD_CS);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }

  Serial.printf_P(PSTR("Connected\r\nRSSI: "));
  Serial.print(WiFi.RSSI());
  Serial.print(" IP: ");
  Serial.println(WiFi.localIP());

    Serial.printf("Connect to AC101 codec... ");
    while (not ac.begin(IIC_DATA, IIC_CLK))
    {
        Serial.printf("Failed!\n");
        delay(1000);
    }
    Serial.printf("OK\n");

    ac.SetVolumeSpeaker(volume);
    ac.SetVolumeHeadphone(volume);
//  ac.DumpRegisters();

    // Enable amplifier
    pinMode(GPIO_PA_EN, OUTPUT);
    digitalWrite(GPIO_PA_EN, HIGH);

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DSIN);
  audio.setVolume(10); // 0...21

  audio.connecttoFS(SD, "/320k_test.mp3");
//  audio.connecttoSD("/Banana Boat Song - Harry Belafonte.mp3");
//  audio.connecttohost("http://mp3channels.webradio.antenne.de:80/oldies-but-goldies");
//  audio.connecttohost("http://dg-rbb-http-dus-dtag-cdn.cast.addradio.de/rbb/antennebrandenburg/live/mp3/128/stream.mp3");
//  audio.connecttospeech("Wenn die Hunde schlafen, kann der Wolf gut Schafe stehlen.", "de");

}

//-----------------------------------------------------------------------

void loop()
{
    audio.loop();
}

// optional
void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
}
void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}
void audio_showstreaminfo(const char *info){
    Serial.print("streaminfo  ");Serial.println(info);
}
void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}
void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}
void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}
void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}
void audio_eof_speech(const char *info){
    Serial.print("eof_speech  ");Serial.println(info);
}
