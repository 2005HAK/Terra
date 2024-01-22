import time
import numpy as np
import cv2

from func_utils import getOrientation,getCenterXBBox
from auv_control import alignGate,checkCenter
from pre_processing import gammaCorrection,clahe
from detect_objects import detect_object,plot_detection_results

# Parameters: image - array containing the image data acquired by the AUV
#             status - integer that indicates in which states is the detect gate task

# Returns: action - String containing the direction that the AUV has to move to align the gate with the image center
#          orig_image - image containing the bounding boxes around the detected gate legs, and the gate center point (red circle)
#          status - updated status after the code execution
def detect_gate(image,model,status):
	# Aferição do tempo de inicio da inspeção
	start = time.time()

	# Imagem original - copiada para fins de visualização
	orig_image = image.copy()
	#cv2.imshow("Original Image",image)

	# Aplicação de correção Gamma na imagem
	GammaValue = 1.70 # default - 1.70
	image = gammaCorrection(image,GammaValue)

	# Aplicação de melhoria por CLAHE (Contrast Limited Adjust Histogram Equalization) na imagem (em cada canal de cor)
	clipLim = 2 # default - 2
	gridSize = 5 # default - 5
	image = clahe(image,clipLim,gridSize)
	#cv2.imshow("Image after corrections",image)

	# Aplicando Gaussian Blur para remover ruidos de dimensões pequenas na imagem
	image = cv2.GaussianBlur(image,(5,5),0)

	# Convertendo imagem de BGR para HSV
	image = cv2.cvtColor(image,cv2.COLOR_BGR2HSV)

	# Separando imagem segundo canal HSV, e realizando segmentação pela cor laranja (e tons próximos)
	# Valores anteriores de H - Low: 0; High: 75 (com imagens filtradas)

	low = np.array([5,10,10]) # default - 5,10,10
	high = np.array([70,150,150]) # default - 70, 150, 150

	mask = cv2.inRange(image,low,high)
	result = cv2.bitwise_and(image,image,mask=mask)

	# Transformando imagem de BGR para tons de cinza (gray), e apos isso aplicando binarizacao
	gray = cv2.cvtColor(result,cv2.COLOR_HSV2BGR)
	gray = cv2.cvtColor(result,cv2.COLOR_BGR2GRAY)
	_,thresh = cv2.threshold(gray,30,255,cv2.THRESH_BINARY)

	# Encontrando linhas verticais das barras do portal
	kernel = cv2.getStructuringElement(cv2.MORPH_RECT,(3,3))
	eroded = cv2.erode(thresh,kernel,iterations=1)
	dilated = cv2.dilate(eroded,kernel,iterations=2)

	# Formando linhas solidas das pernas e gerando contornos
	kernel = cv2.getStructuringElement(cv2.MORPH_RECT,(5,5))
	close = cv2.morphologyEx(dilated,cv2.MORPH_CLOSE,kernel)

	# Extrai contornos a partir de imagem filtrada
	contours,_ = cv2.findContours(close,cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_SIMPLE)

	# Define quantos contornos vao ser utilizados para a detecção baseado no status atual da tarefa
	number_poles = 2

	# Filtrando contornos de acordo com aspect ratio (W/L) e pela area 
	contours = [c for c in contours if (max(cv2.minAreaRect(c)[1])/min(cv2.minAreaRect(c)[1]) > 5.0 and cv2.contourArea(c) > 500.0 and getOrientation(c) >= 75.0 and getOrientation(c) <= 105.0)]
	contours_sorted = sorted(contours,key=cv2.contourArea,reverse=True)[:number_poles]

	# Numero de contornos detectados (relativos as pernas do portal)
	number_contours_found = len(contours_sorted)

	# Variaveis que definem o centro do portal na imagem (x,y) e a orientação do AUV em relação ao portal
	center_gate_x = 0
	center_gate_y = 0
	ratio = 0.0
	aligning_center_gate_to_image = False
	action = ""

	# Contador que define em qual loop do contorno estamos atualmente (para detectar coordenadas centrais do portal apenas com dados das duas pernas maiores)
	counter = 0

	# Se ainda não se alinhou com o centro do lado correto do portal
	if status <= 2:
		# Se está nos estágios de alinhamento das coordenadas da imagem com centro da imagem
		if len(contours_sorted) > 0:
			for c in contours_sorted:
				# Extraindo retangulo de area minima, relativo a cada contorno detectado
				rect = cv2.minAreaRect(c)
				
				#Extraindo dados do retangulo encontrado
				(center),_,_ = rect

				# Coordenadas dos vertices do retangulo - Ordem dos pontos de box: bottom left, top left, top right, bottom right
				box = cv2.boxPoints(rect)

				# Conversao de float para int das coordenadas do retangulo obtido
				box = np.int0(box)

				# Desenhando na imagem o retangulo obtido (contido na variavel box)
				cv2.drawContours(orig_image,[box],0,(0, 255, 0),2)

				# Acumulando valor do centro de perna do portal
				if counter <= 1:
					center_gate_x += center[0]
					center_gate_y += center[1]

				counter += 1

		# Verificações sobre numero de pernas do portal detectados
		# Encontrando as coordenadas centrais do portal (x,y)
		if len(contours_sorted) == 2:
			# Ordenando contornos de acordo com valor de center_x (contornos com valor crescente da esquerda para a direita)
			contours_sorted.sort(key=getCenterXBBox)
			left_leg = cv2.minAreaRect(contours_sorted[0])
			right_leg = cv2.minAreaRect(contours_sorted[1])

			# Ratio representa a proporção entre o comprimento das pernas do portal detectadas, com um valor abaixo de 0.9 ou acima de 1.1 podendo representar que o AUV esta angulado em relacao ao portal
			ratio = max(left_leg[1])/max(right_leg[1])

			center_gate_x /= 2 # center_x
			center_gate_y /= 2 # center_y

			if status == 1:

				if left_leg[1][0]*left_leg[1][1] >= 3000.0 and right_leg[1][0]*right_leg[1][1] >= 3000.0:
					if checkCenter(center_gate_x,center_gate_y) == True:
						status = 2
						aligning_center_gate_to_image = False
					else:
						aligning_center_gate_to_image = True
						action = alignGate(center_gate_x,center_gate_y,ratio,number_contours_found,2,20) # Forçando status = 2 para realização do alinhamento com centro da imagem
			else:

				detections = detect_object(orig_image,model,"G_Man", 0.4)
				orig_image = plot_detection_results(orig_image,detections,"G_Man")
				if len(detections) > 0:
					width = orig_image.shape[1]
					center_gate_x = int((width*detections[0]["box"][1]+width*detections[0]["box"][3])/2) # novo centro do portal em x considerando o personagem detectado

				if checkCenter(center_gate_x,center_gate_y) == True:
					status = 3	

		# Desenhando circulo em coordenada de centro do portal na imagem
		cv2.circle(orig_image,[int(center_gate_x),int(center_gate_y)],5,(0,0,255),2)

	# Exibindo na tela o status atual da tarefa de detecção e alinhamento com o portal
	if status == 1:
		print("Alinhando AUV com centro do portal!")
	elif status == 2:
		print("Alinhando AUV com centro do lado correto do portal!")
	elif status == 3:
		print("Navgando para frente para passar pelo portal!")

	# Analise do posicionamento do centro do portal em relação à imagem, para envio de string adequada ao script de controle
	if aligning_center_gate_to_image == False:
		action = alignGate(center_gate_x,center_gate_y,ratio,number_contours_found,status,20)
	print("Action: ",action)

	# Aferição do tempo final da inspeção
	end = time.time()
	print("Time: ",end-start, "s") # Exibicao do tempo de processamento do script para essa imagem
	print("-------------------------------\n")

	return action, orig_image, status