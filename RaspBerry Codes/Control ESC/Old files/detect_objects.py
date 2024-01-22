# Import packages
import time
import os
import cv2
import numpy as np
import tensorflow as tf

import label_map_util

# Necessário instalar object detection API para executar script

# Grab path to current working directory
CWD_PATH = os.getcwd()

# Path to label map file
PATH_TO_LABELS = os.path.join(CWD_PATH,'training','labelmap.pbtxt')

# Number of classes the object detector can identify
NUM_CLASSES = 4

# Load the label map.
# Label maps map indices to category names, so that when our convolution
# network predicts `5`, we know that this corresponds to `king`.
# Here we use internal utility functions, but anything that returns a
# dictionary mapping integers to appropriate string labels would be fine
label_map = label_map_util.load_labelmap(PATH_TO_LABELS)
categories = label_map_util.convert_label_map_to_categories(label_map, max_num_classes=NUM_CLASSES, use_display_name=True)
category_index = label_map_util.create_category_index(categories)

def load_model(path_to_model):
    start = time.time()
    tf.keras.backend.clear_session()
    model = tf.saved_model.load(path_to_model)
    end = time.time()
    print("Tempo de carregamento modelo: ",end-start," s")
    
    return model

def detect_object(image,model,target="",min_score=0.4):
    image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
    input_tensor = np.expand_dims(image_rgb, axis=0)

    detections = model(input_tensor)

    boxes = detections['detection_boxes'].numpy()[0]
    max_boxes_to_draw = boxes.shape[0]
    scores = detections['detection_scores'].numpy()[0]

    coordinates = []
    for i in range(min(max_boxes_to_draw, boxes.shape[0])):
        class_id = int(detections['detection_classes'].numpy()[0][i])
        if scores[i] > min_score and category_index[class_id]["name"] == target:
            coordinates.append({
                "box": boxes[i],
                "class_name": category_index[class_id]["name"],
                "score": scores[i]
            })

    return coordinates

def plot_detection_results(image,detections,target):

    height = image.shape[0]
    width = image.shape[1]
    offset_text_y = 10

    for i in range(len(detections)):
        if detections[i]["class_name"] == target:
            point_1_rect = (int(width*detections[i]["box"][1]),int(height*detections[i]["box"][0]))
            point_2_rect = (int(width*detections[i]["box"][3]),int(height*detections[i]["box"][2]))
            cv2.rectangle(image,point_1_rect,point_2_rect,(255,0,0),2)
            x_text = int(width*detections[i]["box"][1])
            y_text = int(height*detections[i]["box"][0])-offset_text_y
            cv2.putText(image,detections[i]["class_name"] + " " + str(round(detections[i]["score"]*100,2)) + "%",(x_text,y_text),cv2.FONT_HERSHEY_SIMPLEX,0.7,(255,0,0),2,cv2.LINE_AA)

    return image