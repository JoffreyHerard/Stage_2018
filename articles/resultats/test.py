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


corrupt=0
max_degree_IN= 4
max_degree_GW= 4
nb_vertex= 1000
nb_graph=100
current_degree_IN=1
current_degree_GW=1

#try 4D
simu_msg_evaluate="1"

filename=str(current_degree_IN)+"-"+str(current_degree_GW)
dirs = os.listdir( "extract/"+filename+"/"+simu_msg_evaluate)
