const {
    SerialPort
} = require('serialport')

// Define the port name and baud rate
const portName = '/dev/cu.usbmodem11101'; // Change this to the correct port for your Arduino
const baudRate = 115200; // Change this to match the baud rate used by your Arduino

//console.log(SerialPort)
// Create a new SerialPort instance
const port = new SerialPort({
    path: portName,
    baudRate: baudRate
});

// Open the serial port
port.on('open', () => {
    console.log(`Serial port ${portName} is open.`);

    // Example G-code command
    const gcode = 'G0 X10 Y10'; // Move to position X=10, Y=10

    // Send the G-code command
    port.write(`${gcode}\n`, (err) => {
        if (err) {
            console.error('Error writing to serial port:', err);
        } else {
            console.log(`Sent G-code command: ${gcode}`);
        }
    });
});

// Log errors
port.on('error', (err) => {
    console.error('Error:', err);
});