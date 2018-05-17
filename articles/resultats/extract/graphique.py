from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
import numpy as np

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')


#POUR 1 MSG PAR JOUR en  agrege
#x =msg IN
#y=numero simulation
#z=degre IN
#c=degre GW


max_degree_IN= 4
max_degree_GW= 4
nb_vertex= 1000
nb_graph=100
current_degree_IN=4
current_degree_GW=4
while current_degree_IN < max_degree_IN+1:
	while current_degree_GW < max_degree_GW+1:
		i = 0
		#0 a 99.dat
		while i < nb_graph:
			








x = np.random.standard_normal(100) 
y = np.random.standard_normal(100)
z = np.random.standard_normal(100)
c = np.random.standard_normal(100)
ax.scatter(x, y, z, c=c, cmap=plt.hot())
plt.show()


ax.scatter(x, y, z, c=c, cmap=plt.hot())
plt.show()