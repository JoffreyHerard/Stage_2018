#! /usr/bin/python

from igraph import *
import sys
import random

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
def createGraph(lower_bound,upper_bound,filename):
	#Random number is the probability which have a LoraGateway to have one isolated node
	p=random.random()
	#Random number is the probability which have a LoraGateway to have another isolated node
	p2=random.random()
	current_graph=0
	nb_max_vertex=1000
	nb_degree_max_IN=4
	nb_degree_max_GW=4
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
		current_well=nb_vertex
		#on augmente le nombre de sommet actuel
		nb_vertex=nb_vertex+1
		nbwell=1+nbwell
		#Tant que nb_noeud < 1000 ET degre_actuel < degre max ET iteration< degre max faire
		actual_degree_GW=0
		nb_loop_GW=0
		while nb_vertex <nb_max_vertex and actual_degree_GW < nb_degree_max_GW and nb_loop_GW < nb_degree_max_GW:
			# x <- tirer probabilite
			current_gw=0
		 	x=random.random()
			#Si x > p1
			if x > p :
				names.append("GW")
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
				while nb_vertex <nb_max_vertex and actual_degree_IN < nb_degree_max_IN and nb_loop_IN < nb_degree_max_IN:
					# y <- tirer probabilite
					current_in=0
				 	y=random.random()
					#Si x > p2%
					if y > p2 :
						nbin=nbin+1
						names.append("IN")
						actual_degree_IN=actual_degree_IN+1
						#on repertorie l'id de l'IN
						id_in.append(nb_vertex)
						#rajouter IN
						g.add_vertices(1)
						current_in=nb_vertex
						nb_vertex=nb_vertex+1
						#cree connection entre GW et IBN
						g.add_edges([(current_gw,current_in)])
					nb_loop_IN=nb_loop_IN+1
			nb_loop_GW=nb_loop_GW+1
	g.vs["label"]=names
	g.es[0]
	#g.write_dot(filename+".dot")
	#Making file .ned
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
	#Making file omnetpp.ini
	fic=open("omnetppini/"+filename+".ini","w")
	fic.write("[General]\n")
	fic.write("network = lora.simulations.Net\n")
	fic.write("sim-time-limit=1000s\n")
	i = 0
	while i < nbwell:
		fic.write("Net.WELL["+str(i)+"].id="+str(i)+"\n")
		i=i+1
	j = 0
	while j < nbgw:
		fic.write("Net.WGW["+str(j)+"].id="+str(i)+"\n")
		i=i+1
		j=j+1
	k = 0
	while k < nbin:
		fic.write("Net.IN["+str(k)+"].id="+str(i)+"\n")
		k=k+1
		i=i+1
	fic.close()

if(len(sys.argv) ==3):
	#Getting numbers of LoRaWAN gateways and LoRagateways and Isolated nodes
	lower_bound= int(sys.argv[1])
	upper_bound= int(sys.argv[2])
	nb_graph=100
	i = 0
	while i < nb_graph:
		filename="graphe"+str(i)
		createGraph(lower_bound,upper_bound,filename)
		i=i+1
	print("JOB FINISHED")
else:
	print "3 arguments is needed, the right way to use this python script is :\n"
	print "python grapheGenerator.py <LOWER BOUND> <UPPER BOUND>"
