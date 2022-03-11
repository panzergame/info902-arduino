from bluedot.btcomm import BluetoothClient
from signal import pause

def data_received(data):
    print(data)

c = BluetoothClient("00:0E:0E:0E:AB:89", data_received)
print(c.connected)

pause() 
