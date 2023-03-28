/*
 ____ _____ _   _ ____  ____   ___   __  __
/ ___|_   _| | | |  _ \|  _ \ / _ \  \ \/ /
\___ \ | | | | | | |_) | |_) | | | |  \  /
 ___) || | | |_| |  __/|  _ <| |_| |  /  \
|____/ |_|  \___/|_|   |_| \_\\___/  /_/\_\                  
                          
A mini-rapid-prototyped-drawmatic-postographo-plotter
by Gordan Savicic 2023

*/



// CONFIG
let hideUI = false;
let enableServos = false;

// ------------------------
let arm1, arm2;
let segArm1, segArm2, segBase1, segBase2;
let armLength = 115;
let baseLength = 50;
let angle = 0;
let serial;
let input, button, cancel;
let mode = 0;
let x, y;
let sq, c, pts;

function setup() {
  createCanvas(400, 400);
  //frameRate(25);

  serial = new p5.SerialPort();

  // Get a list the ports available and adjust the correct port
  // serial.list(listPorts); 
  serial.on('connected', serverConnected);
  serial.open("/dev/tty.usbmodem14101"); //{baudrate: 115200}
  serial.on('data', serialEvent); // callback for when new data

  // Initialize the arms
  // seg 1 is the moving arm, seg2 is the base arm
  segArm1 = new Segment(0, 0, armLength, radians(0), 0);
  segBase1 = segArm1.createParent(baseLength, radians(50), 1);

  segArm2 = new Segment(0, 0, armLength, radians(50), 2);
  segBase2 = segArm2.createParent(baseLength, radians(50), 3);

  arm1 = new Arm(150, 0, segArm1, segBase1);
  arm2 = new Arm(250, 0, segArm2, segBase2, segArm1);

  // basic setup
  initUI();

  arm1.follow(200, 120);
  arm2.follow(200, 20);

  sq = new Square(175, 100, 30);
  ci = new myCircle(width * .5, 120, 30, angle)
}


function draw() {
  background(220);
  switch (mode) {
    // follow mouse
    case 0:
      arm1.follow(mouseX, mouseY);
      arm2.follow(arm1.segArm.to.x, arm1.segArm.to.y);
      break;

    // follow square
    case 1:
      [x, y] = sq.draw();
      arm1.segBase.angle = radians(x)
      arm1.follow(x, y);
      arm2.follow(arm1.segArm.to.x, arm1.segArm.to.y);
      break;

    // follow a circle path
    case 2:
      [x, y] = ci.draw();
      ci.update();
      arm1.follow(x, y);
      arm2.follow(arm1.segArm.to.x, arm1.segArm.to.y);
      break;
  }

  arm1.update();
  arm2.update();
  arm1.show();
  arm2.show();

  // send angles to servos if enabled
  if (enableServos) arm1.moveServo(serial, arm1.segBase.angle, arm2.segBase.angle);
}


function listPorts(_args) {
  console.log(_args);
}

function serverConnected() {
  print("... Connected to Server");
  serial.write("X51,Y50\n")
}

function serialEvent() {
  var inByte = serial.read();
  inData = inByte;
  // print("received serial")
}

function sendSerial() {
  cmd = `${input.value()}\n`;
  serial.write(cmd)
}

// All button and input elements
function initUI() {
  stroke(0)
  strokeWeight(1)

  input = createInput();
  input.position(20, height - 50);

  button = createButton('send');
  button.position(145, height - 50);
  button.mousePressed(sendSerial);

  button = createButton('X');
  button.position(input.x + input.width + button.width, height - 50);
  button.mousePressed(remove);

  button = createButton('0');
  button.mousePressed(() => mode = 0);

  button = createButton('1');
  button.mousePressed(() => mode = 1);

  button = createButton('2');
  button.mousePressed(() => mode = 2);

  servoButton = createButton(`Servo: on/off`)
    .mouseReleased(() => enableServos = !enableServos);
}