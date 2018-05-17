from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
import numpy as np
import os
import sys
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')


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



corrupt=0
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
		current_degree_IN=current_degree_IN+1		


figure=ax.scatter(numero_simulation, msgtosend, data_bat_gw, c=degree_gw, cmap="cool")

#figure=ax.scatter(N_numero_simulation, N_msgtosend, N_data_msg_gw, c=N_degree_in, cmap=plt.cool())

#plt.plot(numero_simulation, data_msg_gw,msgtosend)
plt.ylabel('Data messages by day')
plt.xlabel("simulation number")
#plt.zlabel("Total messages sent")
# Add a color bar which maps values to colors.
fig.colorbar(figure, shrink=0.5, aspect=5)
#Sim/MSG_SEND/data_bat_gw/degree_gw
plt.savefig('batGW_Din.png')
plt.show()
#print("maximum des msg IN  "+str(np.max(data_msg_in)))
#print("maximum des msg GW "+str(np.max(data_msg_gw)))
#print("maximum des msg WELL "+str(np.max(data_msg_well)))