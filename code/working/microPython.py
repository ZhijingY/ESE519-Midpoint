from machine import RTC, Pin, PWM, Timer
from machine import I2C, sleep
import mpu6050
import machine
import network
import esp32
import socket
import json
import gc
import time
import urequests

red_led = Pin(33, Pin.OUT)
green_led = Pin(15, Pin.OUT)

red_led.value(0)
green_led.value(0)

ssid = "kaolagu"
password = "87654321"
#ssid = "Jin"
#password = "12345678yao"
wlan = network.WLAN(network.STA_IF)
wlan.active(True)
if not wlan.isconnected():
    print('connecting to network...')
    wlan.connect(ssid, password)
    while not wlan.isconnected():
        pass
print('Connected to', ssid)
print('IP address:', wlan.ifconfig())

i2c = I2C(1, scl=Pin(22), sda = Pin(23))
mpu = mpu6050.accel(i2c)

motion = False
timerFlag = 0
detectFlag = 0
timer0 = machine.Timer(0)
timer1 = machine.Timer(1)
order = 0;

def interruptHandler(timer0):
    global timerFlag
    timerFlag = 1
    
def detectHandler(timer1):
    global detectFlag
    detectFlag = 1
    
def calibrate_sensor():
    mpu.set_afs()
    mpu.calib_Ac()
    print("Calibration of accelerometer is done!")
    print(mpu.get_Ac_values())

timer0.init(mode = Timer.PERIODIC, period = 5000, callback = interruptHandler)
timer1.init(mode = Timer.PERIODIC, period = 6000, callback = detectHandler)

# Config AFS
calibrate_sensor()

# while loop 0.3
while(True):
    if(timerFlag == 1):
        state = machine.disable_irq()
        timerFlag = 0
        machine.enable_irq(state)
        
        if wlan.isconnected():
            gc.collect()
            print("Reading system status")
            status = urequests.get(url = 'https://api.thingspeak.com/channels/1721581/feeds/last.json').json()
            status = status['field1'].strip()
            order = order + 1
    
            if(status == 'Drink'):
                if(order == 1) :
                    green_led.value(1)
                    red_led.value(0)
                    print("Current status is drink")
                    print("Preparing drink")
                else :
                    urequests.get(url = 'https://api.thingspeak.com/update?api_key=6M9XOWZ408MD81G3&field1=check')
                
            elif(status == 'White lady'):
                if(order == 1) :
                    green_led.value(0)
                    red_led.value(1)
                    print("Current status is white lady")
                    print("Preparing drink")
                else :
                    urequests.get(url = 'https://api.thingspeak.com/update?api_key=6M9XOWZ408MD81G3&field1=check')
                
            else :
                green_led.value(0)
                red_led.value(0)
                order = 0
                print("Ready for next order")
