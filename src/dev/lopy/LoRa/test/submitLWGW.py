from network import LoRa
import socket
import time
import pycom
import uos
import binascii
import struct
from machine import Timer
pycom.heartbeat(False)
pycom.rgbled(0xff00)
lora = LoRa(mode=LoRa.LORAWAN, region=LoRa.EU868)
s = socket.socket(socket.AF_LORA, socket.SOCK_RAW)
s.setsockopt(socket.SOL_LORA, socket.SO_DR, 5)

# create an OTAA authentication parameters
app_eui = binascii.unhexlify('70 B3 D5 7E F0 00 49 E1'.replace(' ',''))
app_key = binascii.unhexlify('30 4C 99 26 3E A5 E6 43 B5 A0 8C B3 25 4A 61 FA'.replace(' ',''))
dev_eui = binascii.unhexlify(binascii.hexlify(lora.mac()).decode('ascii').upper())
print(dev_eui)
#dev_addr = struct.unpack(">l", binascii.unhexlify('00 00 00 05'.replace(' ','')))[0]
# join a network using OTAA (Over the Air Activation)
lora.join(activation=LoRa.OTAA, auth=(dev_eui,app_eui, app_key), timeout=0)

# wait until the module has joined the network
while not lora.has_joined():
    time.sleep(2.5)
    print('Not yet joined...')
print('Connected to Objenious LoRaWAN!')

# send some data
while True:
    time.sleep(2.5)
    s.setblocking(True)
    s.send(bytes([0x32, 0x18, 0x41]))
    s.setblocking(False)
