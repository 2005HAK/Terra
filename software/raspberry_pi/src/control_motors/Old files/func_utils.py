import cv2
import numpy as np
from math import atan2,pi

# Function that performs PCA analysis to determine the angle of a bounding box in respect to the horizontal
def getOrientation(pts):
	# Constructing a buffer used by the pca analysis
	sz = len(pts)
	data_pts = np.empty((sz, 2), dtype=np.float64)
	for i in range(data_pts.shape[0]):
		data_pts[i,0] = pts[i,0,0]
		data_pts[i,1] = pts[i,0,1]

	# Performing PCA analysis
	mean = np.empty((0))
	mean, eigenvectors, _ = cv2.PCACompute2(data_pts, mean)

	angle = atan2(eigenvectors[0,1], eigenvectors[0,0])*180/pi # orientation in degrees

	return angle

# Function to return the x coordinate of the bounding box center respective to each gate leg
def getCenterXBBox(contour):
	bounding_box = cv2.minAreaRect(contour)
	return bounding_box[0][0]

