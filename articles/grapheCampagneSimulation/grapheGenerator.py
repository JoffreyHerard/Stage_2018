#! /usr/bin/python

from igraph import *
import sys
import random
import os

file_written=0
def whatsthat(number, well, gw, IN):
	if number not in well:
		if number not in gw:
			toR=("IN",IN.index(number))
			return toR
		else:
			toR=("WGW",gw.index(number))
			return toR
	else:
		toR=("WELL",well.index(number))
		return toR
def createGraph(lower_bound,upper_bound,filename,max_degree_IN,max_degree_GW,nb_vertex_max_input):
	global file_written
	#print("\t\t   -----SIMULATION AVEC PARAMETRES SUIVANT :-----")
	#print("\t lower_bound : "+str(lower_bound)+"/ upper_bound : "+str(upper_bound)+" /nom de fichier :"+filename+"/ max degree IN "+str(max_degree_IN)+"/max_degree_GW "+str(max_degree_GW))
	#Random number is the probability which have a LoraGateway to have one isolated node
	p=random.random()
	#Random number is the probability which have a LoraGateway to have another isolated node
	p2=random.random()
	current_graph=0
	nb_max_vertex=nb_vertex_max_input
	nb_degree_max_IN=max_degree_IN
	nb_degree_max_GW=max_degree_GW
	names=[]
	id_well=[]
	id_gw=[]
	id_in=[]
	nb_vertex=0
	nbwell=0
	nbgw=0
	nbin=0
	current_well=0
	#make the graph
	g = Graph()
	#Adding LoRaWAN, LGW, IN

	#Tant que nb_noeud < 1000 faire
	while nb_vertex <nb_max_vertex :
		#rajouter Puit
		g.add_vertices(1)
		if nb_vertex >0 :
			g.add_edges([(current_well,nb_vertex)])
		#on repertorie l'id du puit
		id_well.append(nb_vertex)
		names.append("WELL")
		#names.append(str(nb_vertex))
		current_well=nb_vertex
		#on augmente le nombre de sommet actuel
		nb_vertex=nb_vertex+1
		nbwell=1+nbwell
		#Tant que nb_noeud < 1000 ET degre_actuel < degre max ET iteration< degre max faire
		actual_degree_GW=0
		nb_loop_GW=0
		while nb_vertex <nb_max_vertex and actual_degree_GW < nb_degree_max_GW and nb_loop_GW < nb_degree_max_GW:
			# x <- tirer probabilite
			#print("PREMIER WHILE : "+ str(nb_vertex) +" < "+ str(nb_max_vertex) +" ET "+str(actual_degree_GW)+" < " + str(nb_degree_max_GW) +" AND "+str(nb_loop_GW)+" < "+str(nb_degree_max_GW))
			current_gw=0
		 	x=random.random()
			#Si x > p1
			if x > p and actual_degree_GW < nb_degree_max_GW:

				names.append("GW")
				#names.append(str(nb_vertex))
				nbgw=nbgw+1
				actual_degree_GW=actual_degree_GW+1
				#on repertorie l'id de la gw
				id_gw.append(nb_vertex)
				#rajouter GW
				g.add_vertices(1)
				current_gw=nb_vertex
				nb_vertex=nb_vertex+1
				#cree connection entre GW et PUIT
				g.add_edges([(current_well,current_gw)])
				#Tant que nb_noeud < 1000 ET degre_actuel < degre max ET iteration< degre max faire
				actual_degree_IN=0
				nb_loop_IN=0

				#au moins un IN

				nbin=nbin+1
				names.append("IN")
				#names.append(str(nb_vertex))
				actual_degree_IN=actual_degree_IN+1
				#on repertorie l'id de l'IN
				id_in.append(nb_vertex)
				#rajouter IN
				g.add_vertices(1)
				current_in=nb_vertex
				nb_vertex=nb_vertex+1
				#cree connection entre GW et IN
				g.add_edges([(current_gw,current_in)])
				#print(" On ajoute un IN entre"+str(current_gw)+" et "+str(current_in))

				while nb_vertex <nb_max_vertex and actual_degree_IN < nb_degree_max_IN and nb_loop_IN < nb_degree_max_IN:
					#print("APRES DEUXIEME WHILE : "+ str(nb_vertex) +" < "+ str(nb_max_vertex) +" ET "+str(actual_degree_IN)+" < " + str(nb_degree_max_IN) +" AND "+str(nb_loop_IN)+" < "+str(nb_degree_max_IN))

					# y <- tirer probabilite
					current_in=0
				 	y=random.random()
					#Si x > p2%
					if y > p2 :
						nbin=nbin+1
						names.append("IN")
						#names.append(str(nb_vertex))
						actual_degree_IN=actual_degree_IN+1
						#on repertorie l'id de l'IN
						id_in.append(nb_vertex)
						#rajouter IN
						g.add_vertices(1)
						current_in=nb_vertex
						nb_vertex=nb_vertex+1
						#cree connection entre GW et IBN
						g.add_edges([(current_gw,current_in)])
						#print(" On ajoute un IN entre"+str(current_gw)+" et "+str(current_in))
					nb_loop_IN=nb_loop_IN+1
			nb_loop_GW=nb_loop_GW+1
	g.vs["label"]=names
	g.es[0]
	#g.write_dot(filename+".dot")
	#Making file .ned
	file_written=file_written+1
	#print "graphe/"+filename+".ned"
	fic=open("graphe/"+filename+".ned","w")
	fic.write("package lora.simulations;\n")
	fic.write("import lora.*;\n")
	fic.write("network Net\n")
	fic.write("{\n")

	fic.write("\t submodules:\n");
	fic.write("\t\t WELL["+str(nbwell)+"]: LWGW; \n");
	fic.write("\t\t WGW["+str(nbgw)+"]: LGW; \n");
	fic.write("\t\t IN["+str(nbin)+"]: IsoN; \n");
	fic.write("\t connections:\n");
	for l in g.get_edgelist():
		firstVertex=whatsthat(l[0],id_well,id_gw,id_in)
		secondVertex=whatsthat(l[1],id_well,id_gw,id_in)
		nomSommet0=firstVertex[0]
		num_sommet0=firstVertex[1]
		nomSommet1=secondVertex[0]
		num_sommet1=secondVertex[1]
		delay1=random.randint(lower_bound,upper_bound)
		delay2=random.randint(lower_bound,upper_bound)

		fic.write("\t\t\t" + ""+nomSommet0+"["+str(num_sommet0)+"].channelsO++" + " --> " +"{delay="+str(delay1)+"ms;}" + " --> " +""+nomSommet1+"["+str(num_sommet1)+"].channelsI++" + ";\n")
		fic.write("\t\t\t" + ""+nomSommet0+"["+str(num_sommet0)+"].channelsI++" + " <-- " +"{delay="+str(delay2)+"ms;}" + " <-- " +  ""+nomSommet1+"["+str(num_sommet1)+"].channelsO++" + ";\n")

	fic.write("}")
	fic.close()







#DEBUT 10 MESSAGES PAR JOUR
	# AGREGATION
	#Making file omnetpp.ini
	#print "omnetppini/"+filename+"-10MSG.ini"
	file_written=file_written+1
	fic=open("omnetppini/"+filename+"-10MSG.ini","w")
	fic.write("[General]\n")
	fic.write("user-interface=Cmdenv\n")
	fic.write("network = lora.simulations.Net\n")
	fic.write("sim-time-limit=86400s\n")
	fic.write("ned-path=/Users/joffreyherard/Documents/Master_2/Projet_fin_etude/dev/src/src;/Users/joffreyherard/Documents/Master_2/Projet_fin_etude/dev/src/simulations\n")
	i = 0
	while i < nbwell:
		fic.write("Net.WELL["+str(i)+"].id="+str(i)+"\n")
		fic.write("Net.WELL["+str(i)+"].file=\""+filename+"-10MSG\"\n")
		fic.write("Net.WELL["+str(i)+"].slot="+str(8640)+"\n") #changer pour nb message
		i=i+1
	j = 0
	while j < nbgw:
		fic.write("Net.WGW["+str(j)+"].id="+str(i)+"\n")
		fic.write("Net.WGW["+str(j)+"].file=\""+filename+"-10MSG\"\n")
		fic.write("Net.WGW["+str(j)+"].agreg=true\n")#changer pour false
		fic.write("Net.WGW["+str(j)+"].battery="+str(23760000)+"\n")
		i=i+1
		j=j+1
	k = 0
	while k < nbin:
		fic.write("Net.IN["+str(k)+"].id="+str(i)+"\n")
		fic.write("Net.IN["+str(k)+"].file=\""+filename+"-10MSG\"\n")
		fic.write("Net.IN["+str(k)+"].battery="+str(23760000)+"\n")
		k=k+1
		i=i+1
	fic.close()

	file_written=file_written+1
	#print "omnetppini/"+filename+"-NA-10MSG.ini"
	fic=open("omnetppini/"+filename+"-NA-10MSG.ini","w")
	fic.write("[General]\n")
	fic.write("user-interface=Cmdenv\n")
	fic.write("network = lora.simulations.Net\n")
	fic.write("sim-time-limit=86400s\n")
	fic.write("ned-path=/Users/joffreyherard/Documents/Master_2/Projet_fin_etude/dev/src/src;/Users/joffreyherard/Documents/Master_2/Projet_fin_etude/dev/src/simulations\n")
	i = 0
	while i < nbwell:
		fic.write("Net.WELL["+str(i)+"].id="+str(i)+"\n")
		fic.write("Net.WELL["+str(i)+"].file=\""+filename+"-NA-10MSG\"\n")
		fic.write("Net.WELL["+str(i)+"].slot="+str(8640)+"\n") #changer pour nb message
		i=i+1
	j = 0
	while j < nbgw:
		fic.write("Net.WGW["+str(j)+"].id="+str(i)+"\n")
		fic.write("Net.WGW["+str(j)+"].file=\""+filename+"-NA-10MSG\"\n")
		fic.write("Net.WGW["+str(j)+"].agreg=false\n")#changer pour false
		fic.write("Net.WGW["+str(j)+"].battery="+str(23760000)+"\n")
		i=i+1
		j=j+1
	k = 0
	while k < nbin:
		fic.write("Net.IN["+str(k)+"].id="+str(i)+"\n")
		fic.write("Net.IN["+str(k)+"].file=\""+filename+"-NA-10MSG\"\n")
		fic.write("Net.IN["+str(k)+"].battery="+str(23760000)+"\n")
		k=k+1
		i=i+1
	fic.close()



#FIN 10 MESSAGES PAR JOUR




#DEBUT 1 MESSAGES PAR JOUR 12 h

	# AGREGATION
	#Making file omnetpp.ini
	file_written=file_written+1
	#print "omnetppini/"+filename+"-1MSG.ini"
	fic=open("omnetppini/"+filename+"-1MSG.ini","w")
	fic.write("[General]\n")
	fic.write("user-interface=Cmdenv\n")
	fic.write("network = lora.simulations.Net\n")
	fic.write("sim-time-limit=86400s\n")
	fic.write("ned-path=/Users/joffreyherard/Documents/Master_2/Projet_fin_etude/dev/src/src;/Users/joffreyherard/Documents/Master_2/Projet_fin_etude/dev/src/simulations\n")
	i = 0
	while i < nbwell:
		fic.write("Net.WELL["+str(i)+"].id="+str(i)+"\n")
		fic.write("Net.WELL["+str(i)+"].slot="+str(86400)+"\n") #changer pour nb message
		fic.write("Net.WELL["+str(i)+"].file=\""+filename+"-1MSG\"\n")
		i=i+1
	j = 0
	while j < nbgw:
		fic.write("Net.WGW["+str(j)+"].id="+str(i)+"\n")
		fic.write("Net.WGW["+str(j)+"].file=\""+filename+"-1MSG\"\n")
		fic.write("Net.WGW["+str(j)+"].agreg=true\n")#changer pour false
		fic.write("Net.WGW["+str(j)+"].battery="+str(23760000)+"\n")
		i=i+1
		j=j+1
	k = 0
	while k < nbin:
		fic.write("Net.IN["+str(k)+"].id="+str(i)+"\n")
		fic.write("Net.IN["+str(k)+"].file=\""+filename+"-1MSG\"\n")
		fic.write("Net.IN["+str(k)+"].battery="+str(23760000)+"\n")
		k=k+1
		i=i+1
	fic.close()


	file_written=file_written+1
	#print "omnetppini/"+filename+"-NA-1MSG.ini"
	fic=open("omnetppini/"+filename+"-NA-1MSG.ini","w")
	fic.write("[General]\n")
	fic.write("user-interface=Cmdenv\n")
	fic.write("network = lora.simulations.Net\n")
	fic.write("sim-time-limit=86400s\n")
	fic.write("ned-path=/Users/joffreyherard/Documents/Master_2/Projet_fin_etude/dev/src/src;/Users/joffreyherard/Documents/Master_2/Projet_fin_etude/dev/src/simulations\n")
	i = 0
	while i < nbwell:
		fic.write("Net.WELL["+str(i)+"].id="+str(i)+"\n")
		fic.write("Net.WELL["+str(i)+"].slot="+str(86400)+"\n") #changer pour nb message
		fic.write("Net.WELL["+str(i)+"].file=\""+filename+"-NA-1MSG\"\n")
		i=i+1
	j = 0
	while j < nbgw:
		fic.write("Net.WGW["+str(j)+"].id="+str(i)+"\n")
		fic.write("Net.WGW["+str(j)+"].file=\""+filename+"-NA-1MSG\"\n")
		fic.write("Net.WGW["+str(j)+"].agreg=true\n")#changer pour false
		fic.write("Net.WGW["+str(j)+"].battery="+str(23760000)+"\n")
		i=i+1
		j=j+1
	k = 0
	while k < nbin:
		fic.write("Net.IN["+str(k)+"].id="+str(i)+"\n")
		fic.write("Net.IN["+str(k)+"].file=\""+filename+"-NA-1MSG\"\n")
		fic.write("Net.IN["+str(k)+"].battery="+str(23760000)+"\n")
		k=k+1
		i=i+1
	fic.close()



#FIN 1 MESSAGES PAR JOUR



#DEBUT 2 MESSAGES PAR JOUR 12 h

	# AGREGATION
	#Making file omnetpp.ini
	file_written=file_written+1
	#print "omnetppini/"+filename+"-2MSG.ini"
	fic=open("omnetppini/"+filename+"-2MSG.ini","w")
	fic.write("[General]\n")
	fic.write("user-interface=Cmdenv\n")
	fic.write("network = lora.simulations.Net\n")
	fic.write("sim-time-limit=86400s\n")
	fic.write("ned-path=/Users/joffreyherard/Documents/Master_2/Projet_fin_etude/dev/src/src;/Users/joffreyherard/Documents/Master_2/Projet_fin_etude/dev/src/simulations\n")
	i = 0
	while i < nbwell:
		fic.write("Net.WELL["+str(i)+"].id="+str(i)+"\n")
		fic.write("Net.WELL["+str(i)+"].slot="+str(43200)+"\n") #changer pour nb message
		fic.write("Net.WELL["+str(i)+"].file=\""+filename+"-2MSG\"\n")
		i=i+1
	j = 0
	while j < nbgw:
		fic.write("Net.WGW["+str(j)+"].id="+str(i)+"\n")
		fic.write("Net.WGW["+str(j)+"].file=\""+filename+"-2MSG\"\n")
		fic.write("Net.WGW["+str(j)+"].agreg=true\n")#changer pour false
		fic.write("Net.WGW["+str(j)+"].battery="+str(23760000)+"\n")
		i=i+1
		j=j+1
	k = 0
	while k < nbin:
		fic.write("Net.IN["+str(k)+"].id="+str(i)+"\n")
		fic.write("Net.IN["+str(k)+"].file=\""+filename+"-2MSG\"\n")
		fic.write("Net.IN["+str(k)+"].battery="+str(23760000)+"\n")
		k=k+1
		i=i+1
	fic.close()

	file_written=file_written+1
	#print "omnetppini/"+filename+"-NA-2MSG.ini"
	fic=open("omnetppini/"+filename+"-NA-2MSG.ini","w")
	fic.write("[General]\n")
	fic.write("user-interface=Cmdenv\n")
	fic.write("network = lora.simulations.Net\n")
	fic.write("sim-time-limit=86400s\n")
	fic.write("ned-path=/Users/joffreyherard/Documents/Master_2/Projet_fin_etude/dev/src/src;/Users/joffreyherard/Documents/Master_2/Projet_fin_etude/dev/src/simulations\n")
	i = 0
	while i < nbwell:
		fic.write("Net.WELL["+str(i)+"].id="+str(i)+"\n")
		fic.write("Net.WELL["+str(i)+"].slot="+str(43200)+"\n") #changer pour nb message
		fic.write("Net.WELL["+str(i)+"].file=\""+filename+"-NA-2MSG\"\n")
		i=i+1
	j = 0
	while j < nbgw:
		fic.write("Net.WGW["+str(j)+"].id="+str(i)+"\n")
		fic.write("Net.WGW["+str(j)+"].file=\""+filename+"-NA-2MSG\"\n")
		fic.write("Net.WGW["+str(j)+"].agreg=true\n")#changer pour false
		fic.write("Net.WGW["+str(j)+"].battery="+str(23760000)+"\n")
		i=i+1
		j=j+1
	k = 0
	while k < nbin:
		fic.write("Net.IN["+str(k)+"].id="+str(i)+"\n")
		fic.write("Net.IN["+str(k)+"].file=\""+filename+"-NA-2MSG\"\n")
		fic.write("Net.IN["+str(k)+"].battery="+str(23760000)+"\n")
		k=k+1
		i=i+1
	fic.close()



#FIN 2 MESSAGES PAR JOUR




#DEBUT 1 MESSAGES PAR HEURE

	# AGREGATION
	#Making file omnetpp.ini
	file_written=file_written+1
	#print "omnetppini/"+filename+"-24MSG.ini"
	fic=open("omnetppini/"+filename+"-24MSG.ini","w")
	fic.write("[General]\n")
	fic.write("user-interface=Cmdenv\n")
	fic.write("network = lora.simulations.Net\n")
	fic.write("sim-time-limit=86400s\n")
	fic.write("ned-path=/Users/joffreyherard/Documents/Master_2/Projet_fin_etude/dev/src/src;/Users/joffreyherard/Documents/Master_2/Projet_fin_etude/dev/src/simulations\n")
	i = 0
	while i < nbwell:
		fic.write("Net.WELL["+str(i)+"].id="+str(i)+"\n")
		fic.write("Net.WELL["+str(i)+"].slot="+str(3600)+"\n") #changer pour nb message
		fic.write("Net.WELL["+str(i)+"].file=\""+filename+"-24MSG\"\n")
		i=i+1
	j = 0
	while j < nbgw:
		fic.write("Net.WGW["+str(j)+"].id="+str(i)+"\n")
		fic.write("Net.WGW["+str(j)+"].file=\""+filename+"-24MSG\"\n")
		fic.write("Net.WGW["+str(j)+"].agreg=true\n")#changer pour false
		fic.write("Net.WGW["+str(j)+"].battery="+str(23760000)+"\n")
		i=i+1
		j=j+1
	k = 0
	while k < nbin:
		fic.write("Net.IN["+str(k)+"].id="+str(i)+"\n")
		fic.write("Net.IN["+str(k)+"].file=\""+filename+"-24MSG\"\n")
		fic.write("Net.IN["+str(k)+"].battery="+str(23760000)+"\n")
		k=k+1
		i=i+1
	fic.close()

	file_written=file_written+1
	#print "omnetppini/"+filename+"-NA-24MSG.ini"
	fic=open("omnetppini/"+filename+"-NA-24MSG.ini","w")
	fic.write("[General]\n")
	fic.write("user-interface=Cmdenv\n")
	fic.write("network = lora.simulations.Net\n")
	fic.write("sim-time-limit=86400s\n")
	fic.write("ned-path=/Users/joffreyherard/Documents/Master_2/Projet_fin_etude/dev/src/src;/Users/joffreyherard/Documents/Master_2/Projet_fin_etude/dev/src/simulations\n")
	i = 0
	while i < nbwell:
		fic.write("Net.WELL["+str(i)+"].id="+str(i)+"\n")
		fic.write("Net.WELL["+str(i)+"].slot="+str(3600)+"\n") #changer pour nb message
		fic.write("Net.WELL["+str(i)+"].file=\""+filename+"-NA-24MSG\"\n")
		i=i+1
	j = 0
	while j < nbgw:
		fic.write("Net.WGW["+str(j)+"].id="+str(i)+"\n")
		fic.write("Net.WGW["+str(j)+"].file=\""+filename+"-NA-24MSG\"\n")
		fic.write("Net.WGW["+str(j)+"].agreg=true\n")#changer pour false
		fic.write("Net.WGW["+str(j)+"].battery="+str(23760000)+"\n")
		i=i+1
		j=j+1
	k = 0
	while k < nbin:
		fic.write("Net.IN["+str(k)+"].id="+str(i)+"\n")
		fic.write("Net.IN["+str(k)+"].file=\""+filename+"-NA-24MSG\"\n")
		fic.write("Net.IN["+str(k)+"].battery="+str(23760000)+"\n")
		k=k+1
		i=i+1
	fic.close()


#FIN 1 MESSAGES PAR HEURE




if(len(sys.argv) ==6):
	#Getting numbers of LoRaWAN gateways and LoRagateways and Isolated nodes
	lower_bound= int(sys.argv[1])
	upper_bound= int(sys.argv[2])
	max_degree_IN= int(sys.argv[3])
	max_degree_GW= int(sys.argv[4])
	nb_vertex= int(sys.argv[5])
	nb_graph=100
	current_degree_IN=1
	current_degree_GW=1
	while current_degree_IN < max_degree_IN+1:
		while current_degree_GW < max_degree_GW+1:
			i = 0
			while i < nb_graph:
				filename="graphe"+str(i)+"_IN_"+str(current_degree_IN)+"_GW_"+str(current_degree_GW)
				createGraph(lower_bound,upper_bound,filename,current_degree_IN,current_degree_GW,nb_vertex)
				i=i+1
			current_degree_GW=current_degree_GW+1
		current_degree_GW=1
		current_degree_IN=current_degree_IN+1
	print(str(file_written)+"files written")
else:
	print "6 arguments is needed, the right way to use this python script is :\n"
	print "python grapheGenerator.py <LOWER BOUND> <UPPER BOUND> <MAX IN PER GW> <MAX GW PER WELL> <MAX VERTEX>"
