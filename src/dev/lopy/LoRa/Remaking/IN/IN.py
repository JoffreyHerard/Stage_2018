from network import LoRa
import socket
import machine
import uos
import time
import pycom
import messageLoRa
from messageLoRa import messageLoRa
from machine import Timer

isListening=True

class IN:
	global isListening
	id=1
	timer=0
	data=42
	tryDiscover=0
	tryRegister=0
	discovered=False
	registered=False
	ack_Data=False
	frequency=1.0
	slot=10
	myLoRa=-1
	tryDataReq=-1
	clock=None
	sock=None
	listeningTime=10.0
	def __init__(self,id,frequency,slot):
		# create an OTAA authentication parameters
		self.lora = LoRa(mode=LoRa.LORA, region=LoRa.EU868)
		self.sock = socket.socket(socket.AF_LORA, socket.SOCK_RAW)
		self.sock.setblocking(False)
		self.id=id
		self.timer=0
		self.data=42
		self.tryDiscover=0
		self.tryRegister=0
		self.discovered=False
		self.registered=False
		self.ack_Data=False
		self.frequency=1.0
		self.slot=slot
		self.myLoRa=-1
		self.tryDataReq=0
		self.isListening=True
		self.listeningTime=10.0
		pycom.heartbeat(False)
		pycom.rgbled(0xff00)

	def start(self):
		#ecrire le maion
		global isListening
		while True:
		    if isListening:
		        #print("I am awake : my LoRaGW is "+str(myLoRa)+" and my slot is "+str(slot))
		        pycom.rgbled(0x007f00) # green
		        try:    #We are not discovered yet
		            while not self.discovered:
		                self.notDiscovered()
		                rnd=self.Random()
		                print("Try Discover in "+str(rnd))
		                time.sleep(rnd)
		            while not self.registered and self.discovered:
		                self.notRegistered()
		                rnd=self.Random()
		                print("Try Register in "+str(rnd))
		                time.sleep(rnd)
		            dataR=self.sock.recv(128)
		            msg =messageLoRa()
		            msg.fillMessage(dataR)
		            if msg.kind=="4" and msg.id_dest == str(self.id):
		                self.sendData()
		                print("I sent my data")
		                print("I try to change my slot and listening time")
		                self.slot=float(msg.slots)
		                self.listeningTime=float(msg.listeningtime) #slot d'une duree de 40 seconde
		                isListening=False
						#if self.clock = None :
						#	del self.clock
		                self.clock = TimerL(self.slot,1)
		                toto=False
		        except OSError as err:
		            print("OS error: {0}".format(err))
		        #except err:
		        #   print("EAGAIN error: {0}".format(err))
		    else:
		        pycom.rgbled(0x7f0000) #red
		        try:
					print("my slot is :"+str(self.slot))
		            print("I am sleeping")
		            time.sleep(self.slot)
		            del self.clock
		            self.clock = TimerL(self.listeningTime,2)
		            isListening=True
		        except OSError as err:
		            print("OS error: {0}".format(err))
		        except err:
		            print("EAGAIN error: {0}".format(err))

	def Random(self):
	    result = ((uos.urandom(1)[0] / 256 )*3)+2
	    return result
	def change_frequency(self,frequency_d):
	    current_frequency=self.lora.frequency()
	    if current_frequency != frequency_d:
	        print("FREQUENCY WAS CHANGED FROM :"+str(current_frequency)+" TO= ")
	        if frequency_d == 1:
	            self.lora.frequency(868000000)
	            print("868000000")
	        if frequency_d == 2:
	            self.lora.frequency(868100000)
	            print("868100000")
	        if frequency_d == 3:
	            self.lora.frequency(868300000)
	            print("868300000")
	        if frequency_d == 4:
	            self.lora.frequency(868500000)
	            print("868500000")
	        if frequency_d == 5:
	            self.lora.frequency(864100000)
	            print("864100000")
	        if frequency_d == 6:
	            self.lora.frequency(864300000)
	            print("864300000")
	        if frequency_d == 7:
	            self.lora.frequency(864500000)
	            print("864500000")
	class TimerL:
	    def __init__(self,timing,kind):
	        self.seconds = 0
	        if kind == 1:
	            self.__alarm = Timer.Alarm(self._first_handler, timing, periodic=True)
	        else:
	            self.__alarm = Timer.Alarm(self._seconds_handler, timing, periodic=True)

	    def _first_handler(self, alarm):
	        global isListening
	        alarm.cancel() # stop it
	        isListening=True
	    def _seconds_handler(self, alarm):
	        global isListening
	        alarm.cancel() # stop it
	        isListening=False
	def notDiscovered(self):
	    #print("PHASE NOT DISCOVERED STARTED "+str(tryDiscover))
	    self.sock.send('Discover,'+str(1)+','+str(self.frequency)+','+str(self.slot)+','+str(self.id)+','+str(-1)+','+str(-1)+','+str(-1))
	    print("Discover sent by "+str(self.id))
	    data=self.sock.recv(128)
	    msg =messageLoRa()
	    msg.fillMessage(data)
	    #print("dest ==="+str(msg.get_dest()))
	    if msg.messageName == "Accept" and msg.id_dest == str(self.id):
	        self.myLoRa=msg.id_src
	        self.frequency=msg.frequency
	        self.change_frequency(msg.frequency)
	        print("Receive ACCEPT msg")
	        self.discovered=True
	    else:
	        time.sleep(1)
	    self.tryDiscover+=1
	    #print("PHASE NOT DISCOVERED ENDED\n")
	def notRegistered(self):
	    #send some data
	    #print("PHASE NOT REGISTERED STARTED\n")
	    self.sock.send('Register,'+str(3)+','+str(self.frequency)+','+str(self.slot)+','+str(self.id)+','+str(self.myLoRa)+','+str(-1)+','+str(-1))
	    print("Register sent")
	    # get any data received...
	    data=self.sock.recv(128)
	    msg =messageLoRa()
	    msg.fillMessage(data)
	    if msg.messageName == "DataReq" and msg.id_src== self.myLoRa and msg.id_dest == str(self.id):
	        self.registered=True
	    else:
	        self.tryRegister+=1
	    #print("PHASE NOT REGISTERED ENDED\n")
	def sendData(self):
	    #send some data
	    #print("PHASE SEND DATA STARTED\n")
	    #print('DataRes,'+str(5)+','+str(frequency)+','+str(slot)+','+str(id)+','+str(myLoRa)+','+str(data)+','+str(70))
	    self.sock.send('DataRes,'+str(5)+','+str(self.frequency)+','+str(self.slot)+','+str(self.id)+','+str(self.myLoRa)+','+str(self.data)+','+str(70))
	    #time.sleep(20)
	    print("DataResponse sent")
	    #print("PHASE SEND DATA ENDED\n")
class TimerL:
	def __init__(self,timing,kind):
		#global isListening
		self.seconds = 0
		if kind == 1:
			self.__alarm = Timer.Alarm(self._first_handler, timing, periodic=True)
		else:
			self.__alarm = Timer.Alarm(self._seconds_handler, timing, periodic=True)
	def _first_handler(self, alarm):
		global isListening
		alarm.cancel() # stop it
		isListening=True
	def _seconds_handler(self, alarm):
		global isListening
		alarm.cancel() # stop it
		isListening=False
test=IN(1, 1.0,20)
test.start()
