/*
 ____ _____ _   _ ____  ____   ___   __  __
/ ___|_   _| | | |  _ \|  _ \ / _ \  \ \/ /
\___ \ | | | | | | |_) | |_) | | | |  \  /
 ___) || | | |_| |  __/|  _ <| |_| |  /  \
|____/ |_|  \___/|_|   |_| \_\\___/  /_/\_\                  
                          
A mini-rapid-prototyped-drawmatic-postographo-plotter
by Gordan Savicic 2023

5-BAR LINKAGE CONFIGURATION
============================ 
This sketch calculates inverse kinematics for a 5-bar linkage robot.

Physical Parameters (measure your robot):
- armLength: Distance from motor pivot to end effector joint
- baseLength: Distance between the two motor pivot points

The reachable workspace depends on these values:
- Maximum reach: 2 × armLength - baseLength
- Minimum reach: baseLength
- Optimal workspace: Center region where angles are in valid ranges

Serial Port Configuration:
- Update serial.open() with your Arduino's port
- macOS: /dev/cu.usbserial-XXXX or /dev/cu.usbmodem-XXXX
- Windows: COM3, COM4, etc.
- Linux: /dev/ttyUSB0 or /dev/ttyACM0
*/

// ============================================================================
// CONFIGURATION
// ============================================================================

// UI Settings
let hideUI = false;
let enableServos = false; // Start with motors OFF (safer)

// 5-Bar Linkage Physical Parameters (measured in mm)
// MEASURED VALUES: Base = 25mm, Arm = 75mm
// CALCULATED WORKSPACE:
//   - Maximum reach: 125mm (2 × 75 - 25)
//   - Minimum reach: 25mm
//   - Optimal working area: ~40mm to 100mm from center
let armLength = 75; // Length of moving arm segments (motor shaft to joint)
let baseLength = 25; // Distance between motor shaft centers

// Visualization scale factor (multiply mm by this to get pixels)
let visualScale = 2.5; // 2.5px per mm for better visibility on screen

// Visual offset for arm positioning on canvas
let arm1BaseX = 200; // Left arm base X position
let arm1BaseY = 100; // Left arm base Y position

// Serial Port Configuration
const SERIAL_PORT = "/dev/cu.usbserial-140"; // ⚠️ UPDATE THIS!
const BAUD_RATE = 9600;

// Motor Direction Configuration
// Set to -1 to invert motor direction, 1 for normal
// For 5-bar linkage: motors should oppose each other (one +1, one -1)
let MOTOR_L_DIR = 1; // Left motor (Y command) - inverted
let MOTOR_R_DIR = 1; // Right motor (X command) - inverted

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================
let arm1, arm2;
let segArm1, segArm2, segBase1, segBase2;
let angle = 0;
let serial;
let input, button, cancel, servoButton;
let mode = 0; // Start in mouse follow mode (0 = mouse, 1 = square, 2 = circle, -1 = idle)
let x, y;
let sq, c, pts;
let homeX, homeY; // Home position for arms

function setup() {
  createCanvas(600, 600);
  //frameRate(25);

  serial = new p5.SerialPort();

  // Get a list the ports available and adjust the correct port
  //serial.list(listPorts);
  serial.on("connected", serverConnected);
  serial.open(SERIAL_PORT, { baudRate: BAUD_RATE });
  serial.on("data", serialEvent); // callback for when new data

  // Initialize the arms with configured parameters (scaled for visualization)
  // seg 1 is the moving arm, seg2 is the base arm
  let armLengthPx = armLength * visualScale;
  let baseLengthPx = baseLength * visualScale;

  // Angle limits with dynamic collision avoidance
  // Wider limits since smart avoidance will prevent actual collisions
  // Left motor: 0° to 180° (full range below baseline)
  // Right motor: 0° to 180° (full range below baseline)
  segArm1 = new Segment(0, 0, armLengthPx, radians(0), 0);
  segBase1 = segArm1.createParent(baseLengthPx, radians(70), 1, 5, 300); // Start open (right)

  segArm2 = new Segment(0, 0, armLengthPx, radians(50), 2);
  segBase2 = segArm2.createParent(baseLengthPx, radians(110), 3, -45, 250); // Start open (left)

  // Set up collision detection references between motor arms
  segBase1.otherSegment = segBase2;
  segBase2.otherSegment = segBase1;

  arm1 = new Arm(
    arm1BaseX,
    arm1BaseY,
    segArm1,
    segBase1,
    undefined,
    MOTOR_L_DIR,
    MOTOR_R_DIR,
  );
  arm2 = new Arm(
    arm1BaseX + baseLengthPx,
    arm1BaseY,
    segArm2,
    segBase2,
    segArm1,
  );

  // basic setup
  initUI();

  // Home position (centered in safe workspace)
  homeX = arm1BaseX + baseLengthPx / 2;
  homeY = arm1BaseY + armLengthPx * 0.88; // Within new conservative reach limit

  // Initialize arms at home position
  arm1.follow(homeX, homeY);
  arm2.follow(homeX, homeY);

  // Initialize drawing shapes (adjusted for smaller workspace)
  sq = new Square(homeX, homeY - 10, 30);
  ci = new myCircle(homeX, homeY - 40, 35, angle);

  // Print configuration to console
  console.log("=== STUPX 5-Bar Linkage Configuration ===");
  console.log(`Physical Measurements:`);
  console.log(`  Arm Length: ${armLength} mm (${armLengthPx} px)`);
  console.log(`  Base Length: ${baseLength} mm (${baseLengthPx} px)`);
  console.log(`Workspace:`);
  console.log(
    `  Max Reach: ${2 * armLength - baseLength} mm (${2 * armLengthPx - baseLengthPx} px)`,
  );
  console.log(`  Min Reach: ${baseLength} mm (${baseLengthPx} px)`);
  console.log(
    `  Optimal Range: ${Math.round(baseLength * 1.6)}-${Math.round((2 * armLength - baseLength) * 0.8)} mm`,
  );
  console.log(`Serial:`);
  console.log(`  Port: ${SERIAL_PORT}`);
  console.log(`  Baud Rate: ${BAUD_RATE}`);
  console.log(`Motor Direction:`);
  console.log(
    `  Left: ${MOTOR_L_DIR > 0 ? "Normal" : "Inverted"} (${MOTOR_L_DIR})`,
  );
  console.log(
    `  Right: ${MOTOR_R_DIR > 0 ? "Normal" : "Inverted"} (${MOTOR_R_DIR})`,
  );
  console.log("=========================================");
}

// Constrain target position to valid workspace (no points behind motors)
function constrainToWorkspace(targetX, targetY) {
  let armLengthPx = armLength * visualScale;
  let baseLengthPx = baseLength * visualScale;

  // Center point between the two motors
  let centerX = arm1BaseX + baseLengthPx / 2;
  let centerY = arm1BaseY;

  // Maximum and minimum reach from center (for 5-bar linkage)
  // REDUCED to avoid ambiguous/collision zones
  let maxReach = armLengthPx * 1.1; // More conservative limit
  let minReach = baseLengthPx * 1.2; // Keep away from motor area

  // Calculate distance from center
  let dx = targetX - centerX;
  let dy = targetY - centerY;
  let dist = sqrt(dx * dx + dy * dy);

  // Don't allow points behind the motors (above the base line)
  // Add buffer zone
  if (dy < armLengthPx * 0.3) {
    targetY = centerY + armLengthPx * 0.3; // Keep in safe forward zone
    dy = targetY - centerY;
    dist = sqrt(dx * dx + dy * dy);
  }

  // Constrain to maximum reach
  if (dist > maxReach) {
    let angle = atan2(dy, dx);
    targetX = centerX + cos(angle) * maxReach;
    targetY = centerY + sin(angle) * maxReach;
  }

  // Constrain to minimum reach
  if (dist < minReach && dist > 0) {
    let angle = atan2(dy, dx);
    targetX = centerX + cos(angle) * minReach;
    targetY = centerY + sin(angle) * minReach;
  }

  return [targetX, targetY];
}

// Draw workspace boundaries for visual reference
function drawWorkspace() {
  let armLengthPx = armLength * visualScale;
  let baseLengthPx = baseLength * visualScale;

  let centerX = arm1BaseX + baseLengthPx / 2;
  let centerY = arm1BaseY;

  // For 5-bar linkage: max reach when arms extend at angles
  let maxReach = armLengthPx * 1.3;
  let minReach = baseLengthPx * 0.8;

  // Draw maximum reach arc (only below baseline)
  noFill();
  stroke(150, 150, 255, 100);
  strokeWeight(1);
  arc(centerX, centerY, maxReach * 2, maxReach * 2, 0, PI);

  // Draw minimum reach arc
  stroke(255, 150, 150, 100);
  arc(centerX, centerY, minReach * 2, minReach * 2, 0, PI);

  // Draw baseline (motor axis)
  stroke(100, 100, 100, 150);
  strokeWeight(2);
  line(arm1BaseX, arm1BaseY, arm1BaseX + baseLengthPx, arm1BaseY);
}

// Smart collision avoidance - now handled by Segment class methods
function avoidCollision() {
  // Check and avoid collision using segment's built-in methods
  let collision1 = arm1.segBase.avoidCollision(30, 15);
  let collision2 = arm2.segBase.avoidCollision(30, 15);

  let collisionWarning = collision1 || collision2;

  // Visual warning indicators
  if (collisionWarning) {
    // Red circle at center
    fill(255, 0, 0, 100);
    noStroke();
    ellipse(arm1BaseX + (baseLength * visualScale) / 2, arm1BaseY, 50, 50);

    // Highlight the endpoint collision zones
    fill(255, 0, 0, 150);
    ellipse(arm1.segBase.to.x, arm1.segBase.to.y, 15, 15);
    ellipse(arm2.segBase.to.x, arm2.segBase.to.y, 15, 15);

    // Show distances for debugging
    let endDist = dist(
      arm1.segBase.to.x,
      arm1.segBase.to.y,
      arm2.segBase.to.x,
      arm2.segBase.to.y,
    );
    let segDist = arm1.segBase.segmentToSegmentDistance(arm2.segBase);

    fill(255, 0, 0);
    textSize(10);
    text(`Endpoint: ${Math.round(endDist)}px`, 20, 160);
    text(`Segment: ${Math.round(segDist)}px`, 20, 175);
  }
}

// Calculate minimum distance between two line segments
function segmentDistance(x1, y1, x2, y2, x3, y3, x4, y4) {
  // Line segment 1: from (x1,y1) to (x2,y2)
  // Line segment 2: from (x3,y3) to (x4,y4)

  // Calculate all point-to-segment distances
  let d1 = pointToSegmentDistance(x1, y1, x3, y3, x4, y4);
  let d2 = pointToSegmentDistance(x2, y2, x3, y3, x4, y4);
  let d3 = pointToSegmentDistance(x3, y3, x1, y1, x2, y2);
  let d4 = pointToSegmentDistance(x4, y4, x1, y1, x2, y2);

  return Math.min(d1, d2, d3, d4);
}

// Calculate distance from point (px, py) to line segment (x1,y1)-(x2,y2)
function pointToSegmentDistance(px, py, x1, y1, x2, y2) {
  let dx = x2 - x1;
  let dy = y2 - y1;
  let len2 = dx * dx + dy * dy;

  if (len2 === 0) return dist(px, py, x1, y1);

  // Calculate projection parameter
  let t = ((px - x1) * dx + (py - y1) * dy) / len2;
  t = Math.max(0, Math.min(1, t));

  // Find closest point on segment
  let closestX = x1 + t * dx;
  let closestY = y1 + t * dy;

  return dist(px, py, closestX, closestY);
}

function draw() {
  background(220);

  // Draw valid workspace boundaries
  drawWorkspace();

  switch (mode) {
    // idle - stay at home position
    case -1:
      arm1.follow(homeX, homeY);
      arm2.follow(homeX, homeY);
      break;

    // follow mouse
    case 0:
      // Constrain mouse to valid workspace
      let [constrainedX, constrainedY] = constrainToWorkspace(mouseX, mouseY);
      arm1.follow(constrainedX, constrainedY);
      arm2.follow(arm1.segArm.to.x, arm1.segArm.to.y);

      // Draw indicator at constrained position (kept for debugging)
      fill(0, 255, 0, 100);
      noStroke();
      ellipse(constrainedX, constrainedY, 10, 10);
      break;

    // follow square
    case 1:
      [x, y] = sq.draw();
      arm1.segBase.angle = radians(x);
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

  // Apply smart collision avoidance before updating
  avoidCollision();

  arm1.update();
  arm2.update();
  arm1.show();
  arm2.show();

  // Display current mode and status
  fill(0);
  noStroke();
  textSize(12);
  textAlign(LEFT);
  let modeText = ["Home (Idle)", "Mouse Follow", "Square Path", "Circle Path"];
  text(`Mode: ${modeText[mode + 1]}`, 20, 130);
  text(`Steppers: ${enableServos ? "ON" : "OFF"}`, 20, 145);

  // send angles to servos if enabled
  if (enableServos)
    arm1.moveServo(serial, arm1.segBase.angle, arm2.segBase.angle);
}

function listPorts(_args) {
  console.log(_args);
}

function serverConnected() {
  console.log("✅ Connected to serial server");
  // Send home position command (arms at rest)
  setTimeout(() => {
    serial.write("X0,Y0\n");
    console.log("🏠 Sent home position: X0,Y0");
  }, 500);
}

function serialEvent() {
  var inByte = serial.read();
  inData = inByte;
  // Only log meaningful messages, not every byte
  // print("received serial");
}

function sendSerial() {
  cmd = `${input.value()}\n`;
  serial.write(cmd);
}

// All button and input elements
function initUI() {
  stroke(0);
  strokeWeight(1);

  input = createInput();
  input.position(20, 50);

  button = createButton("send");
  button.position(145, 50);
  button.mousePressed(sendSerial);

  button = createButton("X");
  button.position(input.x + input.width + button.width, 50);
  button.mousePressed(remove);

  // Mode selection buttons
  button = createButton("Home");
  button.position(20, 80);
  button.mousePressed(() => {
    mode = -1;
    console.log("Mode: Home (idle)");
  });

  button = createButton("Mouse");
  button.position(70, 80);
  button.mousePressed(() => {
    mode = 0;
    console.log("Mode: Mouse follow");
  });

  button = createButton("Square");
  button.position(130, 80);
  button.mousePressed(() => {
    mode = 1;
    console.log("Mode: Square path");
  });

  button = createButton("Circle");
  button.position(195, 80);
  button.mousePressed(() => {
    mode = 2;
    console.log("Mode: Circle path");
  });

  // Stepper enable/disable toggle with status display
  servoButton = createButton(`Steppers: OFF`); // Start as OFF (safer)
  servoButton.position(260, 80);
  servoButton.style("background-color", "#ffcccc"); // Red for OFF
  servoButton.mouseReleased(() => {
    enableServos = !enableServos;
    if (enableServos) {
      servoButton.html("Steppers: ON");
      servoButton.style("background-color", "#ccffcc");
      console.log("✅ Steppers ENABLED - motors will move");
      serial.write("E\n");
    } else {
      servoButton.html("Steppers: OFF");
      servoButton.style("background-color", "#ffcccc");
      console.log("⛔ Steppers DISABLED - motors disengaged");
      mode = -1; // Stop drawing, go to idle/home mode
      serial.write("E\n");
    }
  });
}
