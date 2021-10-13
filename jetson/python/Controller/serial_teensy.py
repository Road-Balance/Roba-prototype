import serial
import asyncio
import argparse

from time import sleep

ap = argparse.ArgumentParser()
ap.add_argument("-p", "--port", type=str, help="path to optional input video file")
args = vars(ap.parse_args())
print(args)


class TeensySender(object):

    # moves like joystick
    joyDict = {
        'ABS_X': 127,
        'ABS_Y': 127,
        'ABS_RX': 127,
        'ABS_RY': 127,
    }

    prevjoyDict = {
        'ABS_X': 127,
        'ABS_Y': 127,
        'ABS_RX': 127,
        'ABS_RY': 127,
    }

    def __init__(self, port_name="/dev/ttyACM0", baud_rate=115200, pub_period=0.1):
        super().__init__()

        self._baud_rate = baud_rate
        self._pub_period = pub_period
        self._msg_header = bytes([255, 1])

        self._ser = serial.Serial(port_name, self._baud_rate, timeout=10)
        self._msg_header = bytes([255, 1])
        self._loop = asyncio.get_event_loop()

    def __del__(self):
        pass

    def set_axis_val(self, ABS_X, ABS_Y, ABS_RX, ABS_RY):
        joyDict['ABS_X'] = ABS_X
        joyDict['ABS_Y'] = ABS_Y
        joyDict['ABS_RX'] = ABS_RX
        joyDict['ABS_RY'] = ABS_RY

    def isChange(self):

        for k in self.joyDict.keys():
            if self.joyDict[k] != self.prevjoyDict[k]:
                return True

        return False

    def setDict(self):

        for k in self.joyDict.keys():
            self.prevjoyDict[k] = self.joyDict[k]

    def parseJoyDict(self):

        output = bytes()
        bytes_list = []

        for val in self.joyDict.values():
            # int_val = min( int(val / 257), 254 )
            # bytes_list.append(int_val)
            bytes_list.append(val)

        print(bytes_list)
        output = bytes(bytes_list)

        return output

    def control_loop(self):
        self.ABS_X = 129
        self.AX_1Y = 129
        self.AX_2X = 129
        self.AX_2Y = 129

        if self.isChange():
            payload = self.parseJoyDict()
            send_msg = self._msg_header + payload
            # print(send_msg)  # You'll catch ASCII Conversion

            self._ser.write(send_msg)
            self.setDict()
    
    async def controlLoopExecutor(self):
        while True:
            await self._loop.run_in_executor(None, self.control_loop)

    def run(self):
        try:
            asyncio.ensure_future(self.controlLoopExecutor())
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
