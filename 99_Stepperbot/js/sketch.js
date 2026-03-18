/*
 ____ _____ _   _ ____  ____   ___   __  __
/ ___|_   _| | | |  _ \|  _ \ / _ \  \ \/ /
\___ \ | | | | | | |_) | |_) | | | |  \  /
 ___) || | | |_| |  __/|  _ <| |_| |  /  \
|____/ |_|  \___/|_|   |_| \_\\___/  /_/\_\                  
                          
4-BAR PARALLELOGRAM LINKAGE
============================

Configuration:
- Two motors separated by 43mm (motor axis)
- Motor arms: 40mm total, using hole at 25mm from axis
- Connecting arm 1: 75mm
- Connecting arm 2: 90mm (75mm + 15mm extension with pen)
- Asymmetric 4-bar linkage

Segments:
  Motor1 --[25mm]--> Joint1 --[75mm]--> Pen
  Motor2 --[25mm]--> Joint2 --[90mm]--> Pen
  |<---- 43mm ---->|
*/

// ============================================================================
// PHYSICAL CONFIGURATION
// ============================================================================
const MOTOR_SEPARATION = 43; // mm - distance between motor axis
const MOTOR_ARM_LENGTH = 25; // mm - effective length (hole position)
const LINK_ARM_1_LENGTH = 75; // mm - first connecting arm
const LINK_ARM_2_LENGTH = 75; // mm - second connecting arm to the joint
const PEN_OFFSET = 15; // mm - pen tip offset mounted on link arm 2 beyond the joint

// Motor angle limits for IK, in degrees
const MOTOR_1_MIN_ANGLE_DEG = 0;
const MOTOR_1_MAX_ANGLE_DEG = 205;
const MOTOR_2_MIN_ANGLE_DEG = -15;
const MOTOR_2_MAX_ANGLE_DEG = 180;

// Workspace reach margins
// 1.0 = use full geometric reach, >1.0 or <1.0 shrink the shown workspace
const INNER_REACH_MARGIN = 0.7;
const OUTER_REACH_MARGIN = 1;
const WORKSPACE_MIN_NEIGHBORS = 8;
const WORKSPACE_MIN_ANGLE_MARGIN_DEG = 0;

// Demo path sizes (will be reduced automatically if they exceed the reachable area)
const DEFAULT_SQUARE_SIZE = 45;
const DEFAULT_CIRCLE_RADIUS = 20;
const MAX_RECT_TILT_ANGLE_DEG = 25;
const STUPX_TILT_ANGLE_DEG = 20;
const STUPX_TEXT = "STPX\n26";
const STUPX_TEXT_SCALE = 1.5;
const STUPX_TEXT_OFFSET_X = 40;
const STUPX_TEXT_OFFSET_Y = 5;
const DRAWING_HOME_TARGET_X = 260;
const DRAWING_HOME_TARGET_Y = 355;

// Visualization
const SCALE = 3.5; // pixels per mm (larger scale for smaller robot)
const MOTOR1_X = 200; // Canvas position of left motor
const MOTOR1_Y = 100; // Canvas position baseline

// Serial Configuration
const SERIAL_PORT = "/dev/cu.usbserial-1120";
const BAUD_RATE = 9600;
const STARTUP_HOME_CMD_X = -90;
const STARTUP_HOME_CMD_Y = 0;

// Motor direction multipliers
let MOTOR_L_DIR = -1; // Left motor direction (Motor 1)
let MOTOR_R_DIR = -1; // Right motor direction (Motor 2) - INVERSED

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================
let serial;
let enableSteppers = false; // Start with motors off so manual homing is possible
let mode = -1; // 0 = mouse, 1 = square, 2 = circle, -1 = home

// 4-bar linkage segments
let motorArm1, motorArm2; // Motor-controlled arms (red)
let linkArm1, linkArm2; // Passive connecting arms (blue)
let jointPos; // Meeting point of the passive arms
let penPos; // Actual pen tip position

// Last sent angles for change detection
let lastSentAngle1 = null;
let lastSentAngle2 = null;
let homePos; // Home position

// Command queueing to prevent Arduino overload
let arduinoBusy = false; // Is Arduino currently executing a command?
let commandQueue = []; // Queue of pending commands
let lastCommandTime = 0; // Timestamp of last command sent
const MIN_COMMAND_INTERVAL = 100; // Minimum ms between commands
const MAX_COMMAND_QUEUE = 5;
const ARDUINO_BUSY_TIMEOUT_MS = 150;
const IK_ANGLE_EPSILON = (2 * Math.PI) / 180; // Hysteresis near angle limits to avoid branch flips

// Motor positions in canvas coordinates
let motor1Pos, motor2Pos;

// Workspace boundaries
let workspacePolygon = []; // Calculated valid workspace boundary
let workspacePoints = []; // Sampled points where the pen can actually reach
let lastAngles = {
  theta1: (90 * Math.PI) / 180,
  theta2: (90 * Math.PI) / 180,
}; // Temporary - will be set by IK for pen at (274, 405)

// UI elements
let input, servoButton;
let ikDebugLogArea = null;

// Drawing path shapes
let sq, ci; // Square and circle path generators
let largestRectangle = null;
let rectPath = null;
let stupxTextBox = null;
let stupxTextPath = null;
let stupxTextSegments = [];
let fillPath = []; // Ordered list of workspace dots for fill mode
let fillIndex = 0;
let debugStartupPenPoint = null;
let debugNinetyPenPoint = null;
let debugStartupReachable = false;
let debugNinetyReachable = false;
let debugStartupReason = "";
let debugNinetyReason = "";
let manualPreviewTarget = null;

function getMotorAnglesFromCommandHome(cmdX, cmdY) {
  return {
    theta1: radians(90 + cmdY / MOTOR_L_DIR),
    theta2: radians(cmdX / MOTOR_R_DIR),
  };
}

function estimatePenPointForMotorAngles(theta1, theta2) {
  let joint1X = motor1Pos.x + cos(theta1) * MOTOR_ARM_LENGTH * SCALE;
  let joint1Y = motor1Pos.y + sin(theta1) * MOTOR_ARM_LENGTH * SCALE;
  let joint2X = motor2Pos.x + cos(theta2) * MOTOR_ARM_LENGTH * SCALE;
  let joint2Y = motor2Pos.y + sin(theta2) * MOTOR_ARM_LENGTH * SCALE;

  let joint = circleIntersection(
    joint1X,
    joint1Y,
    LINK_ARM_1_LENGTH * SCALE,
    joint2X,
    joint2Y,
    LINK_ARM_2_LENGTH * SCALE,
  );

  if (!joint) {
    return null;
  }

  let dx = joint.x - joint2X;
  let dy = joint.y - joint2Y;
  let len = Math.hypot(dx, dy);
  if (len < 1e-6) {
    return createVector(joint.x, joint.y);
  }

  let penOffsetPx = PEN_OFFSET * SCALE;
  dx /= len;
  dy /= len;
  return createVector(joint.x + dx * penOffsetPx, joint.y + dy * penOffsetPx);
}

// ============================================================================
// SETUP
// ============================================================================
function setup() {
  createCanvas(600, 600);

  // Calculate motor positions
  motor1Pos = createVector(MOTOR1_X, MOTOR1_Y);
  motor2Pos = createVector(MOTOR1_X + MOTOR_SEPARATION * SCALE, MOTOR1_Y);

  let startupAngles = getMotorAnglesFromCommandHome(
    STARTUP_HOME_CMD_X,
    STARTUP_HOME_CMD_Y,
  );

  // Initialize segments from the manual startup pose.
  motorArm1 = new Segment(
    motor1Pos.x,
    motor1Pos.y,
    MOTOR_ARM_LENGTH * SCALE,
    startupAngles.theta1,
    [255, 0, 0],
  );
  motorArm2 = new Segment(
    motor2Pos.x,
    motor2Pos.y,
    MOTOR_ARM_LENGTH * SCALE,
    startupAngles.theta2,
    [255, 0, 0],
  );
  linkArm1 = new Segment(0, 0, LINK_ARM_1_LENGTH * SCALE, 0, [0, 0, 255]);
  linkArm2 = new Segment(0, 0, LINK_ARM_2_LENGTH * SCALE, 0, [0, 0, 255]);

  if (!syncLinkageToMotorAngles()) {
    console.warn("⚠ Startup home pose invalid, falling back to 90°/90°");
    motorArm1.setAngle(radians(90));
    motorArm2.setAngle(radians(90));
    syncLinkageToMotorAngles();
  }

  // Remember home angles
  lastAngles.theta1 = motorArm1.angle;
  lastAngles.theta2 = motorArm2.angle;

  // Calculate workspace after home/lastAngles are established
  calculateWorkspace();
  buildFillPath();

  let preferredHome = findNearestReachablePoint(
    DRAWING_HOME_TARGET_X,
    DRAWING_HOME_TARGET_Y,
  );
  homePos = preferredHome
    ? createVector(preferredHome.x, preferredHome.y)
    : penPos.copy();

  // Initialize drawing path shapes
  let safeCircleRadius = findSafeCircleRadiusFromTop(
    homePos.x,
    homePos.y,
    DEFAULT_CIRCLE_RADIUS,
  );
  let safeSquareSize = findSafeTiltedSquareSizeFromTop(
    homePos.x,
    homePos.y,
    DEFAULT_SQUARE_SIZE,
  );

  sq = new Square(homePos.x, homePos.y, safeSquareSize, false, true);
  ci = new myCircle(
    homePos.x,
    homePos.y - safeCircleRadius,
    safeCircleRadius,
    20,
  );

  debugStartupPenPoint = penPos.copy();
  let startupDebug = canReachWithIKDebug(
    debugStartupPenPoint.x,
    debugStartupPenPoint.y,
  );
  debugStartupReachable = startupDebug.reachable;
  debugStartupReason = startupDebug.reason;

  debugNinetyPenPoint = estimatePenPointForMotorAngles(
    radians(90),
    radians(90),
  );
  if (debugNinetyPenPoint) {
    let ninetyDebug = canReachWithIKDebug(
      debugNinetyPenPoint.x,
      debugNinetyPenPoint.y,
    );
    debugNinetyReachable = ninetyDebug.reachable;
    debugNinetyReason = ninetyDebug.reason;
  } else {
    debugNinetyReachable = false;
    debugNinetyReason = "no forward-kinematics intersection";
  }

  console.log(
    `[IK DEBUG] startup pen reachable=${debugStartupReachable} reason=${debugStartupReason}`,
  );
  console.log(
    `[IK DEBUG] 90/90 pen reachable=${debugNinetyReachable} reason=${debugNinetyReason}`,
  );

  // Setup serial
  serial = new p5.SerialPort();
  serial.on("connected", serverConnected);
  serial.on("data", serialEvent); // Listen for Arduino responses
  serial.open(SERIAL_PORT, { baudRate: BAUD_RATE });

  // Initialize UI
  initUI();
  logIKDebugToUI();

  console.log("=== 4-BAR ASYMMETRIC LINKAGE INITIALIZED ===");
  console.log(`Motor separation: ${MOTOR_SEPARATION}mm`);
  console.log(`Motor arm length: ${MOTOR_ARM_LENGTH}mm`);
  console.log(`Link arm 1 length: ${LINK_ARM_1_LENGTH}mm`);
  console.log(`Link arm 2 length: ${LINK_ARM_2_LENGTH}mm (with pen)`);
  console.log(
    `Startup pen position: (${Math.round(penPos.x)}, ${Math.round(penPos.y)})`,
  );
  console.log(
    `Pen home position: (${Math.round(homePos.x)}, ${Math.round(homePos.y)})`,
  );
  console.log(
    `Motor angles at home: M1=${Math.round(degrees(motorArm1.angle))}° M2=${Math.round(degrees(motorArm2.angle))}°`,
  );

  // Calculate what Arduino command would be sent
  let homeCmd1 = (degrees(motorArm1.angle) - 90) * MOTOR_L_DIR;
  let homeCmd2 = degrees(motorArm2.angle) * MOTOR_R_DIR;
  console.log(
    `Arduino command for home: X${Math.round(homeCmd2)},Y${Math.round(homeCmd1)}`,
  );
  console.log(
    `Manual startup home expected before enabling: X${STARTUP_HOME_CMD_X},Y${STARTUP_HOME_CMD_Y}`,
  );
  console.log(`Workspace area: ${workspacePolygon.length} boundary points`);
  console.log(`Safe square size: ${Math.round(safeSquareSize)}px`);
  console.log(`Safe circle radius: ${Math.round(safeCircleRadius)}px`);
  if (largestRectangle) {
    console.log(
      `Largest tilted rectangle: ${Math.round(largestRectangle.width)} x ${Math.round(largestRectangle.height)} px @ ${largestRectangle.angleDeg}°`,
    );
  }
}

// ============================================================================
// WORKSPACE VISUALIZATION
// ============================================================================
function drawWorkspace() {
  if (workspacePoints.length === 0) return;

  noStroke();
  fill(100, 255, 100, 40);
  for (let p of workspacePoints) {
    circle(p.x, p.y, 5);
  }

  if (mode === 1) {
    drawSquarePreview();
  }

  if (mode === 2) {
    drawCirclePreview();
  }

  if (mode === 4) {
    drawLargestRectanglePreview();
  }

  if (mode === 5) {
    drawStupxPreview();
  }

  drawReachabilityDebugOverlay();
}

function drawReachabilityDebugOverlay() {
  textSize(11);
  textAlign(LEFT);

  if (debugStartupPenPoint) {
    stroke(30, 160, 255);
    strokeWeight(2);
    noFill();
    circle(debugStartupPenPoint.x, debugStartupPenPoint.y, 12);
    noStroke();
    fill(30, 160, 255);
    text(
      `startup ${debugStartupReachable ? "OK" : "X"}`,
      debugStartupPenPoint.x + 8,
      debugStartupPenPoint.y - 8,
    );
    text(
      debugStartupReason,
      debugStartupPenPoint.x + 8,
      debugStartupPenPoint.y + 6,
    );
  }
}

function formatIKDebugEntry(label, point, reachable, reason) {
  if (!point) {
    return `${label}: n/a`;
  }

  return `${label}: (${Math.round(point.x)}, ${Math.round(point.y)}) | ${
    reachable ? "OK" : "X"
  } | ${reason}`;
}

function logIKDebugToUI() {
  if (!motorArm1 || !motorArm2 || !penPos) {
    return;
  }

  debugStartupPenPoint = penPos.copy();
  let startupDebug = canReachWithIKDebug(
    debugStartupPenPoint.x,
    debugStartupPenPoint.y,
  );
  debugStartupReachable = startupDebug.reachable;
  debugStartupReason = startupDebug.reason;

  debugNinetyPenPoint = estimatePenPointForMotorAngles(
    radians(90),
    radians(90),
  );
  if (debugNinetyPenPoint) {
    let ninetyDebug = canReachWithIKDebug(
      debugNinetyPenPoint.x,
      debugNinetyPenPoint.y,
    );
    debugNinetyReachable = ninetyDebug.reachable;
    debugNinetyReason = ninetyDebug.reason;
  } else {
    debugNinetyReachable = false;
    debugNinetyReason = "no forward-kinematics intersection";
  }

  let currentPenDebug = canReachWithIKDebug(penPos.x, penPos.y);
  let lines = [
    `IK Debug @ ${new Date().toLocaleTimeString()}`,
    formatIKDebugEntry(
      "startup",
      debugStartupPenPoint,
      debugStartupReachable,
      debugStartupReason,
    ),
    formatIKDebugEntry(
      "90/90",
      debugNinetyPenPoint,
      debugNinetyReachable,
      debugNinetyReason,
    ),
    formatIKDebugEntry(
      "current pen",
      penPos,
      currentPenDebug.reachable,
      currentPenDebug.reason,
    ),
  ];

  if (ikDebugLogArea) {
    ikDebugLogArea.value(lines.join("\n"));
  }

  for (let line of lines) {
    console.log(`[IK DEBUG UI] ${line}`);
  }
}

function applyManualXYPreview(cmdX, cmdY) {
  if (!motorArm1 || !motorArm2) {
    return false;
  }

  let theta1 = radians(90 + cmdY / MOTOR_L_DIR);
  let theta2 = radians(cmdX / MOTOR_R_DIR);

  motorArm1.setAngle(theta1);
  motorArm2.setAngle(theta2);

  if (!syncLinkageToMotorAngles()) {
    return false;
  }

  lastAngles.theta1 = motorArm1.angle;
  lastAngles.theta2 = motorArm2.angle;
  manualPreviewTarget = penPos.copy();
  mode = 6;

  logIKDebugToUI();
  return true;
}

function drawSquarePreview() {
  if (!sq || !sq.getPathPoints) {
    return;
  }

  let points = sq.getPathPoints();
  if (!points || points.length < 2) {
    return;
  }

  noFill();
  stroke(255, 140, 0, 220);
  strokeWeight(2);
  beginShape();
  for (let point of points) {
    vertex(point[0], point[1]);
  }
  endShape();
}

function drawCirclePreview() {
  if (!ci || !ci.getOutlinePoints) {
    return;
  }

  let points = ci.getOutlinePoints(8);
  if (!points || points.length < 2) {
    return;
  }

  noFill();
  stroke(255, 140, 0, 220);
  strokeWeight(2);
  beginShape();
  for (let point of points) {
    vertex(point[0], point[1]);
  }
  endShape(CLOSE);
}

function drawLargestRectanglePreview() {
  if (!largestRectangle || !largestRectangle.corners) {
    return;
  }

  fill(255, 140, 0, 28);
  stroke(255, 140, 0, 180);
  strokeWeight(2);
  beginShape();
  for (let corner of largestRectangle.corners) {
    vertex(corner.x, corner.y);
  }
  endShape(CLOSE);

  noStroke();
  fill(255, 140, 0, 200);
  for (let corner of largestRectangle.corners) {
    circle(corner.x, corner.y, 6);
  }
}

function drawStupxPreview() {
  if (stupxTextSegments.length === 0) {
    return;
  }

  noFill();
  stroke(255, 90, 0, 220);
  strokeWeight(2);
  for (let segment of stupxTextSegments) {
    beginShape();
    for (let point of segment) {
      vertex(point.x, point.y);
    }
    endShape();
  }
}

function startLargestRectangleMode() {
  if (!largestRectangle) {
    largestRectangle = findLargestTiltedRectangleInWorkspace({
      angleDeg: MAX_RECT_TILT_ANGLE_DEG,
      sizeStep: 6,
      sampleStep: 6,
      centerStride: 2,
      minWidth: 12,
      minHeight: 12,
    });

    if (largestRectangle) {
      console.log(
        `Largest tilted rectangle: ${Math.round(largestRectangle.width)} x ${Math.round(largestRectangle.height)} px @ ${largestRectangle.angleDeg}° | area ${Math.round(largestRectangle.area)} px²`,
      );
    }
  }

  if (!largestRectangle) {
    console.warn("⚠ No tilted rectangle found in workspace");
    return;
  }

  rectPath = new RotatedRectanglePath(largestRectangle.corners);
  mode = 4;
}

function startSquareMode() {
  if (sq && sq.reset) {
    sq.reset();
  }
  mode = 1;
}

function startCircleMode() {
  if (ci && ci.reset) {
    ci.reset();
  }
  mode = 2;
}

function isMotionPipelineReady() {
  return (
    !arduinoBusy &&
    commandQueue.length === 0 &&
    millis() - lastCommandTime >= MIN_COMMAND_INTERVAL
  );
}

function advanceSingleRunPath(path, onDone = null, allowAdvance = true) {
  if (!path) {
    return [homePos.x, homePos.y];
  }

  if (!allowAdvance) {
    return [penPos.x, penPos.y];
  }

  let [x, y] = path.draw();

  if (
    path.isComplete &&
    path.isComplete() &&
    dist(penPos.x, penPos.y, x, y) < 4
  ) {
    manualPreviewTarget = createVector(x, y);
    mode = 6;
    if (onDone) {
      onDone();
    }
    return [x, y];
  }

  return [x, y];
}

function startStupxMode() {
  let textLayout = getStrokeTextLayout(STUPX_TEXT, {
    gapUnits: 0.22,
    lineGapUnits: 0.35,
  });

  if (!textLayout) {
    console.warn("⚠ Could not build STUPX text layout");
    return;
  }

  stupxTextBox = findLargestTiltedRectangleInWorkspace({
    angleDeg: STUPX_TILT_ANGLE_DEG,
    aspectRatio: textLayout.aspectRatio,
    sizeStep: 6,
    sampleStep: 6,
    centerStride: 2,
    minWidth: 18,
    minHeight: 18,
  });

  if (!stupxTextBox) {
    console.warn("⚠ No valid STUPX text area found in workspace");
    return;
  }

  let textGeometry = buildStrokeTextPathInRotatedRectangle(
    STUPX_TEXT,
    stupxTextBox,
    {
      paddingX: stupxTextBox.width * 0.08,
      paddingY: stupxTextBox.height * 0.18,
      gapUnits: 0.22,
      lineGapUnits: 0.35,
      textScale: STUPX_TEXT_SCALE,
      offsetX: STUPX_TEXT_OFFSET_X,
      offsetY: STUPX_TEXT_OFFSET_Y,
    },
  );

  stupxTextSegments = textGeometry.segments;

  if (textGeometry.pathPoints.length > 1) {
    stupxTextPath = new PolylinePath(textGeometry.pathPoints, 28);
  } else {
    stupxTextPath = null;
  }

  console.log(
    `STUPX text area: ${Math.round(stupxTextBox.width)} x ${Math.round(stupxTextBox.height)} px @ ${stupxTextBox.angleDeg}°`,
  );

  mode = 5;
}

function drawMotors() {
  // Draw motor base line
  stroke(100);
  strokeWeight(3);
  line(motor1Pos.x, motor1Pos.y, motor2Pos.x, motor2Pos.y);

  // Draw motor positions
  fill(50);
  noStroke();
  ellipse(motor1Pos.x, motor1Pos.y, 12, 12);
  ellipse(motor2Pos.x, motor2Pos.y, 12, 12);

  // Labels
  fill(0);
  textSize(10);
  textAlign(CENTER);
  text("M1", motor1Pos.x, motor1Pos.y - 10);
  text("M2", motor2Pos.x, motor2Pos.y - 10);
}

function drawPen() {
  // Draw where each link arm endpoint is
  // Link arm 1 endpoint
  fill(0, 255, 0, 150);
  noStroke();
  ellipse(linkArm1.endpoint.x, linkArm1.endpoint.y, 10, 10);

  // Link arm 2 endpoint
  fill(255, 255, 0, 150);
  ellipse(linkArm2.endpoint.x, linkArm2.endpoint.y, 10, 10);

  // Draw actual pen position with its mounting offset from the joint
  stroke(180, 0, 180, 120);
  strokeWeight(2);
  line(jointPos.x, jointPos.y, penPos.x, penPos.y);

  fill(80, 0, 120, 180);
  noStroke();
  ellipse(jointPos.x, jointPos.y, 8, 8);

  fill(255, 0, 255);
  ellipse(penPos.x, penPos.y, 12, 12);

  // Draw crosshair
  stroke(255, 0, 255);
  strokeWeight(1);
  line(penPos.x - 10, penPos.y, penPos.x + 10, penPos.y);
  line(penPos.x, penPos.y - 10, penPos.x, penPos.y + 10);

  // Draw distance between endpoints (should be ~0)
  let endpointDist = dist(
    linkArm1.endpoint.x,
    linkArm1.endpoint.y,
    linkArm2.endpoint.x,
    linkArm2.endpoint.y,
  );
}

// ============================================================================
// MAIN LOOP
// ============================================================================
function draw() {
  background(240);

  // Draw workspace first (background)
  drawWorkspace();
  drawMotors();

  // Update target position based on mode
  let targetX, targetY;
  let allowPathAdvance = !enableSteppers || isMotionPipelineReady();

  switch (mode) {
    case -1: // Home
      targetX = homePos.x;
      targetY = homePos.y;
      break;

    case 0: // Mouse follow
      targetX = mouseX;
      targetY = mouseY;
      break;

    case 1: // Square
      [targetX, targetY] = advanceSingleRunPath(sq, null, allowPathAdvance);
      break;

    case 2: // Circle
      [targetX, targetY] = advanceSingleRunPath(ci, null, allowPathAdvance);
      break;

    case 3: // Fill workspace dots
      [targetX, targetY] = getFillTarget();
      break;

    case 4: // Largest tilted rectangle
      [targetX, targetY] = advanceSingleRunPath(
        rectPath,
        null,
        allowPathAdvance,
      );
      break;

    case 5: // STUPX text
      [targetX, targetY] = advanceSingleRunPath(
        stupxTextPath,
        null,
        allowPathAdvance,
      );
      break;

    case 6: // Manual XY preview
      if (manualPreviewTarget) {
        targetX = manualPreviewTarget.x;
        targetY = manualPreviewTarget.y;
      } else {
        targetX = penPos.x;
        targetY = penPos.y;
      }
      break;
  }

  // Constrain to workspace
  let solvedJoint = solveJointTargetForPen(targetX, targetY);

  if (solvedJoint) {
    calculateIK(solvedJoint.x, solvedJoint.y);
  } else {
    // Find nearest reachable point
    let nearest = findNearestReachablePoint(targetX, targetY);
    if (nearest) {
      let nearestJoint = solveJointTargetForPen(nearest.x, nearest.y);
      if (nearestJoint) {
        calculateIK(nearestJoint.x, nearestJoint.y);
      }
    }
  }

  // Draw all segments
  motorArm1.show();
  motorArm2.show();
  linkArm1.show();
  linkArm2.show();

  // Draw pen
  drawPen();

  // Send to motors if enabled
  if (enableSteppers) {
    sendMotorCommands();
  }

  // Process command queue
  processCommandQueue();

  // Display info
  displayInfo();
}

function sendMotorCommands() {
  // Convert angles to degrees
  let angle1 = degrees(motorArm1.angle);
  let angle2 = degrees(motorArm2.angle);

  // Only send if angles changed significantly (> 0.5 degrees)
  if (
    lastSentAngle1 !== null &&
    lastSentAngle2 !== null &&
    Math.abs(angle1 - lastSentAngle1) < 0.5 &&
    Math.abs(angle2 - lastSentAngle2) < 0.5
  ) {
    return; // No significant change, don't send
  }

  // Apply -90° offset and direction
  // M1 needs -90° offset, M2 doesn't (different mounting)
  let cmdL = (angle1 - 90) * MOTOR_L_DIR;
  let cmdR = angle2 * MOTOR_R_DIR;

  let cmd = `X${Math.round(cmdR)},Y${Math.round(cmdL)}`;

  // Add to queue instead of sending immediately
  queueCommand(cmd, angle1, angle2);
}

function queueCommand(cmd, angle1, angle2) {
  // Check if this command is already in queue or is same as last command
  if (
    commandQueue.length > 0 &&
    commandQueue[commandQueue.length - 1].cmd === cmd
  ) {
    return; // Don't queue duplicate commands
  }

  if (commandQueue.length >= MAX_COMMAND_QUEUE) {
    return; // Backpressure: keep fidelity, don't drop old path points
  }

  commandQueue.push({ cmd, angle1, angle2 });
}

function processCommandQueue() {
  // Don't send if Arduino is busy or no commands in queue
  if (arduinoBusy || commandQueue.length === 0) {
    return;
  }

  // Enforce minimum time between commands
  let now = millis();
  if (now - lastCommandTime < MIN_COMMAND_INTERVAL) {
    return;
  }

  // Send next command
  let { cmd, angle1, angle2 } = commandQueue.shift();

  if (serial && serial.write) {
    serial.write(cmd + "\n");
    console.log(
      `→ Arduino: ${cmd} | M1=${Math.round(angle1)}° M2=${Math.round(angle2)}° | Queue: ${commandQueue.length}`,
    );

    arduinoBusy = true;
    lastCommandTime = now;
    lastSentAngle1 = angle1;
    lastSentAngle2 = angle2;

    // Safety timeout in case DONE is missed
    setTimeout(() => {
      if (arduinoBusy) {
        arduinoBusy = false;
      }
    }, ARDUINO_BUSY_TIMEOUT_MS);
  }
}
