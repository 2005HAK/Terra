def extract_boxes(data_received):
    return data_received["boxes"]

def med_point(xyxy = []):
    """
    param: x and y coords. of detected object
    return: x and y coords. as a list of medium point or [-1, -1] if param is null
    """
    return [(xyxy[0] + xyxy[2]) / 2, (xyxy[1] + xyxy[3]) / 2] if xyxy else [-1, -1]

def mov_rel_pos(xyxy):
    dir1 = ""
    dir2 = ""

    xm, ym = med_point(xyxy)

    for i in range(10):
        if(xm < 615):
            dir1 = "LEFT"
        elif(xm > 665):
            dir1 = "RIGHT"
        if(ym < 335):
            dir2 = "UP"
        elif(ym > 385):
            dir2 = "DOWN"
        else:
            break

    return [dir1, dir2]

x1, y1, x2, y2 = extract_boxes({"boxes": [0,0,3,4]})

print(mov_rel_pos([x1, y1]))