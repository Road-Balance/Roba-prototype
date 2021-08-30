import serial
import asyncio
import argparse

from time import sleep

ap = argparse.ArgumentParser()
ap.add_argument("-p", "--port", type=str, help="path to optional input video file")
args = vars(ap.parse_args())
print(args)


class TeensySender(object):
    def __init__(self, port_name="/dev/ttyACM0", baud_rate=115200, pub_period=0.1):
        super().__init__()

        self._baud_rate = baud_rate
        self._pub_period = pub_period
        self._msg_header = bytes([255, 1])

        self._ser = serial.Serial(port_name, self._baud_rate, timeout=10)
        self._loop = asyncio.get_event_loop()

    def __del__(self):
        pass

    def set_axis_val(self, AX_1X, AX_1Y, AX_2X, AX_2Y):
        self.AX_1X = AX_1X
        self.AX_1Y = AX_1Y
        self.AX_2X = AX_2X
        self.AX_2Y = AX_2Y

    async def send_bytes(self):
        self.AX_1X = 129
        self.AX_1Y = 129
        self.AX_2X = 129
        self.AX_2Y = 254

        while True:
            # self.AX_1Y = self.AX_1Y + 1 if self.AX_1Y < 255 else 0

            send_msg = bytes([self.AX_1X, self.AX_1Y, self.AX_2X, self.AX_2Y])
            send_msg = self._msg_header + send_msg
            print(send_msg)  # You'll catch ASCII Conversion

            self._ser.write(send_msg)

            await asyncio.sleep(self._pub_period)

    def run(self):
        try:
            asyncio.ensure_future(self.send_bytes())
            self._loop.run_forever()
        except KeyboardInterrupt:
            pass
        finally:
            self._loop.close()
            print("====== Loop Closed =========")


if __name__ == "__main__":

    if args["port"] is None:
        print("Port Name required")
        print("Exit Process...")
        exit(1)

    my_sender = TeensySender(port_name=args["port"], baud_rate=115200, pub_period=0.01)
    try:
        my_sender.run()
    except Exception as e:
        print(e)
    finally:
        print("Done...")
