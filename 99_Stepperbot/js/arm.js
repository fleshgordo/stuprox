class Arm {
  constructor(x, y, segArm, segBase, segFollow) {
    this.base = createVector(x, y);
    this.segArm = segArm;
    this.segBase = segBase;
    this.segFollow = segFollow;
    this.disableFollow = false;
    this.oldMouseX = 0;
    this.oldMouseY = 0;
    this.offset = 0; // adjust!!
  }

  setBase() {
    this.segBase.setBase(this.base)
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
    this.segBase.follow(this.segArm.from.x, this.segArm.from.y)
    this.setBase();
  }

  calculateOffset() {
    if (this.segFollow !== undefined) {
      let dist = Math.abs(this.segArm.to.x - this.segFollow.to.x)
      if (dist > 15) {
        this.disableFollow = true;
        console.log(`WARNING -> dist: ${dist}`);
        this.segFollow.follow(this.segArm.to.x, this.segArm.to.y);
      } else {
        this.disableFollow = false;
      }
    }

  }

  moveServo(serial, angle_1,angle_2) {
    let cmd = `X${int(degrees(angle_1))+this.offset},Y${int(degrees(angle_2))+this.offset}\n`
    serial.write(cmd)
    console.log(cmd) 
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