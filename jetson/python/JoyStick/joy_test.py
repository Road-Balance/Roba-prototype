import inputs
import serial
import asyncio


port_name="/dev/ttyACM0"
baud_rate=115200
pub_period=0.5

class JoySerialSenderTwoBytes(object):

    joyDict = {
        'ABS_X': 32767,
        'ABS_Y': 32767,
        'ABS_RX': 32767,
        'ABS_RY': 32767,
    }

    def __init__(self):
        super().__init__()

        print(inputs.devices.gamepads)

        pads = inputs.devices.gamepads
        if len(pads) == 0:
            raise Exception("Couldn't find any Gamepads!")
    
        self._ser = serial.Serial(port_name, baud_rate, timeout=10)
        if self._ser.name != port_name:
            raise Exception("Couldn't find MCU!")

        self.msg_header = bytes([255, 1])
        self._loop = asyncio.get_event_loop()

    def parseJoyDict(self):

        output = bytes()
        bytes_list = []

        for val in self.joyDict.values():
            int_val = min( int(val / 257), 254 )
            bytes_list.append(int_val)

        print(bytes_list)
        output = bytes(bytes_list)

        return output

    async def joyLoop(self):
        events = inputs.get_gamepad()
        for event in events:
            if event.code in self.joyDict:

                # 뭔짓을 하다 돌아와도 0으로 되게
                if event.state < 1024 and event.state > -1024:
                    joy_val = 0
                else:
                    joy_val = event.state

                # TODO : 좌끝 => 65535 우끝 => 0 이다.
                self.joyDict[event.code] = -joy_val + 32767

                payload = self.parseJoyDict()
                send_msg = self.msg_header + payload
                self._ser.write(send_msg)

                print("======================")
                print(send_msg)

    async def joyLoopExecutor(self):
        while True:
            await self._loop.run_in_executor(None, self.joyLoop)

    def run(self):
        try:
            asyncio.ensure_future(self.joyLoop())
            self._loop.run_forever()
        except KeyboardInterrupt:
            pass
        finally:
            self._loop.close()
            print("====== Loop Closed =========")


if __name__=="__main__":

    my_serial = JoySerialSenderTwoBytes()
    my_serial.run()
