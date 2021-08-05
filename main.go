package main

import (
	"flag"
	"fmt"
	"strconv"
	"time"

	"github.com/simulatedsimian/joystick"
	"github.com/sirupsen/logrus"

	ui "github.com/gizak/termui/v3"
	"github.com/gizak/termui/v3/widgets"
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
)

func main() {
	Logger = logrus.New()

	var jsid int
	flag.IntVar(&jsid, "id", 0, "joystick device id")
	flag.Parse()

	js, err := joystick.Open(jsid)
	if err != nil {
		Logger.WithError(err).Fatalln("fail to init jostick")
	}
	defer js.Close()

	Logger.WithFields(logrus.Fields{
		"Name":         js.Name(),
		"Axis Count":   js.AxisCount(),
		"Button Count": js.ButtonCount(),
	}).Info("joystick device is opened")

	if err := ui.Init(); err != nil {
		Logger.WithError(err).Fatalln("failed to init termui")
	}
	defer ui.Close()

	tableValue := widgets.NewTable()
	tableValue.Rows = [][]string{
		{"Axis1 X", "0"},
		{"Axis1 Y", "0"},
		{"Axis2 X", "0"},
		{"Axis2 Y", "0"},
		{"Teensy X1", "0"},
		{"Teensy Y1", "0"},
		{"Teensy X2", "0"},
		{"Teensy Y2", "0"},
		{"Teensy msg", ""},
	}
	tableValue.TextStyle = ui.NewStyle(ui.ColorWhite)
	tableValue.SetRect(0, 0, 40, 19)
	tableValue.ColumnWidths = []int{10, 30}
	ui.Render(tableValue)
	uiEvents := ui.PollEvents()
	uiRefresh := time.NewTicker(time.Microsecond * 200).C

	go readJoystick(js, 100)
	for {
		select {
		case e := <-uiEvents:
			switch e.ID {
			case "q", "<C-c>":
				return
			}
		case <-uiRefresh:
			tableValue.Rows[0][1] = strconv.Itoa(currentJoystickState.Axis1X)
			tableValue.Rows[1][1] = strconv.Itoa(currentJoystickState.Axis1Y)
			tableValue.Rows[2][1] = strconv.Itoa(currentJoystickState.Axis2X)
			tableValue.Rows[3][1] = strconv.Itoa(currentJoystickState.Axis2Y)
			tableValue.Rows[4][1] = strconv.Itoa(int(currentTeensyValue.x1))
			tableValue.Rows[5][1] = strconv.Itoa(int(currentTeensyValue.y1))
			tableValue.Rows[6][1] = strconv.Itoa(int(currentTeensyValue.x2))
			tableValue.Rows[7][1] = strconv.Itoa(int(currentTeensyValue.y2))
			tableValue.Rows[8][1] = fmt.Sprintf("%v ", teensyStateToMessage(currentTeensyValue))
			ui.Render(tableValue)
		}
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
	teensy.x1 = byte((joystick.Axis1X + 32767) / 256)
	teensy.y1 = byte((joystick.Axis1Y + 32767) / 256)
	teensy.x2 = byte((joystick.Axis2X + 32767) / 256)
	teensy.y2 = byte((joystick.Axis2Y + 32767) / 256)
}

func teensyStateToMessage(teensy teensyValue) [5]byte {
	var b [5]byte
	b[0] = 0xFF
	b[1] = teensy.x1
	b[2] = teensy.y1
	b[3] = teensy.x2
	b[4] = teensy.y2
	return b
}
