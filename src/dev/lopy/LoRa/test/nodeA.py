from network import LoRa
import socket
import machine
import time

# initialize LoRa in LORA mode
# Please pick the region that matches where you are using the device:
# Asia = LoRa.AS923
# Australia = LoRa.AU915
# Europe = LoRa.EU868
# United States = LoRa.US915
# more params can also be given, like frequency, tx power and spreading factor
lora = LoRa(mode=LoRa.LORA, region=LoRa.EU868)

# create a raw LoRa socket
s = socket.socket(socket.AF_LORA, socket.SOCK_RAW)
while True:
    s.setblocking(False)
    data = s.recv(128)
    #if data =="Discover,1,1.0,20,70b3d5499c3dd0ac,-1,-1,-1":
    print(data)
    time.sleep(1)
    # wait a random amount of time
