
from machine import I2S
from machine import Pin
import array
import gc
import speech


class audio:
    def __init__(self):
        super().__init__()
        bck_pin = Pin(26)
        ws_pin = Pin(25)
        sdout_pin = Pin(27)
        self.audio_out = I2S(I2S.NUM0,
                bck=bck_pin,
                ws=ws_pin,
                sdout=sdout_pin,
                standard=I2S.PHILIPS,
                mode=I2S.MASTER_TX,
                dataformat=I2S.B16,
                channelformat=I2S.ONLY_RIGHT,
                samplerate=16000,
                #samplerate=44100,
                dmacount=16,
                dmalen=512)

 
    def say(self,text):
        a = bytearray(4000*8)
        size = speech.say(text, a , pitch=80, speed=72, mouth=128, throat=128)
        self.audio_out.write(a[:size])
        del a
        gc.collect()
        
        
    def close(self):
        self.audio_out.deinit()
        
    def say_sentance(self,text):
        list = text.split()
        print(list)
        for word in list:
            print(word)
            self.say(word)





