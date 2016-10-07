#!/usr/bin/python
import socket
import time
import operator
import numpy as np
from math import *
import matplotlib.pyplot as plt

# Data received at this port from beaglebone
Addr = ("",7891)

# Lambertian order
M_HP = 1.141065
M_LP = 25.62632

# T_HP = 9901.6122;
# T_HP = 20737.34333 	#lux
T_HP = 10288.7840373534 #adcval constants for Total Output power, area of pd
T_LP = 10647.3334 #??

fig, ax = plt.subplots()	
prev_d = 0.0
fig.set_size_inches(6,6)
m = M_HP #default
T = T_HP #default
#-------------------------------------------------------------------------------
def select_source(src):
	if src == 'h':
		m = M_HP
		T = T_HP
	else:
		m = M_LP
		T = T_LP

	return
#-------------------------------------------------------------------------------	
# Receives sensor values from BeagleBone
def sensor_read():
	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
	sock.bind(Addr)

	string = sock.recv(100).strip().rstrip('\x00') # buffer size is 20 bytes
	
	values = string.split();
	lval   = float (values[0])
	pval   = float (values[1])
	ornt   = float (values[2])

	return (lval, pval, ornt)
#-------------------------------------------------------------------------------	
def angle_guard(angle):
	if angle > 2*pi:
		return angle - (2*pi)
	elif angle < 0:
		return angle + (2*pi)
	else:
		return angle
#-------------------------------------------------------------------------------
# Calulcates angle of irradiation psi using
# 2 states S1 and S2 of the system
def calc_psi(S1, S2):
	t1   = S1[0]
	r1   = S1[1]
	RSS1 = S1[2]


	t2   = S2[0]
	r2   = S2[1]
	RSS2 = S2[2]

	deltaT = t1 -t2
	try:
		R =  RSS1/RSS2
	except Exception, e:
		return (-9999)
	diff = 9999

	angle = 0.0
	for psi_ref in np.arange(t1 - pi/2, t1 + pi/2, 0.01):
		psi_1  = angle_guard(psi_ref - t1)
		psi_2  = angle_guard(psi_ref - t2)
		# psi_2  = psi_1 + deltaT

		# rel1   = pi - t1 + r1
		# rel2   = pi - t2 + r2
		
		rel1   = angle_guard(r1 - t1)
		rel2   = angle_guard(r2 - t2 )

		# theta1 = psi_1 - rel1 +  pi
		# theta2 = psi_2 - rel2 +  pi

		theta1 = angle_guard(pi + psi_1 - rel1)
		theta2 = angle_guard(pi + psi_2 - rel2)
#
#		RSS1				cos^m(psi_1) * cos(theta_1)
#	  -------- 	 = 		 ---------------------------------
#		RSS2				cos^m(psi_2) * cos(theta_2)
#
		try :
			partA = pow(cos(psi_1)/cos(psi_2), m)
			partB = cos(theta1)/cos(theta2)
		except :
			pass

		temp =  partA * partB

		if temp > 0:
			cur_diff = abs(temp - R)
			if cur_diff < diff:
				diff = cur_diff
				angle = psi_ref

				frel1 = degrees(rel1)
				frel2 = degrees(rel2)
				fpsi1 = degrees(psi_1)
				fpsi2 = degrees(psi_2)
				ftheta1 = degrees(theta1)
				ftheta2 = degrees(theta2)
		#end if
	#end for
	
	print "rel1: ", frel1, " rel2: ", frel2, " fpsi1: ", fpsi1, " fpsi2:", fpsi2
	return angle_guard(angle)	
	# return angle		
#-------------------------------------------------------------------------------
def get_tx_ornt():
	# return radians(float(raw_input("Enter Tx orientation: ")))
	return radians(46.0) #Transmitter orientation fixed
#-------------------------------------------------------------------------------
def get_states():
	
	raw_input("Press [enter] to save State 1 ")
	
	lval, pval, ornt = sensor_read() 
	t =	get_tx_ornt()
	S1 = (t, radians(ornt), pval)
	print "State 1: ", S1

	raw_input("Press [enter] to save State 2 ")
	t =	get_tx_ornt()
	lval, pval, ornt = sensor_read() 
	S2 = (t, radians(ornt), pval)
	print "State 2: ", S2

	return (S1, S2)
#-------------------------------------------------------------------------------
def channel_loss(RSS, psi, theta):
	dsqr = T * (m + 1) * pow(cos(psi), m) * cos(theta)
	d = sqrt(dsqr/RSS)
	return d;	
#-------------------------------------------------------------------------------	
def calc_xy_components(d, irr):
	try:
		x = d * sin(irr)
		y = d * cos(irr)
	except Exception, e:
		print "ERROR:" + str(e)
		exit()

	return (x,y)
#-------------------------------------------------------------------------------	
def get_xy(S, psi_r):
	# irr	 = psi_r - S[0]
	# rel = pi - S[0] + S[1]
	# inc  = irr - rel
	irr = psi_r - S[0]
	rel = S[1] - S[0]
	inc  = pi + irr - rel
	RSS = S[2]

	try:
		d = channel_loss(RSS,irr,inc)
		# x = d * sin(irr)
		# y = d * cos(irr)

		x  = d * cos((pi/2) - irr)
		y  = d * sin((pi/2) - irr)
	except Exception, e:
		# print "ERROR: " + str(e)
		return (-999,-999,-999)

	return (d,x,y)	
#-------------------------------------------------------------------------------	
def rss_contour(S):
	tx_angle = S[0] 
	rx_angle = S[1]
	RSS      = S[2]

	H = sqrt((T * (m + 1)) /  RSS ) #ideal distance
	contour = []
	x = []
	y = []
	for alpha in np.arange(- pi/2, pi/2, 0.01):
		# phi = tx_angle + alpha
		phi = alpha
		# phi = (phi if phi < pi/2 else pi/2)
		# angles for incidence and irradiaion cannot be greater than 90
		rel = pi - tx_angle + rx_angle
		theta = phi - rel
		# theta = (theta if theta < pi/2 else pi/2)



		try:
			Wir = pow((cos(phi)), m)
			Win = cos(theta)
			val = sqrt(Wir * Win)
		except:
			val = 0
		d = abs(H * val)
		contour.append(d);
		xcorr, ycorr = calc_xy_components(d, phi)
		x.append(xcorr)
		y.append(ycorr)

	return (x, y)
#-------------------------------------------------------------------------------		
def position_plot(S, psi_r, c1, c2, d, px, py):
	global fig, ax
	# d, px, py = get_xy(S, psi_r)
	# if d == -999:
	# 	return
	# print px, py, degrees(psi_r)
	graph = fig.add_subplot(111)
	graph.plot(np.zeros(70), np.arange(0,70),'k--')
	plt.annotate(
				"Tx Normal",
				xy = (0,70),
				xytext = (0,10),
		        textcoords = 'offset points', ha = 'center', va = 'top')

	lx = []
	for ly in np.arange(0, py):
		lx.append(ly/tan(pi/2 - (psi_r - S[0])))

	graph.plot(lx, np.arange(0, py),'y.')
	# TX
	graph.scatter(0, 0, s=3.14*30, c='black', marker="^")
	plt.annotate(
				"Tx",
				xy = (0,0),
				xytext = (-30,-10),
		        textcoords = 'offset points', ha = 'center', va = 'bottom')

	# RX
	graph.scatter(px, py, s=3.14*30, c='green', marker="v", alpha=0.5)
	postr = '(%.2f'%(px) + ', ' + '%.2f)'%(py) 
	plt.annotate(
		    	"Rx " + postr, 
		        xy = (px, py), xytext = (0,15),
		        textcoords = 'offset points', ha = 'center', va = 'bottom')
		        # bbox = dict(boxstyle = 'round,pad=0.5', fc = 'yellow', alpha = 0.5),
		        # arrowprops = dict(arrowstyle = '-', connectionstyle = 'arc3,rad=0'))	


	cx = c1[0]
	cy = c1[1]	   
	graph.plot(cx, cy, 'r--')

	cx = c2[0]
	cy = c2[1]	   
	graph.plot(cx, cy, 'b--')


	graph.grid(True)
	graph.set_title("Positions")
	graph.set_xlabel("X")
	graph.set_ylabel("Y")
	
	plt.yticks(np.arange(0, 80, 10))
	plt.xticks(np.arange(-100, 100, 10))

	# plt.show()
	fig.canvas.draw()
	plt.pause(0.001)
	fig.clf()

#-------------------------------------------------------------------------------	
def read_state():
	lval, pval, ornt = sensor_read() 
	t =	get_tx_ornt()
	ornt = ornt + 90.0  # phone was placed at 90 degrees to the beaglebone
	S = (t, radians(ornt), pval)
	# print "State 1: ", S1
	return S
#-------------------------------------------------------------------------------	
def test():
	S1, S2 = get_states()
	psi_r  = calc_psi(S1,S2)
	# print psi_r
	position_plot(S2, psi_r)
#-------------------------------------------------------------------------------	
# Qualifier function to make ensure a minimum threshold for 
# change in orientation
def state_change_qualifier(S1, S2):
	threshold = radians(1) # threshold = 1 degree
	if (
		(abs(S1[0] - S2[0]) > threshold) 
		or (abs(S1[1] - S2[1]) > threshold) 
		):
		return True

	# elif abs(S2[2] - S1[2]) >0:
	# 	return True 
	
	else:
		return False

	# if (S2[2] - S1[2] != 0) :
	# 	return True
	# else:
	# 	return False

#-------------------------------------------------------------------------------
# A closed form expression for calculating psi
# Only to be used when transmitter is fixed
def fast_psi(S1, S2):
	t1   = S1[0]
	r1   = S1[1]
	RSS1 = S1[2]


	t2   = S2[0]
	r2   = S2[1]
	RSS2 = S2[2]

	rel1 = r1 - t1
	rel2 = r2 - t2

	try:
		# closed form expresssion for calculating psi
		K = RSS1/RSS2
		x = ((K * cos(rel2)) - cos(rel1))/(sin(rel1) - (K*sin(rel2)))
	except Exception,e:
		return -9999
	
	# print "K: ", K, " RSS1: ", RSS1, " RSS2: ", RSS2, " rel1: ", degrees(rel1), " rel2: ", degrees(rel2), " x:", degrees(atan(x))
	return (atan(x) + t1)
#-------------------------------------------------------------------------------	
def live_positions(S1):
	global prev_d
	# tor = float (raw_input("Read S2"))
	# print tor
	# tor = radians(tor)
	# print tor

	# t2 = S2[0] + tor
	# t2 = S2[0]
	# S2 = (t2, S2[1], S2[2])


	S2 = read_state()
	while not state_change_qualifier(S1, S2):
		S2 = read_state()

	# calculate irradiation angle psi
	psi_r = calc_psi(S1, S2)
	# psi_r = fast_psi(S1, S2)
	if psi_r != -9999:
		c1    = rss_contour(S1)
		c2    = rss_contour(S2)

		d, x, y = get_xy(S2, psi_r)
		if d == -999:
			return
		if d > 10:
			# print "psi_r: ", degrees(psi_r), "psi: ", degrees(psi_r - S2[0]), "dRSS: ", (S2[2] - S1[2]) 
			print degrees(psi_r), 'dRSS: ', S2[2] - S1[2]
			position_plot(S2, psi_r, c1, c2, d, x, y)
#-------------------------------------------------------------------------------	
def read_preamble():
	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
	sock.bind(Addr)

	string   = sock.recv(100).strip().rstrip('\x00') # buffer size is 20 bytes
	string   = [float(x) for x in string.split()]	
	ornt     = string[-1]
	preamble = string[:-1]

	symbol_high = preamble[::2]
	symbol_low  = preamble[1::2]

	rpwr    = map(operator.sub, symbol_high, symbol_low)
	avgpwr  = sum(rpwr) / 12.0

	sd  = np.std(rpwr)
	return (sd, ornt, avgpwr)
#-------------------------------------------------------------------------------	

select_source('h')
# test()

# State S = (t, r, R)
# t = transmitter orientation
# r = receiver orientation
# R = photodiode value(received power)

while True:
	S1 = read_state()
	live_positions(S1)

# exit()
# while True:
# 	sd, rxo, rss = read_preamble()
# 	if sd < 1.0:
# 	 print sd, rxo, rss