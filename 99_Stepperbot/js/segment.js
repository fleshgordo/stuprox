class Segment {
  constructor(x, y, len, angle, id) {
    this.from = createVector(x, y); // a
    this.to = createVector(); // b
    this.len = len;
    this.angle = angle;
    this.id = id;
    this.parent = null;
    this.dir = true;
    this.reCalculate();
    this.maxDeg = 180;
    this.minDeg = 0;
  }

  createParent(len, angle, id) {
    let parent = new Segment(0, 0, len, angle, id);
    this.parent = parent;
    parent.follow(this.from.x, this.from.y);
    this.reCalculate();
    return this.parent;
  }

  setBase(base) {
    this.from = p5.Vector.copy(base)
  }

  follow(target_x, target_y) {
    let target = createVector(target_x, target_y);
    let dir = p5.Vector.sub(target, this.from);
    this.angle = dir.heading();
    if (this.parent == null) {
      this.checkLimits();
      //console.log(`motor id: ${this.id} angle: ${this.angle * 180 / PI} `)
    }
    dir.setMag(this.len)
    dir.mult(-1)
    this.from = p5.Vector.add(target, dir)
  }

  reCalculate() {
    let dx = cos(this.angle) * this.len;
    let dy = sin(this.angle) * this.len;
    this.to.set(this.from.x + dx, this.from.y + dy); // b
  }

  checkLimits() {
    if (degrees(this.angle) > this.maxDeg) {
      this.angle = radians(this.maxDeg)
    }
    if (degrees(this.angle) < this.minDeg) {
      this.angle = radians(this.minDeg)
    }
    this.reCalculate();
  }

  update() {
    if (this.parent != null) {
      this.from = this.parent.to.copy(); // a=
    }
    this.reCalculate();
  }

  show() {
    strokeWeight(2);
    this.parent == null ? stroke(255, 0, 0) : stroke(0);

    line(this.from.x, this.from.y, this.to.x, this.to.y);
    noStroke();
    fill(51);
    strokeWeight(5);
    ellipse(this.from.x, this.from.y, 5, 5);
    ellipse(this.to.x, this.to.y, 5, 5);
  }
}