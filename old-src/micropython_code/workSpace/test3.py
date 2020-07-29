import music


def main():
    wav_player = music.Music()
    wav_player.SDcard_init()
    while True:
        if wav_player.index is not wav_player.last_index:
            wav_player.Display_list()
            wav_player.index = wav_player.last_index
            
        wav_player.Keyboard_scanf()
        
        if wav_player.play_status == 1:
            wav_player.Audio_init()
            wav_player.Audio_play()
            wav_player.Audio_close()


if __name__ == '__main__':
  main()












