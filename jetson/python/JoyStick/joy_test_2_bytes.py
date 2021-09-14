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
        self.send_msg = bytes()

    def parse2Bytes(self, integer_val):
        output = bytes()

        bytes_val = hex(int(integer_val))

        low_byte = int(integer_val / (2**8))
        high_byte = integer_val % 128

        # 어차피 분해능은 32767이 아닌 0 - 255 이다. => 이건 2 바이트 버전이다.
        print(integer_val, integer_val / 257, bytes_val, hex(high_byte), hex(low_byte))

        return high_byte, low_byte

    def parseJoyDict(self):

        output = bytes()
        bytes_list = []

        for val in self.joyDict.values():
            high_byte, low_byte = self.parse2Bytes(val)

            bytes_list.append(high_byte)
            bytes_list.append(low_byte)

            # print(high_byte, low_byte)
            # output.append(low_byte)
            # output.append(high_byte)
        output = bytes(bytes_list)

        return output

    async def sendBytes(self):
        while True:
            self._ser.write(self.send_msg)
            print("======================")
            print(self.send_msg)
            await asyncio.sleep(0.01)

    async def joyLoop(self):
        while True:
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
                    self.send_msg = self.msg_header + payload

                    await asyncio.sleep(0.01)

    def run(self):
        try:
            asyncio.ensure_future(self.joyLoop())
            asyncio.ensure_future(self.sendBytes())
            self._loop.run_forever()
        except KeyboardInterrupt:
            pass
        finally:
            self._loop.close()
            print("====== Loop Closed =========")


if __name__=="__main__":

    my_serial = JoySerialSenderTwoBytes()
    my_serial.run()
