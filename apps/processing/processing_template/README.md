# Blimpy Template

A template to control a blimpy drone via Processing.

## Installation

### Install Processing *(PC)*
1. Download the offical release https://processing.org/download
2. Unpack the ZIP file
3. Rename the processing folder from `processing-4.0b7` to `processing`
4. Move the folder to `C:\Program Files\`

### Install Processing *(MAC)*
1. Download the offical release https://processing.org/download
2. Install the app by opening the downloaded file
3. Open Processing, open the `tools` menu and klick `install processing-java`

### Add MQTT Library *(PC & MAC)*

1. Open Processing 4 
2. Open `Sketch > Import Library > Add Library`
3. Search for the MQTT library by Joel Gaehwiler
4. Klick install

> Alternatively you could download the library manually from https://github.com/256dpi/processing-mqtt

### (Optional) Add Processing to VSCode *(PC)*
1. Open Windows Settings `Windows+i`
2. Go to "System" and klick "About" or "Info" (depending on Windows version)
3. Klick "Advanced system settings"
4. In the Modal go to "Advanced" > "Environment Variables"
5. Select "Path" and klick "Edit"
6. Add a new line by doubleklicking and write `C:\Program Files\processing`
7. Close all windows with OK
8. Open your project folder in VSCode
9. Go to the Extensions tab `Ctrl+Shift+X`
10. Install "Processing Language" extension
11. Open the command prompt with `Ctrl+Shift+P`
12. Run > `Processing: Create Task File`
13. Afterwards Run > `Processing: Run Processing Project`
14. Restart VSCode

You can now build your project by pressing `Ctrl+Shift+B`

> Tipp: For better intellisense you can also install the Processing VSCode Extension
### (Optional) Add Processing to VSCode *(MAC)*
1. Open your project folder on VSCode
2. Go to the Extensions tab `Ctrl+Shift+X`
3. Install "Processing Language" extension
4. Open the command prompt with `Ctrl+Shift+P`
5. Run > `Processing: Create Task File`
6. Afterwards Run > `Processing: Run Processing Project`
7. Restart VSCode

You can now build your project by pressing `Ctrl+Shift+B`

> Tipp: For better intellisense you can also install the Processing VSCode Extension

## Use the DroneController Class
To access your drone you just have to create an instance of this class and access a Input function to send a signal to it.

### Create Instance
`controller = new DroneController(processingRef, serviceUrl, deviceId, name);`
- processingRef = reference to the processing PApplet (usually *this*)
- serviceUrl = url of the broker service (incl. port)
- deviceId = name of your device (blimps/\<NAME>)
- name = name of your endpoint node (use whatever you want)

### SendInput
`controller.SendInput(topic, new float[]{.2,0,0,0,0,0});`
- topic = use "motors" / "servos" as topic
- array of 6 floats wich determine the speed of the motors / the angle of the servos

### SendMotorInput
`controller.SendMotorInput(.2,0,0,0,0,0);`
- accepts 6 floats as params wich determine the speed of the motors

`controller.SendMotorInput(0, .2);`
- index = from 0 to 5 wich motor can be accessed
- val = float from 0 to 1 wich determines the speed of the motor

### SendServoInput
`controller.SendServoInput(.2,0,0,0,0,0);`
- accepts 6 floats as params wich determine the angle of the servos

`controller.SendServoInput(0, .2);`
- index = from 0 to 5 wich servo can be accessed
- val = float from 0 to 1 wich determines the angle of the servo

### GetBattery
`controller.GetBattery()`
- returns the battery level as float from 0 to 1

### GetMotors
`controller.GetMotors()`
- returns an array of 6 floats containing the current speed of all motors

### GetServos
`controller.GetMotors()`
- returns an array of 6 floats containing the current angle of all servos

## Authors
Ivo Keller aka. Gonios
