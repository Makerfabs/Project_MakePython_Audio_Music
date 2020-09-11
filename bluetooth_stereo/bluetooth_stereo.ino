#include <btAudio.h>
#include "makerfabs_pin.h"

// Sets the name of the audio device
btAudio audio = btAudio("ESP_Speaker");

void setup() {
 
 // streams audio data to the ESP32   
 audio.begin();
 
 //  outputs the received data to an I2S DAC https://www.adafruit.com/product/3678
 int bck = MP_AUDIO_I2S_BCLK; 
 int ws = MP_AUDIO_I2S_LRC;
 int dout = MP_AUDIO_I2S_DOUT;
 audio.I2S(bck, dout, ws);
}

void loop() {

}