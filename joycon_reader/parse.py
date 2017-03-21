import time
import serial

def byte_to_int16(b1, b2):
	result = (b1 << 8) | b2
	if result > 32767:
		result -= 65536
	return result

ser = serial.Serial('COM4')

try:
	while 1:
		received = ser.readline().decode().replace('\n', '')
		print(received)
except KeyboardInterrupt:
	ser.close()
	exit()