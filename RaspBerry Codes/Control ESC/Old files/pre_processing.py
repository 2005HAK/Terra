import cv2
import numpy as np

# This function performs the gamma correction in a image passed as argument. This technique helps in the contrast and sharpness enhancement
def gammaCorrection(image,gamma):
    invGamma = 1.0/gamma

    lookup_table = np.array([((i/255.0) ** invGamma)*255 for i in np.arange(0,256)]).astype("uint8")
    image = cv2.LUT(image,lookup_table)
    
    return image

# This function executes the CLAHE (Contrast Limited Adjust Histogram Equalization) proccess in the image passed as parameter
def clahe(image,clip_limit,grid_size):
    clahe = cv2.createCLAHE(clipLimit=clip_limit,tileGridSize=(grid_size,grid_size))
    
    for i in range(3): # Iterating by all three color channels doing the CLAHE proccess (channels B,G and R)
        image[:,:,i] = clahe.apply((image[:,:,i]))

    return image