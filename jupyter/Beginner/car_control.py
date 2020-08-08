from pynq import Xlnk
from pynq import Overlay
def pwm(timer,pulse_width, high_width):
    timer.write(0x04,pulse_width) #Pulse Width, 100M/100K = 1000
    timer.write(0x14,high_width) #Pulse High Width

def pwm_start(timer):
    timer.write(0x00,0b000000000000) #Timer 0 be set to Generate mode
    timer.write(0x10,0b000000000000) #Timer 1 be set to Generate mode
    timer.write(0x00,0b001000000000) #enable PWM mode
    timer.write(0x10,0b001000000000) #enable PWM mode
    timer.write(0x00,0b001000000110) #enable GenerateOut signals, down mode
    timer.write(0x10,0b001000000110) #enable GenerateOut signals, down mode
    timer.write(0x10,0b001010010110) #enable PWM
    timer.write(0x00,0b000000010110) #enable PWM
    timer.write(0x00,0b011010010110)

def pwm_stop(timer):
    timer.write(0x00,0b000000010110)
