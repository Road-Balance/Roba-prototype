import serial
import argparse

from time import sleep

ap = argparse.ArgumentParser()
ap.add_argument("-p", "--port", type=str, help="path to optional input video file")
args = vars(ap.parse_args())
print(args)

if args["port"] is None:
    print("Port Name required")
    print("Exit Process...")
    exit(1)

try:
    send_period = 0.5
    msg_header = bytes([255, 1])
    print(msg_header)

    with serial.Serial(args["port"], 115200, timeout=10) as ser:
        while True:
            AX_1X = 129
            AX_1Y = 129
            AX_2X = 129
            AX_2Y = 254
            # steering = input("type steering : ")
            # send_msg = throttle + "," + steering + "," + "\n"
            # send_msg_bytes = msg_header + send_msg.encode("utf-8")
            send_msg = bytes([AX_1X, AX_1Y, AX_2X, AX_2Y])
            send_msg = msg_header + send_msg
            print(send_msg)

            # ser.write(bytes(send_msg, "utf-8"))
            ser.write(send_msg)

            sleep(send_period)
except Exception as e:
    print(e)
