from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
import numpy as np
import os
import sys
#fig = plt.figure()
#ax = fig.add_subplot(111, projection='2d')


#POUR 1 MSG PAR JOUR en  agrege
#x =msg IN
#y=numero simulation
#z=degre IN
#c=degre GW

data_msg_in=[] #x1
data_msg_gw=[] #x2
data_msg_well=[] #x4
data_bat_in=[] #x3
data_bat_gw=[] #x4

degree_in=[] #y
degree_gw=[] #z
numero_simulation=[] #c

msgtosend=[]





N_data_msg_in=[] #x1
N_data_msg_gw=[] #x2
N_data_msg_well=[] #x4
N_data_bat_in=[] #x3
N_data_bat_gw=[] #x4

N_degree_in=[] #y
N_degree_gw=[] #z
N_numero_simulation=[] #c

N_msgtosend=[]



maximum_gw1=[]
degree_max1=[]
maximum_N_gw1=[]


maximum_gw2=[]
degree_max2=[]
maximum_N_gw2=[]


maximum_gw10=[]
degree_max10=[]
maximum_N_gw10=[]


maximum_gw24=[]
degree_max24=[]
maximum_N_gw24=[]




max_degree_IN= 4
max_degree_GW= 4
nb_vertex= 1000
nb_graph=100


#try 4D




simtodo=["1","2","10","24"]

for simu_msg_evaluate in simtodo:
	i=0
	j=0
	current_degree_IN=1
	current_degree_GW=1
	while current_degree_IN < max_degree_IN+1:
		current_degree_GW=1

		data_msg_in=[] #x1
		data_msg_gw=[] #x2
		data_msg_well=[] #x4
		data_bat_in=[] #x3
		data_bat_gw=[] #x4

		degree_in=[] #y
		degree_gw=[] #z
		numero_simulation=[] #c

		msgtosend=[]





		N_data_msg_in=[] #x1
		N_data_msg_gw=[] #x2
		N_data_msg_well=[] #x4
		N_data_bat_in=[] #x3
		N_data_bat_gw=[] #x4

		N_degree_in=[] #y
		N_degree_gw=[] #z
		N_numero_simulation=[] #c

		N_msgtosend=[]
		while current_degree_GW < max_degree_GW+1:
			filename=str(current_degree_IN)+"-"+str(current_degree_GW)
			dirs = os.listdir( "extract/"+filename+"/"+simu_msg_evaluate)
			dirs = np.sort(dirs)
			for file in dirs:
				f1="extract/"+filename+"/"+simu_msg_evaluate+"/"+file
				with open(f1) as f :
					data_msg_in.append(float(f.readline()))
					data_msg_gw.append(float(f.readline()))
					data_msg_well.append(float(f.readline()))
					data_bat_in.append(float(f.readline()))
					data_bat_gw.append(float(f.readline()))
					degree_in.append(current_degree_IN)
					degree_gw.append(current_degree_GW)
					numero_simulation.append(i)
					msgtosend.append(int(simu_msg_evaluate))
					f.close()
				i=i+1
			filename=str(current_degree_IN)+"-"+str(current_degree_GW)
			dirs = os.listdir( "extract/"+filename+"/N"+simu_msg_evaluate)
			dirs = np.sort(dirs)
			for file in dirs:
				f1="extract/"+filename+"/N"+simu_msg_evaluate+"/"+file
				with open(f1) as f :
					N_data_msg_in.append(float(f.readline()))
					N_data_msg_gw.append(float(f.readline()))
					N_data_msg_well.append(float(f.readline()))
					N_data_bat_in.append(float(f.readline()))
					N_data_bat_gw.append(float(f.readline()))
					N_degree_in.append(current_degree_IN)
					N_degree_gw.append(current_degree_GW)
					N_numero_simulation.append(j)
					N_msgtosend.append(int(simu_msg_evaluate))
					f.close()
				j=j+1
			current_degree_GW=current_degree_GW+1
		if simu_msg_evaluate == "1" :
			maximum_gw1.append(max(data_msg_gw))
			degree_max1.append(current_degree_IN)
			maximum_N_gw1.append(max(N_data_msg_gw))
		if simu_msg_evaluate == "2" :
			maximum_gw2.append(max(data_msg_gw))
			degree_max2.append(current_degree_IN)
			maximum_N_gw2.append(max(N_data_msg_gw))

		if simu_msg_evaluate == "10" :
			maximum_gw10.append(max(data_msg_gw))
			degree_max10.append(current_degree_IN)
			maximum_N_gw10.append(max(N_data_msg_gw))

		if simu_msg_evaluate == "24" :
			maximum_gw24.append(max(data_msg_gw))
			degree_max24.append(current_degree_IN)
			maximum_N_gw24.append(max(N_data_msg_gw))
		current_degree_IN=current_degree_IN+1	

courbe1=[]
courbeN1=[]

courbe1.append(maximum_gw1[0])
courbe1.append(maximum_gw2[0])
courbe1.append(maximum_gw10[0])
courbe1.append(maximum_gw24[0])


courbeN1.append(maximum_N_gw1[0])
courbeN1.append(maximum_N_gw2[0])
courbeN1.append(maximum_N_gw10[0])
courbeN1.append(maximum_N_gw24[0])


courbe2=[]
courbeN2=[]

courbe2.append(maximum_gw1[1])
courbe2.append(maximum_gw2[1])
courbe2.append(maximum_gw10[1])
courbe2.append(maximum_gw24[1])


courbeN2.append(maximum_N_gw1[1])
courbeN2.append(maximum_N_gw2[1])
courbeN2.append(maximum_N_gw10[1])
courbeN2.append(maximum_N_gw24[1])



courbe3=[]
courbeN3=[]

courbe3.append(maximum_gw1[2])
courbe3.append(maximum_gw2[2])
courbe3.append(maximum_gw10[2])
courbe3.append(maximum_gw24[2])


courbeN3.append(maximum_N_gw1[2])
courbeN3.append(maximum_N_gw2[2])
courbeN3.append(maximum_N_gw10[2])
courbeN3.append(maximum_N_gw24[2])


courbe4=[]
courbeN4=[]

courbe4.append(maximum_gw1[3])
courbe4.append(maximum_gw2[3])
courbe4.append(maximum_gw10[3])
courbe4.append(maximum_gw24[3])


courbeN4.append(maximum_N_gw1[3])
courbeN4.append(maximum_N_gw2[3])
courbeN4.append(maximum_N_gw10[3])
courbeN4.append(maximum_N_gw24[3])



print(courbe1)
print(courbe2)
print(courbe3)
print(courbe4)

print("NON AGREGER ")

print(courbeN1)
print(courbeN2)
print(courbeN3)
print(courbeN4)


# axe x =  1 2 10 24 
#axe y nombre de message
x= np.array([1,2,10,24])
#toot= np.array([-0.5,-0.5,-0.5,-0.5])
#plt.plot(x, courbe1, "o-", label=" 1 IN max")
#plt.plot(x, courbe2, "o--", label="2 IN max")
#plt.plot(x, courbe3, "o:", label="3 IN max")
#plt.plot(x, courbe4, "o-.", label="4 IN max")


plt.plot(x, courbeN1, "v-", label=" NA-1 IN max")
plt.plot(x, courbeN2, "v--", label="NA-2 IN max")
plt.plot(x, courbeN3, "v:", label="NA-3 IN max")
plt.plot(x, courbeN4, "v-.", label="NA-4 IN max")

plt.xlabel("data messages/day")
plt.ylabel("Message sent")
plt.legend()
plt.savefig('NA.png')
plt.show() # affiche la figure a l'ecran
#courbe = nombre de message 
