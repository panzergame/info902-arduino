from bluedot.btcomm import BluetoothClient
from signal import pause

def data_received(data):
    print(data)

c = BluetoothClient("00:0E:0B:0E:9E:82", data_received)
print(c.connected)

pause() 
