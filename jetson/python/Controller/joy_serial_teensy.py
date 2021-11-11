from asyncio.events import BaseDefaultEventLoopPolicy
import inputs
import serial
import asyncio

# Joystick MUST be "X" Mode!!

# port_name="/dev/ttyACM0"
# baud_rate=115200

class JoySerialSender(object):

    # 0 ~ 5 / 10 steps
    # 0, 0.5, 1.0, 1.5 ...
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

    btnDict = {
        'ABS_HAT0X': 0,
        'ABS_HAT0Y': 0,
    }

    btnDictPrev = {
        'ABS_HAT0X': 0,
        'ABS_HAT0Y': 0,
    }

    STEP = 10

    def __init__(self, port_name=None, baud_rate=None):
        super().__init__()

        print(inputs.devices.gamepads)

        pads = inputs.devices.gamepads
        if len(pads) == 0:
            raise Exception("Couldn't find any Gamepads!")
    
        if port_name == None: 
            port_name = "/dev/ttyACM0"
        if baud_rate == None:
            baud_rate = 115200

        self._ser = serial.Serial(port_name, baud_rate, timeout=10)
        if self._ser.name != port_name:
            raise Exception("Couldn't find MCU!")

        self.msg_header = bytes([255, 1])
        self._loop = asyncio.get_event_loop()

    def parseJoyDict(self):

        output = bytes()
        bytes_list = []

        for val in self.joyDict.values():
            # int_val = min( int(val / 257), 254 )
            # bytes_list.append(int_val)
            bytes_list.append(val)

        for val in self.btnDict.values():
            bytes_list.append(val)

        print(bytes_list)
        output = bytes(bytes_list)

        return output

    def send_signal(self):

        payload = self.parseJoyDict()
        send_msg = self.msg_header + payload
        self._ser.write(send_msg)
        print("======================")
        print(send_msg)

    def joyLoop(self):
        events = inputs.get_gamepad()
        for event in events:
            if (event.code in self.joyDict) or (event.code in self.btnDict):
                if event.code == "ABS_Y" or event.code == "ABS_RY":
                    event.state = - event.state + 255 - 1
                elif event.code in self.btnDict:
                    if event.state != 0:
                        if event.code == "ABS_HAT0Y":
                            self.btnDict[event.code] -= event.state * 5
                        elif event.code == "ABS_HAT0X":
                            self.btnDict[event.code] += event.state * 5 
                        if self.btnDict[event.code] < 0:
                            self.btnDict[event.code] = 0
                        if self.btnDict[event.code] > 200:
                            self.btnDict[event.code] = 200

                    self.send_signal()
                    print(self.btnDict)
                    continue

                event.state = min(int((event.state + 32768) / 256), 254) 

                if abs(self.prevjoyDict[event.code] - event.state) > self.STEP:
                    self.joyDict[event.code] = event.state
                    self.prevjoyDict[event.code] = event.state

                    self.send_signal()
                else:
                    continue
                
            elif event.code == 'BTN_NORTH':
                for key in self.joyDict.keys():
                    self.joyDict[key] = 128
                self.btnDict["ABS_HAT0X"] = 0
                self.btnDict["ABS_HAT0Y"] = 0
                self.send_signal()

    async def joyLoopExecutor(self):
        while True:
            await self._loop.run_in_executor(None, self.joyLoop)

    def run(self):
        try:
            asyncio.ensure_future(self.joyLoopExecutor())
            self._loop.run_forever()
        except KeyboardInterrupt:
            pass
        finally:
            self._loop.close()
            print("====== Loop Closed =========")


if __name__=="__main__":

    my_serial = JoySerialSender()

    # while True:
    #     my_serial.joyLoop()
    my_serial.run()
