import cv2

capture = cv2.VideoCapture(0)
capture.set(cv2.CAP_PROP_FRAME_WIDTH, 3840)
capture.set(cv2.CAP_PROP_FRAME_HEIGHT, 2160)

while cv2.waitKey(1) < 0:
    ret, frame = capture.read()
    if not ret:
        print("CV read error %s", str(ret))

    cv2.imshow("VideoFrame", frame)

capture.release()
cv2.destroyAllWindows()
