# The MIT License (MIT)
# Copyright (c) 2020 Mike Teachman
# https://opensource.org/licenses/MIT

# Purpose:
# - read 16-bit audio samples from a mono formatted WAV file on SD card
# - write audio samples to an I2S amplifier or DAC module 
#
# Sample WAV files in wav_files folder:
#   "taunt-16k-16bits-mono.wav"
#   "taunt-16k-16bits-mono-12db.wav" (lower volume version)
#
# Hardware tested:
# - MAX98357A amplifier module (Adafruit I2S 3W Class D Amplifier Breakout)
# - PCM5102 stereo DAC module
#
# The WAV file will play continuously until a keyboard interrupt is detected or
# the ESP32 is reset
  
from machine import I2S
import sdcard
from machine import Pin
from machine import SPI
import uos

def music():
  #======= USER CONFIGURATION =======
  WAV_FILE = 'iveryhappy44100Hz.wav'
  SAMPLE_RATE_IN_HZ = 44100
  #======= USER CONFIGURATION =======
  
  bck_pin = Pin(26) 
  ws_pin = Pin(25)  
  sdout_pin = Pin(27)
  
  # channelformat settings:
  #     mono WAV:  channelformat=I2S.ONLY_LEFT
  audio_out = I2S(
      I2S.NUM0,
      bck=bck_pin, ws=ws_pin, sdout=sdout_pin, 
      standard=I2S.PHILIPS, 
      mode=I2S.MASTER_TX,
      dataformat=I2S.B16, 
      channelformat=I2S.ONLY_LEFT,
      samplerate=SAMPLE_RATE_IN_HZ,
      dmacount=10, dmalen=512)
  
  # configure SD card
  spi = SPI(2, baudrate = 10000000, polarity = 0, phase = 0, bits = 8, firstbit = SPI.MSB,
            sck = Pin(18, Pin.OUT, Pin.PULL_DOWN),
            mosi = Pin(23, Pin.OUT, Pin.PULL_UP),
            miso = Pin(19, Pin.IN, Pin.PULL_UP))
  
  
  spi.init()  # Ensure right baudrate
  sd = sdcard.SDCard(spi, Pin(22))  # Compatible with PCB
  uos.mount(sd, "/sd")
  wav_file = '/sd/{}'.format(WAV_FILE)
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
                  num_written += audio_out.write(wav_samples_mv[num_written:num_read], timeout=0)
      except (KeyboardInterrupt, Exception) as e:
          print('caught exception {} {}'.format(type(e).__name__, e))
          break
      
  wav.close()
  uos.umount("/sd")
  sd.deinit()
  audio_out.deinit()
  print('Done')





