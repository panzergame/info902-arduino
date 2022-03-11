import serial
ser = serial.Serial('/dev/rfcomm0')  # open serial port


print(ser.name)         # check which port was really used

while True:
    try:
        print(ser.readline())
        print("reussite !!")
    except:
        print("attente")
#ser.write(b'hello')     # write a string

ser.close()             # close port 
