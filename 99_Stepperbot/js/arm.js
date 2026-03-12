class Arm {
  constructor(x, y, segArm, segBase, segFollow, motorLDir = 1, motorRDir = 1) {
    this.base = createVector(x, y);
    this.segArm = segArm;
    this.segBase = segBase;
    this.segFollow = segFollow;
    this.disableFollow = false;
    this.oldMouseX = 0;
    this.oldMouseY = 0;
    this.offset = 0; // adjust!!
    this.lastAngle1 = null;
    this.lastAngle2 = null;
    this.motorLDir = motorLDir; // Motor direction multiplier
    this.motorRDir = motorRDir; // Motor direction multiplier
  }

  setBase() {
    this.segBase.setBase(this.base);
  }

  follow(_x, _y) {
    //this.calculateOffset();

    if (this.segFollow !== undefined) {
      // follow the other end point from robot arm
      this.segArm.follow(this.segFollow.to.x, this.segFollow.to.y);
    } else {
      // if (this.disableFollow === false && this.segFollow !== undefined) this.segArm.follow(_x, _y);
      if (this.disableFollow !== true) this.segArm.follow(_x, _y);
    }
    this.segBase.follow(this.segArm.from.x, this.segArm.from.y);
    this.setBase();
  }

  calculateOffset() {
    if (this.segFollow !== undefined) {
      let dist = Math.abs(this.segArm.to.x - this.segFollow.to.x);
      if (dist > 15) {
        this.disableFollow = true;
        console.log(`WARNING -> dist: ${dist}`);
        this.segFollow.follow(this.segArm.to.x, this.segArm.to.y);
      } else {
        this.disableFollow = false;
      }
    }
  }

  moveServo(serial, angle_1, angle_2) {
    // Apply -90-degree offset to compensate for physical mounting
    // angle_1 → X command → stepperR (right motor) → use motorRDir
    // angle_2 → Y command → stepperL (left motor) → use motorLDir

    // Raw angles from inverse kinematics (in degrees)
    let rawAngle1 = int(degrees(angle_1));
    let rawAngle2 = int(degrees(angle_2));

    // Apply offset and direction
    let newAngle1 = (rawAngle1 - 90 + this.offset) * this.motorRDir;
    let newAngle2 = (rawAngle2 - 90 + this.offset) * this.motorLDir;

    // Only send if angles have changed significantly (reduce jitter)
    if (
      !this.lastAngle1 ||
      !this.lastAngle2 ||
      Math.abs(newAngle1 - this.lastAngle1) > 0.5 ||
      Math.abs(newAngle2 - this.lastAngle2) > 0.5
    ) {
      let cmd = `X${newAngle1},Y${newAngle2}\n`;
      serial.write(cmd);

      // Detailed logging
      console.log(
        `IK→ L:${rawAngle1}° R:${rawAngle2}° | After -90° & dir→ X:${newAngle1}° Y:${newAngle2}° | ${cmd.trim()}`,
      );

      this.lastAngle1 = newAngle1;
      this.lastAngle2 = newAngle2;
    }
  }

  update() {
    this.segBase.update();
    this.segArm.update();
  }

  show() {
    this.segBase.show();
    this.segArm.show();
  }
}
