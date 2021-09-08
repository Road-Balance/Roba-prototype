package main

import (
	"bufio"
	"flag"
	"io"
	"math"
	"time"

	"github.com/simulatedsimian/joystick"
	"github.com/sirupsen/logrus"

	"github.com/jacobsa/go-serial/serial"
)

type joysticValue struct {
	Axis1X, Axis2X, Axis1Y, Axis2Y int
}

type teensyValue struct {
	x1, x2, y1, y2 uint16
}

var (
	Logger               *logrus.Logger
	currentJoystickState joysticValue
	currentTeensyValue   teensyValue
	teensySerialPort     io.ReadWriteCloser
)

var JetsonJoystick []byte = []byte{0, 1, 4, 3}

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
		reader := bufio.NewReader(teensySerialPort)
		for {
			//time.Sleep(500 * time.Millisecond)
			line, err := reader.ReadString('\n')
			if err != nil {
				Logger.WithError(err).Errorln("fail to read port")
				continue
			}
			if len(line) > 0 {
				Logger.Infof("recv %d: %s", len(line), line)
			}
		}
	}()
	for {
		time.Sleep(330 * time.Millisecond)
		msg := teensyStateToMessage(currentTeensyValue)
		if _, err := teensySerialPort.Write(msg); err != nil {
			Logger.WithError(err).Fatalln("fail to send message")
		}
		Logger.Debugf("send : %v", msg)
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

func intToUint16(i int) uint16 {
	var v int = (i + math.MaxInt16)
	if v >= math.MaxUint16 {
		return math.MaxUint16 - 1
	}
	if v <= 0 {
		return 0
	}
	return uint16(v)
}

func bindJoystickToTeensy(joystick *joysticValue, teensy *teensyValue) {
	teensy.x1 = intToUint16(joystick.Axis1X)
	teensy.y1 = intToUint16(joystick.Axis1Y)
	teensy.x2 = intToUint16(joystick.Axis2X)
	teensy.y2 = intToUint16(joystick.Axis2Y)
}

func supressByte(i uint16) byte {
	b := byte(i)
	if b >= 255 {
		return 254
	}
	return b
}

func teensyStateToMessage(teensy teensyValue) []byte {
	var b []byte = make([]byte, 10)
	b[0] = 0xFF
	b[1] = 0x01
	b[2] = supressByte(teensy.x1 >> 8)
	b[3] = supressByte(teensy.x1 & 255)
	b[4] = supressByte(teensy.y1 >> 8)
	b[5] = supressByte(teensy.y1 & 255)
	b[6] = supressByte(teensy.x2 >> 8)
	b[7] = supressByte(teensy.x2 & 255)
	b[8] = supressByte(teensy.y2 >> 8)
	b[9] = supressByte(teensy.y2 & 255)

	return b
}
