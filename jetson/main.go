package main

import (
	"flag"
	"io"
	"time"

	"github.com/simulatedsimian/joystick"
	"github.com/sirupsen/logrus"

	"github.com/jacobsa/go-serial/serial"
)

type joysticValue struct {
	Axis1X, Axis2X, Axis1Y, Axis2Y int
}

type teensyValue struct {
	x1, x2, y1, y2 byte
}

var (
	Logger               *logrus.Logger
	currentJoystickState joysticValue
	currentTeensyValue   teensyValue
	teensySerialPort     io.ReadWriteCloser
)

func main() {
	Logger = logrus.New()

	var (
		jsid, baud int
		port       string
		err        error
		debug      bool
	)
	flag.IntVar(&jsid, "id", 0, "joystick device id")
	flag.StringVar(&port, "sport", "", "serial port")
	flag.IntVar(&baud, "sbaud", 115200, "serial port baudrate")
	flag.BoolVar(&debug, "debug", false, "print debug log")
	flag.Parse()

	if debug {
		Logger.SetLevel(logrus.DebugLevel)
	}

	serialOpt := serial.OpenOptions{
		PortName:        port,
		BaudRate:        uint(baud),
		DataBits:        8,
		StopBits:        1,
		MinimumReadSize: 4,
	}
	if teensySerialPort, err = serial.Open(serialOpt); err != nil {
		Logger.WithError(err).Fatalf("serial port open failure")
	}
	defer teensySerialPort.Close()

	js, err := joystick.Open(jsid)
	if err != nil {
		Logger.WithError(err).Fatalln("fail to init joystick")
	}
	defer js.Close()

	Logger.WithFields(logrus.Fields{
		"Name":         js.Name(),
		"Axis Count":   js.AxisCount(),
		"Button Count": js.ButtonCount(),
	}).Info("joystick device is opened")

	go readJoystick(js, 50)
	go func() {
		var buf []byte = make([]byte, 32)

		for {
			n, err := teensySerialPort.Read(buf)
			if err != nil {
				Logger.WithError(err).Errorln("fail to read port")
				continue
			}
			if n > 0 {
				Logger.Infof("recv: %s", string(buf[:n]))
			}
			time.Sleep(250 * time.Millisecond)
		}
	}()
	for {
		time.Sleep(50 * time.Millisecond)
		msg := teensyStateToMessage(currentTeensyValue)
		if _, err := teensySerialPort.Write(msg); err != nil {
			Logger.WithError(err).Fatalln("fail to send message")
		}
		// Logger.Debugf("send : %v", msg)
		Logger.Println("send : %v", msg)
	}
}

func readJoystick(js joystick.Joystick, interval int) {
	for {
		s, err := js.Read()
		if err != nil {
			panic(err)
		}
		currentJoystickState.Axis1X = s.AxisData[0]
		currentJoystickState.Axis1Y = s.AxisData[1]
		currentJoystickState.Axis2X = s.AxisData[4]
		currentJoystickState.Axis2Y = s.AxisData[3]

		bindJoystickToTeensy(&currentJoystickState, &currentTeensyValue)
		time.Sleep(time.Microsecond * time.Duration(interval))
	}
}

func bindJoystickToTeensy(joystick *joysticValue, teensy *teensyValue) {
	teensy.x1 = 255 - byte((joystick.Axis1X+32767)/256)
	teensy.y1 = 255 - byte((joystick.Axis1Y+32767)/256)
	teensy.x2 = 255 - byte((joystick.Axis2X+32767)/256)
	teensy.y2 = 255 - byte((joystick.Axis2Y+32767)/256)
	if teensy.x1 >= 255 {
		teensy.x1 = 254
	}
	if teensy.x2 >= 255 {
		teensy.x2 = 254
	}
	if teensy.y1 >= 255 {
		teensy.y1 = 254
	}
	if teensy.y2 >= 255 {
		teensy.y2 = 254
	}
}

func teensyStateToMessage(teensy teensyValue) []byte {
	var b []byte = make([]byte, 6)
	b[0] = 0xFF
	b[1] = 0x01
	b[2] = teensy.x1
	b[3] = teensy.y1
	b[4] = teensy.y2
	b[5] = teensy.x2
	return b
}
