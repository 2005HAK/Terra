import cv2 as cv
import numpy as np

from func_utils import getOrientation

def detect_marker(image):

	# Angulo do path marker
	angle = None

	#Conversão para escala HSV
	hsv = cv.cvtColor(image, cv.COLOR_BGR2HSV)

	#Setando os intervalos de laranja
	lowr_orange = np.array([0,100,150])
	uppr_orange = np.array([100,255,255])

	#Mascara e contornos de identificação do path marker
	mask = cv.inRange(hsv,lowr_orange, uppr_orange)
	contours, _ = cv.findContours(mask, cv.RETR_TREE, cv.CHAIN_APPROX_SIMPLE)
	cv.drawContours(image, contours, 0, (0,255,0), 3)

	#Arrays para a criação do retângulo
	contours_poly = [None]*len(contours)

	#Laço para a construção do contoro
	for i, c in enumerate(contours):
		contours_poly[i] = cv.approxPolyDP(c, 3, True)

	#Laço para a criação do retângulo de menor área       
	for cnt in contours:
		rect =  cv.minAreaRect(cnt)
		box = cv.boxPoints(rect)
		box = np.int0(box)
		angle = getOrientation(cnt)

	#Construção do desenho, contorno do path marker
	for i in range(len(contours)):
		color = (0,255,0)
		cv.drawContours(image, contours_poly, i, color)

	#Comando para a construção do retângulo com base nos dados da [box] - Está dando erro em alguns casos
	#Para melhor funcionamento as imagens devem ser boas
	cv.drawContours(image,[box],0,(0,191,255),2)

	return image,angle