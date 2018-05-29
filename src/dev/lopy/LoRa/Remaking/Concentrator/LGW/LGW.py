from network import LoRa
import socket
import time
import pycom
import uos
import messageLoRa
import binascii
import struct
from machine import Timer
from messageLoRa import messageLoRa

isListening 	= True

class LGW:

	lora    		= None
	sock    		= None
	app_eui 		= None
	app_key 		= None
	dev_eui 		= None
	id      		= -1
	timer   		= 0
	NbIN 			= 0
	idRegistered	= []
	isRegistered	= []
	frequency		= 1
	discovered		= False
	slot			= 10
	MyLW			= 0
	nb_harvest		= 0
	listeningTime	= 10.0
	data			= 666

	def __init__(self,app_eui,app_key,dev_eui,id,frequency,slot):
		# create an OTAA authentication parameters
		self.app_eui=binascii.unhexlify(app_eui)
		self.app_key=binascii.unhexlify(app_key)
		self.dev_eui=binascii.unhexlify(dev_eui)
		self.lora = LoRa(mode=LoRa.LORAWAN, region=LoRa.EU868)
		self.id=id
		self.frequency=frequency
		self.slot=slot
		#self.isListening=True
		pycom.heartbeat(False)
		pycom.rgbled(0xff00)
	def start(self):

		global isListening
		# join a network using OTAA (Over the Air Activation)
		self.lora.join(activation=LoRa.OTAA, auth=(self.dev_eui,self.app_eui,self.app_key), timeout=0)
		# wait until the module has joined the network


		while not self.lora.has_joined():
			time.sleep(2.5)
			print('Not yet joined...')
		print('Connected to Objenious LoRaWAN!')
		self.sock = socket.socket(socket.AF_LORA, socket.SOCK_RAW)
		self.sock.setsockopt(socket.SOL_LORA, socket.SO_DR, 5)
		self.sock.setblocking(True)
		# send some data
		self.sock.send(bytes([0x01, 0x02, 0x03]))
		# make the socket non-blocking
		# (because if there's no data received it will block forever...)
		self.sock.setblocking(False)
		self.changetoLoRa()
		time.sleep(2.5)
		clock = TimerL(self.slot,2)
		while True:
		    if isListening:
		        try:
		            pycom.rgbled(0x007f00) # green
		            data = self.sock.recv(128)
		            self.handle_message(data)
		            time.sleep(1.500)
		            self.handle_message(data)
		            time.sleep(1.500)
		            recolte=self.standard()
		            time.sleep(1.500)
		            if recolte !="" :
		                time.sleep(1.500)
		                self.changetoLW()
		                time.sleep(1.500)
		                self.sock.setblocking(True)
		                print(recolte)
		                time.sleep(2)
		                self.send_datatoLWGW(recolte)
		                self.changetoLoRa()
		                self.sock.setblocking(False)
		        except OSError as err:
		            print("OS error: {0}".format(err))
		        #except EAGAIN as err:
		        #   print("EAGAIN error: {0}".format(err))
		    else:
		        pycom.rgbled(0x7f0000) #red
		        try:
		            print("I am sleeping")
		            time.sleep(self.slot)
		            del clock
		            clock = TimerL(self.listeningTime,2)
		            isListening=True
		        except OSError as err:
		            print("OS error: {0}".format(err))
		        #except EAGAIN as err:
		        #    print("EAGAIN error: {0}".format(err))
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
	    else:
	        print("FREQUENCY ALREADY CHANGED")

	def change_AND_send_toLW(self, devEUI_custom, data_custom):
		#print("FONCTION CHANGE TO LW 1")
		self.lora = LoRa(mode=LoRa.LORAWAN, region=LoRa.EU868)
		#print("FONCTION CHANGE TO LW 2 "+str(app_eui)+str(app_key)+str(dev_eui))
		self.lora.join(activation=LoRa.OTAA, auth=(binascii.unhexlify(devEUI_custom), self.app_eui, self.app_key), timeout=0)
		#print("FONCTION CHANGE TO LW 3")
		while not self.lora.has_joined():
			print('CtLW : Not yet joined... as '+str(devEUI_custom))
			time.sleep(2.5)
		print('Connected to Objenious LoRaWAN again ! as '+str(devEUI_custom))
		self.sock = socket.socket(socket.AF_LORA, socket.SOCK_RAW)
		self.sock.setsockopt(socket.SOL_LORA, socket.SO_DR, 5)
		data=data_custom
		taille=str(len(data))+'s'
		databytes = struct.pack(taille, data)
		self.sock.setblocking(True)
		self.sock.send(databytes)
		self.sock.setblocking(False)
		data=""
		time.sleep(1.5)
	def changetoLW(self):
	    #print("FONCTION CHANGE TO LW 1")
	    self.lora = LoRa(mode=LoRa.LORAWAN, region=LoRa.EU868)
	    #print("FONCTION CHANGE TO LW 2 "+str(app_eui)+str(app_key)+str(dev_eui))
	    self.lora.join(activation=LoRa.OTAA, auth=(self.dev_eui, self.app_eui, self.app_key), timeout=0)
	    #print("FONCTION CHANGE TO LW 3")
	    while not self.lora.has_joined():
	        print('LGW : Not yet joined...')
	        time.sleep(2.5)
	    print('Connected to Objenious LoRaWAN again !')
	    self.sock = socket.socket(socket.AF_LORA, socket.SOCK_RAW)
	    self.sock.setsockopt(socket.SOL_LORA, socket.SO_DR, 5)
	def changetoLoRa(self):
	    self.lora = LoRa(mode=LoRa.LORA, region=LoRa.EU868)
	    print('Radio mode is LoRa now !')
	    time.sleep(5)
	def send_datatoLWGW(self,dataString):
		data=dataString
		taille=str(len(data))+'s'
		databytes = struct.pack(taille, data)
		self.sock.send(databytes)
		data=""
		time.sleep(1.500)
	def pairing_phase(self,msg):
	    #print("PAIRING PHASE WITH "+str(msg.id_src)+" STARTED")
	    self.sock.send('Accept,'+str(2)+','+str(self.frequency)+','+str(self.slot)+','+str(self.id)+','+str(msg.id_src)+','+str(-1)+','+str(self.slot*3))
	    if msg.id_src in self.idRegistered:
	        print("Added before")
	    else:
	        self.idRegistered.append(msg.id_src)
	    #print("PAIRING PHASE WITH "+str(msg.id_src)+" ENDED")
	def registering_phase(self,msg):
	    #print("REGISTERING PHASE WITH "+str(msg.id_src)+" STARTED")
	    if msg.id_src in self.idRegistered:
	        self.sock.send('DataReq,'+str(4)+','+str(self.frequency)+','+str(self.slot)+','+str(self.id)+','+str(msg.id_src)+','+str(-1)+','+str(self.slot*3))
	    if msg.id_src in self.isRegistered:
	        print("Added before")
	    else:
	        self.isRegistered.append(msg.id_src)
	    #print("REGISTERING PHASE WITH "+str(msg.id_src)+" ENDED")
	def ack_data(self,msg):
	    #print("STANDARD PHASE STARTED")
	    #global slot
	    print("I received data : "+str(msg.data))
	    self.sock.send('ack,'+str(4)+','+str(self.frequency)+','+str(self.slot)+','+str(id)+','+str(self.msg.id_src)+','+str(-1)+','+str(self.slot*3))
	    #print("STANDARD PHASE ENDED")
	def standard(self):
	    print("STANDARD PHASE STARTED")
	    data_sum=""
	    for idDest in self.isRegistered:
	        print(idDest)
	        print('DataReq,'+str(4)+','+str(self.frequency)+','+str(self.slot)+','+str(self.id)+','+str(idDest)+','+str(-1)+','+str(self.slot*3))
	        self.sock.send('DataReq,'+str(4)+','+str(self.frequency)+','+str(self.slot)+','+str(self.id)+','+str(idDest)+','+str(-1)+','+str(self.slot*3))

	        dataHarvested = self.sock.recv(128)
	        msgH =messageLoRa()
	        msgH.fillMessage(dataHarvested)
	        rnd=self.Random()
	        print("[FIRST Send] for "+str(idDest)+" Request data in "+str(rnd))
	        print(dataHarvested)
	        time.sleep(rnd)
	        while msgH.id_src != str(idDest) or msgH.id_dest != str(self.id) or msgH.kind != "5" or msgH.messageName != "DataRes":
	            rnd=self.Random()
	            print("[Try] for "+str(idDest)+" send Request data in "+str(rnd))
	            time.sleep(rnd)
	            self.sock.send('DataReq,'+str(4)+','+str(self.frequency)+','+str(self.slot)+','+str(self.id)+','+str(idDest)+','+str(-1)+','+str(self.slot*3))
	            dataHarvested = self.sock.recv(128)
	            msgH =messageLoRa()
	            msgH.fillMessage(dataHarvested)
	            print("msg data =========>"+dataHarvested.decode())
	        data_sum=data_sum+str(idDest)+","+str(msgH.data)+":"
		data_sum=data_sum[:-1]
	    print("STANDARD PHASE ENDED")
	    return data_sum
	def handle_message(self,data):
	    msg =messageLoRa()
	    msg.fillMessage(data)
	    print(data)
	    #time.sleep(5)
	    if msg.kind == "1":
	        self.pairing_phase(msg)
	    else:
	        print(msg.kind )
	    if msg.kind == "3" and msg.id_dest == str(self.id):
	        self.registering_phase(msg)
	    if msg.kind == "3" and msg.id_dest != str(self.id):
	        if msg.id_src in idRegistered:
	            self.idRegistered.remove(msg.id_src)
	            print("Delete ID:"+str(msg.id_src)+"from the table idRegistered")
class TimerL:
	def __init__(self,timing,kind):
		global isListening
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

lora = LoRa(mode=LoRa.LORAWAN, region=LoRa.EU868)
app_eui = '70 B3 D5 7E F0 00 49 E1'.replace(' ','')
app_key = '30 4C 99 26 3E A5 E6 43 B5 A0 8C B3 25 4A 61 FA'.replace(' ','')
dev_eui = binascii.hexlify(lora.mac()).decode('ascii').upper()
del lora
#del lora
#print(app_eui)
#print(app_key)
#print(dev_eui)
test= LGW(app_eui,app_key,dev_eui,36,1.0,20)
test.start()
