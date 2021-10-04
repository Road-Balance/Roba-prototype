import inputs
import serial
import asyncio


port_name="/dev/ttyACM1"
baud_rate=115200
pub_period=0.5

class JoySerialSenderTwoBytes(object):

    joyDict = {
        'prev_HAT0X': 0,
        'prev_HAT0Y': 0,
        'ABS_HAT0X': 0,
        'ABS_HAT0Y': 0,
    }
    
    serial_msg = [128, 128, 128, 128]

    def __init__(self):
        super().__init__()

        print(inputs.devices.gamepads)

        pads = inputs.devices.gamepads
        if len(pads) == 0:
            raise Exception("Couldn't find any Gamepads!")
    
        # self._ser = serial.Serial(port_name, baud_rate, timeout=10)
        # if self._ser.name != port_name:
        #     raise Exception("Couldn't find MCU!")

        self.msg_header = bytes([255, 1])
        self._loop = asyncio.get_event_loop()

        self.THROTTLE_STEP = 25
        self.STEERING_STEP = 10

    def parseJoyDict(self):

        output = bytes()
        bytes_list = []

        for val in self.serial_msg:
            # int_val = min( int(val / 257), 254 )
            # bytes_list.append(int_val)
            bytes_list.append(val)

        print(bytes_list)
        output = bytes(bytes_list)

        return output

    def joyLoop(self):
        events = inputs.get_gamepad()
        for event in events:
            if event.code in self.joyDict:
                if event.code == 'ABS_HAT0X':
                    if event.state != 0:
                        self.joyDict['prev_HAT0X'] = event.state

                    if self.joyDict['prev_HAT0X'] == -1 and event.state == 0:
                        if self.serial_msg[2] <= self.STEERING_STEP:
                            self.serial_msg[2] = 0
                        else:
                            self.serial_msg[2] -= self.STEERING_STEP
                    elif self.joyDict['prev_HAT0X'] == +1 and event.state == 0:
                        if self.serial_msg[2] >= 255 - self.STEERING_STEP:
                            self.serial_msg[2] = 254
                        else:
                            self.serial_msg[2] += self.STEERING_STEP

                elif event.code == 'ABS_HAT0Y':
                    if event.state != 0:
                        self.joyDict['prev_HAT0Y'] = event.state

                    if self.joyDict['prev_HAT0Y'] == +1 and event.state == 0:
                        if self.serial_msg[1] <= self.THROTTLE_STEP:
                            self.serial_msg[1] = 0
                        else:
                            self.serial_msg[1] -= self.THROTTLE_STEP
                    elif self.joyDict['prev_HAT0Y'] == -1 and event.state == 0:
                        if self.serial_msg[1] >= 255 - self.THROTTLE_STEP:
                            self.serial_msg[1] = 254
                        else:
                            self.serial_msg[1] += self.THROTTLE_STEP

            elif event.code == 'BTN_NORTH':
                self.serial_msg = [128,128,128,128]

            payload = self.parseJoyDict()
            send_msg = self.msg_header + payload
            # self._ser.write(send_msg)

            print(self.serial_msg)
            print(send_msg)

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

    my_serial = JoySerialSenderTwoBytes()

    # while True:
    #     my_serial.joyLoop()
    my_serial.run()
