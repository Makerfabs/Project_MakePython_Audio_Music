import sdread
import display_ssd1306_i2c
from machine import Pin, SPI, I2S
import time

class Music():
    def __init__(self):
        #obj
        self.Lcd = display_ssd1306_i2c.Display()
        self.Audio = None
        self.SDcard = None
        self.SPI = None

        #GPIO
        self.Pin_up = Pin(39,Pin.IN,Pin.PULL_UP)
        self.Pin_down = Pin(36,Pin.IN,Pin.PULL_UP)
        self.Pin_play = Pin(35,Pin.IN,Pin.PULL_UP)
        self.bck_pin = Pin(26) 
        self.ws_pin = Pin(25)  
        self.sdout_pin = Pin(27)

        #var
        self.index = 0
        self.last_index = -1
        self.play_status = 0
        self.wav_list = None
        self.wav_name = None

        pass

    def Audio_init(self):
        I2S_dataformat = I2S.B16
        I2S_channel = I2S.ONLY_LEFT
        I2S_samplerate = 44100
        self.Audio = I2S(I2S.NUM0,
                            bck=self.bck_pin, ws=self.ws_pin, sdout=self.sdout_pin, 
                            standard=I2S.PHILIPS, 
                            mode=I2S.MASTER_TX,
                            dataformat=I2S_dataformat, 
                            channelformat=I2S_channel,
                            samplerate=I2S_samplerate,
                            dmacount=10, dmalen=512)

    def Audio_close(self):
        self.Audio.deinit()

    def Audio_play(self):
        wav_file = '/sd/{}'.format(self.wav_name)
        wav = open(wav_file,'rb')
  
        # advance to first byte of Data section in WAV file
        pos = wav.seek(44) 
      
        # allocate sample arrays
        #   memoryview used to reduce heap allocation in while loop
        wav_samples = bytearray(2048)
        wav_samples_mv = memoryview(wav_samples)
      
        print('Starting')
        # continuously read audio samples from the WAV file 
        # and write them to an I2S DAC
        while True:
            try:
                num_read = wav.readinto(wav_samples_mv)

                """
                for i in range(len(wav_samples_mv)):
                  wav_samples_mv[i] = int(wav_samples_mv[i]/2)
                  #wav_samples_mv[i] =wav_samples_mv[i] + int ((128 - wav_samples_mv[i]) / 2)
                  pass
                  """

                num_written = 0
                # end of WAV file?
                if num_read == 0:
                    # advance to first byte of Data section
                    pos = wav.seek(44) 
                else:
                    # loop until all samples are written to the I2S peripheral
                    while num_written < num_read:
                        num_written += self.Audio.write(wav_samples_mv[num_written:num_read], timeout=0)
                        if self.Pin_play.value() == 0:
                            wav.close()
                            self.Audio.deinit()
                            self.play_status = 0
                            time.sleep(0.5)
                            print('Hand Out')
                            return
            except (KeyboardInterrupt, Exception) as e:
                print('caught exception {} {}'.format(type(e).__name__, e))
                break
            
        wav.close()
        self.Audio.deinit()
        print('Auto Out')

    def SDcard_init(self):
        self.SDcard = sdread.init_SD()
        sdread.mount(self.SDcard)
        self.wav_list = sdread.get_wav_list(self.SDcard)

    def SDcard_close(self):
        pass

    def Display_list(self):
        self.Lcd.clear()

        if self.index - 2 >= 0:
          self.Lcd.show_text(self.wav_list[self.index - 2],0,0,False,False)

        if self.index - 1 >= 0:
          self.Lcd.show_text(self.wav_list[self.index - 1],0,12,False,False)

        self.Lcd.show_text(self.wav_list[self.index],0,24,False,False)
      
        if self.index + 1 < len(self.wav_list):
          self.Lcd.show_text(self.wav_list[self.index + 1],0,36,False,False)

        if self.index + 2 < len(self.wav_list):
          self.Lcd.show_text(self.wav_list[self.index + 2],0,48,False,False)

        self.Lcd.display.line(0,22,127,22,1)
        self.Lcd.display.line(0,34,127,34,1)
        self.Lcd.show()

    def Keyboard_scanf(self):
        if self.Pin_up.value() == 0 :
          print("up")
          if self.index > 0:
            self.index -= 1
        if self.Pin_down.value() == 0 :
          print("down")
          if self.index < len(self.wav_list) - 1:
            self.index += 1
        if self.Pin_play.value() == 0 :
          print("play")
          self.play_status = 1
          self.wav_name = self.wav_list[self.index]
        time.sleep(0.5)
