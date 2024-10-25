import ia
import threading

# Width and height of the image seen by the camera
IMAGE_WIDTH = 640
IMAGE_HEIGHT = 480

# Center of the image seen by the camera
IMAGE_CENTER = [IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2]

# Size of the zone that is considered the center of the image
ERROR_CENTER = 50

# Distance considered safe for the AUV to approach
SAFE_DISTANCE = 1

ia = ia.Ia()
target_object = None
distance = None

detection_thread = threading.Thread(target=ia.update_data, daemon=True)
detection_thread.start()

def search_objects():
	"""
	Checks if objects were found. Found saved in target_object
	"""

	if ia.found_object():
		target_object = ia.greater_confidence_object()

while target_object is None:
	search_objects()
	
print(f"Target object is {target_object}")

print("Centering...")

xyxy = ia.get_xyxy(target_object)

is_center = False

while not is_center:
	xyxy = ia.get_xyxy(target_object)
	
	print(xyxy)