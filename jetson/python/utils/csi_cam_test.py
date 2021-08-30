# MIT License
# Copyright (c) 2019 JetsonHacks
# See license
# Using a CSI camera (such as the Raspberry Pi Version 2) connected to a
# NVIDIA Jetson Nano Developer Kit using OpenCV
# Drivers for the camera and OpenCV are included in the base image

import cv2
import gi
import numpy as np

# gstreamer_pipeline returns a GStreamer pipeline for capturing from the CSI camera
# Defaults to 1280x720 @ 60fps
# Flip the image by setting the flip_method (most common values: 0 and 2)
# display_width and display_height determine the size of the window on the screen
gi.require_version("Gst", "1.0")
from gi.repository import GObject, Gst
Gst.init(None)


def gstreamer_pipeline(
    sensor_id=0,
    capture_width=1280,
    capture_height=720,
    display_width=1280,
    display_height=720,
    framerate=60,
    flip_method=0,
):
    return ("nvarguscamerasrc sensor_id=0 ! video/x-raw(memory:NVMM),width=1280, height=720, framerate=60/1, format=NV12 ! \
        nvvidconv ! video/x-raw,width=1280, height=720, format=BGRx ! \
        videoconvert ! video/x-raw, format=BGR ! appsink sync=false max-buffers=2 drop=true name=sink emit-signals=true")
    # return (
    #     "nvarguscamerasrc sensor_id=%d ! "
    #     "video/x-raw(memory:NVMM), "
    #     "width=%d, height=%d, "
    #     "format=NV12, framerate=(fraction)%d/1 ! "
    #     "nvvidconv flip-method=%d ! "
    #     "video/x-raw, width=(int)%d, height=(int)%d, format=(string)BGRx ! "
    #     "videoconvert ! "
    #     "video/x-raw, format=(string)BGR ! appsink"
    #     % (
    #         sensor_id,
    #         capture_width,
    #         capture_height,
    #         framerate,
    #         flip_method,
    #         display_width,
    #         display_height,
    #     )
    # )

def gst_to_opencv(sample):
    buf = sample.get_buffer()
    caps = sample.get_caps()

    # print(caps.get_structure(0).get_value("format"))
    # print(caps.get_structure(0).get_value("height"))
    # print(caps.get_structure(0).get_value("width"))

    # print(buf.get_size())

    arr = np.ndarray(
        (
            caps.get_structure(0).get_value("height"),
            caps.get_structure(0).get_value("width"),
            3,
        ),
        buffer=buf.extract_dup(0, buf.get_size()),
        dtype=np.uint8,
    )
    return arr

def show_camera():
    # To flip the image, modify the flip_method parameter (0 and 2 are the most common)
    gst_cmd = gstreamer_pipeline()
    print(gst_cmd)
    pipeline = Gst.parse_launch(gst_cmd)

    sink = pipeline.get_by_name("sink")
    pipeline.set_state(Gst.State.PLAYING)

    while True:
        sample = sink.emit("pull-sample")
        # ret, frame = cap.read()
        if not sample:
            continue
            # self._log.error("GST read error")
        else:
            # This optional function is given by the user. default is identity x->x

            new_frame = gst_to_opencv(sample)

            cv2.imshow("frame", new_frame)
            if cv2.waitKey(1) & 0xFF == ord("q"):
                break


if __name__ == "__main__":
    show_camera()