# import time
import asyncio
import configparser

from math import log
from RTCCam import DualCSICam, DualCam, WebCam, CSICam
from RTCPub import RTCDataChannel
from Controller import TeensySender

# from RCController import Vehicle
# from IRSensor import IRLapTimeCheck

# TODO url video option
class SpiderCar(RTCDataChannel):

    config = configparser.ConfigParser()
    config.read("config.ini")

    def __init__(self, channel_id, cam_id=0):
        super().__init__(channel=channel_id)

        # Cam Config
        # self._cam = WebCam(width=640, height=480, camID=0)
        self._cam = CSICam(width=640, height=480, camID=0, flip_method=0)
        self._peer.video.putSubscription(self._cam)

        # Controller Config
        self._myCar = TeensySender(
            port_name=str(self.config["Port"]["TEENSY_PORT"]),
            baud_rate=int(self.config["Communication"]["BAUD_RATE"]),
            pub_period=float(self.config["Communication"]["PUB_PERIOD"]),
        )

        self._motion = {}
        self._is_back = False
        self._prev_throttle = 0
        self._prev_angle = 0

    async def receiver(self):
        while True:
            self._motion = await self._dc_subscriber.get()
            # print(self._motion)

            key = self._motion["motion"]["key"]
            val = self._motion["motion"]["value"]

            if key == "forward":
                self._myCar.joyDict['ABS_Y'] = int(127 + (val / 20) * 128)
            elif key == "back":
                self._myCar.joyDict['ABS_Y'] = int(127 - (val / 20) * 127)

            if key == "left":
                self._myCar.joyDict['ABS_RX'] = int(127 - (val / 20) * 127)
            elif key == "right":
                self._myCar.joyDict['ABS_RX'] = int(127 + (val / 20) * 128)

    async def printGreeting(self, greeting):
        # print(self._motion)
        await asyncio.sleep(1.0)  # 1초 대기. asyncio.sleep도 네이티브 코루틴
        # await self._loop.run_in_executor(None, self._hub.motor_B.angled, 1)

        print(greeting)

    def run(self):
        try:
            asyncio.ensure_future(self._myCar.controlLoopExecutor())
            asyncio.ensure_future(self.connect())
            asyncio.ensure_future(self.receiver())
            self._loop.run_forever()
        except Exception as e:
            print(e)
        finally:
            self._cam.close()
            print("Done...")

    def __del__(self):
        print("Deletion")
        self._cam.close()
        return super().__del__()


if __name__ == "__main__":
    # [channel] ID: c40hipepjh65aeq6ndj0, Name: SooYoung,Kim, State:    idle, false, 0/0, cTime: 2021/08/03 15:44:52, uTime: 2021/08/03 15:44:52 |
    myRTCBot = SpiderCar(channel_id="c40hipepjh65aeq6ndj0")
    myRTCBot.run()
