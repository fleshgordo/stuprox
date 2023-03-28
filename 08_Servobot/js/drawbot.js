class Bot {
  constructor(arm1,arm2) {
    this.arm1 = arm1;
    this.arm2 = arm2;
  }

  update() {
    this.arm1.update();
    this.arm2.update();
  }

  reCalculate() {
    this.arm1.calculcate(this.arm2);
    this.arm2.calculcate(this.arm1);
  }
  show() {
    this.arm1.show();
    this.arm2.show();
  }
}
