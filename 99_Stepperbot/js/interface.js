// UI, status, serial callbacks, and keyboard controls

function parseXYCommand(cmdText) {
  if (!cmdText) {
    return null;
  }

  let matchX = cmdText.match(/X\s*(-?\d+(?:\.\d+)?)/i);
  let matchY = cmdText.match(/Y\s*(-?\d+(?:\.\d+)?)/i);

  if (!matchX || !matchY) {
    return null;
  }

  return {
    x: Number(matchX[1]),
    y: Number(matchY[1]),
  };
}

function applyPreviewFromCommandText(cmdText) {
  let parsed = parseXYCommand(cmdText);
  if (!parsed) {
    return false;
  }

  let ok = applyManualXYPreview(parsed.x, parsed.y);
  if (!ok) {
    console.warn(`⚠ Preview failed for command: ${cmdText}`);
  }

  return ok;
}

function displayInfo() {
  let boxX = width - 180;
  let boxY = 8;
  let boxW = 175;
  let boxH = 96;

  fill(255, 255, 255, 210);
  stroke(180);
  strokeWeight(1);
  rect(boxX, boxY, boxW, boxH, 8);

  fill(0);
  noStroke();
  textSize(12);
  textAlign(RIGHT);

  let x = width - 20;
  let y = 23;
  let modeNames = [
    "Home",
    "Mouse",
    "Square",
    "Circle",
    "Fill",
    "MaxRect",
    "STUPX",
    "Manual",
  ];
  text(`Mode: ${modeNames[mode + 1]}`, x, y);
  y += 15;
  text(`Steppers: ${enableSteppers ? "ON" : "OFF"}`, x, y);
  y += 15;
  text(`Motor 1: ${Math.round(degrees(motorArm1.angle))}°`, x, y);
  y += 15;
  text(`Motor 2: ${Math.round(degrees(motorArm2.angle))}°`, x, y);
  y += 15;
  let cmdY = Math.round((degrees(motorArm1.angle) - 90) * MOTOR_L_DIR);
  let cmdX = Math.round(degrees(motorArm2.angle) * MOTOR_R_DIR);
  text(`Cmd: X${cmdX},Y${cmdY}`, x, y);
  y += 15;
  text(`Pen: (${Math.round(penPos.x)}, ${Math.round(penPos.y)})`, x, y);
  y += 15;

  let queueStatus = arduinoBusy ? "BUSY" : "READY";
  text(`Arduino: ${queueStatus} | Queue: ${commandQueue.length}`, x, y);
}

function toggleSteppers(sourceLabel = "") {
  enableSteppers = !enableSteppers;

  if (enableSteppers) {
    servoButton.html("Steppers: ON");
    servoButton.style("background-color", "#ccffcc");
    serial.write("H\n");
    serial.write("E\n");
    lastSentAngle1 = null;
    lastSentAngle2 = null;
    console.log(
      `✅ STEPPERS ENABLED${sourceLabel} - Sent H and E commands to Arduino`,
    );
    return;
  }

  servoButton.html("Steppers: OFF");
  servoButton.style("background-color", "#ffcccc");
  mode = -1;
  serial.write("E\n");
  console.log(`⛔ STEPPERS DISABLED${sourceLabel} - Sent E command to Arduino`);
}

function initUI() {
  let btn = createButton("Home");
  btn.position(10, 10);
  btn.mousePressed(() => {
    mode = -1;
  });

  btn = createButton("Mouse");
  btn.position(70, 10);
  btn.mousePressed(() => {
    mode = 0;
  });

  btn = createButton("Square");
  btn.position(135, 10);
  btn.mousePressed(() => {
    startSquareMode();
  });

  btn = createButton("Circle");
  btn.position(205, 10);
  btn.mousePressed(() => {
    startCircleMode();
  });

  btn = createButton("Fill");
  btn.position(265, 10);
  btn.mousePressed(() => {
    startFillMode();
  });

  btn = createButton("Max Rect");
  btn.position(305, 10);
  btn.mousePressed(() => {
    startLargestRectangleMode();
  });

  btn = createButton("STUPX");
  btn.position(380, 10);
  btn.mousePressed(() => {
    startStupxMode();
  });

  servoButton = createButton("Steppers: ON");
  servoButton.position(445, 10);
  servoButton.html("Steppers: OFF");
  servoButton.style("background-color", "#ffcccc");
  servoButton.mousePressed(() => {
    toggleSteppers();
  });

  createP("Manual Command:").position(10, 38).style("margin", "0");
  let cmdInput = createInput("");
  cmdInput.position(10, 60);
  cmdInput.size(120);
  cmdInput.attribute("placeholder", "X45,Y90");

  let sendBtn = createButton("Send");
  sendBtn.position(140, 60);
  sendBtn.mousePressed(() => {
    let cmd = cmdInput.value();
    if (cmd && serial && serial.write) {
      applyPreviewFromCommandText(cmd);
      serial.write(cmd + "\n");
      console.log(`📤 Manual: ${cmd}`);
      cmdInput.value("");
    }
  });

  createP("Multi-Coordinates:").position(10, 92).style("margin", "0");
  let coordTextarea = createElement("textarea");
  coordTextarea.position(10, 114);
  coordTextarea.size(125, 60);
  coordTextarea.attribute("placeholder", "X45,Y90\nX60,Y120\n...");

  let sendMultiBtn = createButton("Send All");
  sendMultiBtn.position(10, 182);
  sendMultiBtn.mousePressed(() => {
    let coords = coordTextarea.value().split("\n");
    let count = 0;

    for (let coord of coords) {
      coord = coord.trim();
      if (coord && serial && serial.write) {
        setTimeout(() => {
          applyPreviewFromCommandText(coord);
          serial.write(coord + "\n");
          console.log(`📤 Multi [${count}]: ${coord}`);
        }, count * 1000);
        count++;
      }
    }

    if (count > 0) {
      console.log(`📤 Queued ${count} commands`);
    }
  });

  let debugBtn = createButton("Debug IK");
  debugBtn.position(90, 182);
  debugBtn.mousePressed(() => {
    logIKDebugToUI();
  });

  let ikDebugTitle = createP("IK Debug Log:");
  ikDebugTitle.style("margin", "0");
  ikDebugTitle.style("position", "fixed");
  ikDebugTitle.style("left", "10px");
  ikDebugTitle.style("bottom", "116px");

  ikDebugLogArea = createElement("textarea");
  ikDebugLogArea.style("position", "fixed");
  ikDebugLogArea.style("left", "10px");
  ikDebugLogArea.style("bottom", "10px");
  ikDebugLogArea.size(260, 92);
  ikDebugLogArea.attribute("readonly", "readonly");
}

function serverConnected() {
  console.log("✅ Connected to serial server");
  console.log(
    "💡 With steppers OFF, manually set motors to X-90,Y0, then enable them",
  );
}

function serialEvent() {
  let data = serial.readLine();
  if (data.length > 0) {
    console.log(`📥 Arduino: ${data}`);

    if (data.includes("DONE") || data.includes("Current position")) {
      arduinoBusy = false;
    }
  }
}

function keyPressed() {
  if (key === "e" || key === "E") {
    toggleSteppers(" (keyboard)");
    return false;
  }

  if (key === "r" || key === "R") {
    logIKDebugToUI();
    return false;
  }
}
