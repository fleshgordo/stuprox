class Arm {
  constructor(x, y, numSegs, segLen, angle) {
    this.base = createVector(x, y);
    this.segs = [];
    this.segs[0] = new Segment(x, y, segLen, angle, 0);
    for (let i = 1; i < numSegs; i++) {
      this.addSegment(segLen, 0, i + 1);
    }
  }

  // add from root
  addSegment(len, angle) {
    let last = this.segs[this.segs.length - 1];
    let s = new Segment(0, 0, len, angle, this.segs.length);
    last.parent = s;
    this.segs.push(s);
    s.follow(last.from.x, last.from.y);
    return s;
  }

  update() {
    // change this.from
    for (let i = 0; i < this.segs.length; i++) {
      const seg = this.segs[i];
      seg.update();
      if (i === 0) {
        seg.follow(mouseX, mouseY);
      } else {
        const previous = this.segs[i - 1];
        seg.follow(previous.from.x, previous.from.y);
      }
    }

    const last = this.segs.length - 1;
    const s = this.segs[last];

    // fixed point
    s.from.x = this.base.x;
    s.from.y = this.base.y;
    s.update();

    for (let i = last - 1; i >= 0; i--) {
      const seg = this.segs[i];
      const next = this.segs[i + 1];
      seg.from.x = next.to.x;
      seg.from.y = next.to.y;
      seg.update();
    }
  }

  show() {
    this.segs.forEach((s) => s.show());
  }
}
