#include "I2S.h"
#include <FS.h>
#include <SD.h>

const int offset = 0x2C;
char data[800];
long i = 0;

void setup()
{
  Serial.begin(115200);
  SPI.begin(18, 19, 23, 5);
  if (!SD.begin(22, SPI))
  {
    Serial.println("Card Mount Failed");
    return;
  }

  File file = SD.open("/sound.wav"); // 44100Hz, 16bit, linear PCM
  Serial.println("OPEN FILE");

  file.seek(22);
  int ch = file.read();
  file.seek(offset);
  I2S_Init();
  Serial.println("I2S");

  while (file.readBytes(data, sizeof(data)))
  {
      I2S_Write(data, sizeof(data));
  }
  file.close();
  for (int i = 0; i < sizeof(data); ++i)
    data[i] = 0; // to prevent buzzing
  for (int i = 0; i < 5; ++i)
    I2S_Write(data, sizeof(data));

  Serial.println("over");
}

void loop()
{
}
