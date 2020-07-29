import os, sdcard
from machine import Pin, SPI, reset

def init_SD():
    spi = SPI(2, baudrate = 10000000, polarity = 0, phase = 0, bits = 8, firstbit = SPI.MSB,
        sck = Pin(18, Pin.OUT, Pin.PULL_DOWN),
        mosi = Pin(23, Pin.OUT, Pin.PULL_UP),
        miso = Pin(19, Pin.IN, Pin.PULL_UP))

    spi.init()  # Ensure right baudrate
    sd = sdcard.SDCard(spi, Pin(22))  # Compatible with PCB
    return sd

def mount(sd):
    vfs = os.VfsFat(sd)
    os.mount(vfs, "/sd")
    print("Already mount")

def umount():
    os.umount("/sd")
    print("Already umount")

def get_wav_list(sd):    
    
    file_list = os.listdir("/sd")
    print(file_list)
    wav_list = []
    for name in file_list:
        if name[-4:] is ".wav":
            wav_list.append(name)

    print(wav_list)
    return wav_list