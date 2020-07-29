
import sdread
import sdtest
import test2
import display_ssd1306_i2c
from machine import Pin
import time

Pin_up = Pin(39,Pin.IN,Pin.PULL_UP)
Pin_down = Pin(36,Pin.IN,Pin.PULL_UP)
Pin_play = Pin(35,Pin.IN,Pin.PULL_UP)

def music_list_show(lcd,wav_list,index):
  lcd.clear()

  if index - 2 >= 0:
    lcd.show_text(wav_list[index - 2],0,0,False,False)

  if index - 1 >= 0:
    lcd.show_text(wav_list[index - 1],0,12,False,False)

  lcd.show_text(wav_list[index],0,24,False,False)
  
  if index + 1 < len(wav_list):
    lcd.show_text(wav_list[index + 1],0,36,False,False)

  if index + 2 < len(wav_list):
    lcd.show_text(wav_list[index + 2],0,48,False,False)

  lcd.display.line(0,22,127,22,1)
  lcd.display.line(0,34,127,34,1)
  lcd.show()


def main():

  lcd = display_ssd1306_i2c.Display()

  sd = sdread.init_SD()
  sdread.mount(sd)
  wav_list = sdread.get_wav_list(sd)
  index = 0
  last_index = -1
  play_status = 0

  while True:
    pass
    if Pin_up.value() == 0 :
      print("up")
      if index > 0:
        index -= 1
    if Pin_down.value() == 0 :
      print("down")
      if index < len(wav_list) - 1:
        index += 1
    if Pin_play.value() == 0 :
      print("play")
      play_status = 1

    if index is not last_index:
      music_list_show(lcd,wav_list,index)
      last_index = index
      
    time.sleep(0.5)
    
    if play_status == 1:
      test2.music(Pin_play,wav_list[index])
      play_status = 0
      time.sleep(0.5)
    

  
  
  sdread.umount()

  #test2.music()
  pass

if __name__ == '__main__':
  main()











