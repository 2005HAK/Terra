## This script is used mainly to convert the returned coordinates by the vision scripts to control commands (like forward, up, left, right, turn and so on)
## There is an additional function that checks if the image and object centers are already aligned

# This function is used to align the image center with the gate center, and send the string command to the AUV navigation script
# Parameters: Coordinates of the center of the gate in image, ratio, number of poles detected in image, current status of the mission (3 stages in this mission)

## Tradução
## Este script é usado principalmente para converter as coordenadas retornadas pelos scripts de visão para controlar comandos (como avançar, subir, esquerda, direita, virar e assim por diante)
## Existe uma função adicional que verifica se os centros da imagem e do objeto já estão alinhados

## Esta função é usada para alinhar o centro da imagem com o centro do portão e enviar o comando string para o script de navegação AUV
## Parâmetros: Coordenadas do centro do portão na imagem, proporção, número de pólos detectados na imagem, estado atual da missão (3 etapas nesta missão)

def alignGate(center_gate_x,center_gate_y,ratio,num_legs_found,current_status,delta_dims = 30):

	img_height = 720
	img_width = 1280
	action = ""

	if current_status == 1: # Align the AUV to the gate center (Alinhe o AUV ao centro do portão)
		if ratio >= 0.70 and ratio <= 0.90:
			action = "TURN RIGHT + LEFT"
		elif ratio >= 1.10 and ratio <= 1.50:
			action = "TURN LEFT + RIGHT"
		elif (ratio >= 0.90 and ratio <= 1.10) or num_legs_found >= 2:
			if center_gate_x >= 0 and center_gate_x < (img_width/2 - delta_dims):
				if center_gate_y >= 0 and center_gate_y < (img_height/2 - delta_dims):
					action = "FWD + LEFT + UP"
				elif center_gate_y >= (img_height/2 - delta_dims) and center_gate_y < (img_height/2 + delta_dims):
					action = "FWD + LEFT"
				elif center_gate_y >= (img_height/2 + delta_dims) and center_gate_y <= img_height:
					action = "FWD + LEFT + DOWN"
			elif center_gate_x >= (img_width/2 - delta_dims) and center_gate_x < (img_width/2 + delta_dims):
				if center_gate_y >= 0 and center_gate_y < (img_height/2 - delta_dims):
					action = "FWD + UP"
				elif center_gate_y >= (img_height/2 - delta_dims) and center_gate_y < (img_height/2 + delta_dims):
					action = "FWD"
				elif center_gate_y >= (img_height/2 + delta_dims) and center_gate_y <= img_height:
					action = "FWD + DOWN"
			elif center_gate_x >= (img_width/2 + delta_dims) and center_gate_x <= img_width:
				if center_gate_y >= 0 and center_gate_y < (img_height/2 - delta_dims):
					action = "FWD + RIGHT+ UP"
				elif center_gate_y >= (img_height/2 - delta_dims) and center_gate_y < (img_height/2 + delta_dims):
					action = "FWD + RIGHT"
				elif center_gate_y >= (img_height/2 + delta_dims) and center_gate_y <= img_height:
					action = "FWD + RIGHT + DOWN"
	elif current_status == 2: # Align the AUV to the center of the right side in the gate (Alinhe o AUV ao centro do lado direito do portão)
		if center_gate_x >= 0 and center_gate_x < (img_width/2 - delta_dims):
			if center_gate_y >= 0 and center_gate_y < (img_height/2 - delta_dims):
				action = "LEFT + UP"
			elif center_gate_y >= (img_height/2 - delta_dims) and center_gate_y < (img_height/2 + delta_dims):
				action = "LEFT"
			elif center_gate_y >= (img_height/2 + delta_dims) and center_gate_y <= img_height:
				action = "LEFT + DOWN"
		elif center_gate_x >= (img_width/2 - delta_dims) and center_gate_x < (img_width/2 + delta_dims):
			if center_gate_y >= 0 and center_gate_y < (img_height/2 - delta_dims):
				action = "UP"
			elif center_gate_y >= (img_height/2 - delta_dims) and center_gate_y < (img_height/2 + delta_dims):
				action = "FWD"
			elif center_gate_y >= (img_height/2 + delta_dims) and center_gate_y <= img_height:
				action = "DOWN"
		elif center_gate_x >= (img_width/2 + delta_dims) and center_gate_x <= img_width:
			if center_gate_y >= 0 and center_gate_y < (img_height/2 - delta_dims):
				action = "RIGHT+ UP"
			elif center_gate_y >= (img_height/2 - delta_dims) and center_gate_y < (img_height/2 + delta_dims):
				action = "RIGHT"
			elif center_gate_y >= (img_height/2 + delta_dims) and center_gate_y <= img_height:
				action = "RIGHT + DOWN"
	elif current_status == 3: # AUV already aligned with respect to the right side, just going forward to pass through the gate (AUV já alinhado em relação ao lado direito, apenas avançando para passar pelo portão)
		action = "FWD"

	return action

# Function that send controls to align the AUV regarding to the bottom marker in the pool

# Função que envia controles para alinhar o AUV em relação ao marcador inferior da piscina
def align_marker(angle):
	
	action = ""

	if angle <= 85.0:
		action = "ROLL COUNTER_CW"
	elif angle >= 95.0:
		action = "ROLL CW"
	
	return action

# This function prepare the action necessary to the auv to align in respect with the chosen buoy

# Esta função prepara a ação necessária para que o auv se alinhe em relação à bóia escolhida
def align_buoy(center_x,center_y,delta_dims = 30):

	img_height = 720
	img_width = 1280
	action = ""

	if center_x >= 0 and center_x < (img_width/2 - delta_dims):
		if center_y >= 0 and center_y < (img_height/2 - delta_dims):
			action = "LEFT + UP"
		elif center_y >= (img_height/2 - delta_dims) and center_y < (img_height/2 + delta_dims):
			action = "LEFT"
		elif center_y >= (img_height/2 + delta_dims) and center_y <= img_height:
			action = "LEFT + DOWN"
	elif center_x >= (img_width/2 - delta_dims) and center_x < (img_width/2 + delta_dims):
		if center_y >= 0 and center_y < (img_height/2 - delta_dims):
			action = "UP"
		elif center_y >= (img_height/2 - delta_dims) and center_y < (img_height/2 + delta_dims):
			action = "FWD"
		elif center_y >= (img_height/2 + delta_dims) and center_y <= img_height:
			action = "DOWN"
	elif center_x >= (img_width/2 + delta_dims) and center_x <= img_width:
		if center_y >= 0 and center_y < (img_height/2 - delta_dims):
			action = "RIGHT+ UP"
		elif center_y >= (img_height/2 - delta_dims) and center_y < (img_height/2 + delta_dims):
			action = "RIGHT"
		elif center_y >= (img_height/2 + delta_dims) and center_y <= img_height:
			action = "RIGHT + DOWN"
	
	return action

# Function that checks if the center of some object in the image is already in the image center (Dentro de uma região de 80x80 pixels)
# delta_x and delta_y determine the sizes of each dimension of the acceptable center in image (dim_x = 2*delta_x, dim_y = 2*delta_y)

# Função que verifica se o centro de algum objeto da imagem já está no centro da imagem (Dentro de uma região de 80x80 pixels)
# delta_x e delta_y determinam os tamanhos de cada dimensão do centro aceitável na imagem (dim_x = 2*delta_x, dim_y = 2*delta_y)

def checkCenter(center_x,center_y,delta_x=30,delta_y=30):

	img_height = 720
	img_width = 1280

	if center_x >= (img_width/2 - delta_x) and center_x <= (img_width/2 + delta_x) and center_y >= (img_height/2 - delta_y) and center_y <= (img_height/2 + delta_y):
		return True
	else:
		return False