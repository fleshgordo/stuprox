// Simple segment helper for the 4-bar linkage visualization
// Each segment has a fixed pivot point and a computed endpoint
class Segment {
  constructor(pivotX, pivotY, length, angle = 0, color = [0, 0, 0]) {
    this.pivot = createVector(pivotX, pivotY); // Fixed pivot point
    this.endpoint = createVector(); // Moving endpoint
    this.length = length;
    this.angle = angle;
    this.color = color;
    this.updateEndpoint();
  }

  // Set the angle and recalculate endpoint
  setAngle(angle) {
    this.angle = angle;
    this.updateEndpoint();
  }

  // Calculate endpoint position based on angle
  updateEndpoint() {
    this.endpoint.x = this.pivot.x + cos(this.angle) * this.length;
    this.endpoint.y = this.pivot.y + sin(this.angle) * this.length;
  }

  // Draw the segment
  show() {
    stroke(this.color[0], this.color[1], this.color[2]);
    strokeWeight(3);
    line(this.pivot.x, this.pivot.y, this.endpoint.x, this.endpoint.y);

    // Draw pivot point
    fill(this.color[0], this.color[1], this.color[2]);
    noStroke();
    ellipse(this.pivot.x, this.pivot.y, 8, 8);

    // Draw endpoint
    ellipse(this.endpoint.x, this.endpoint.y, 6, 6);
  }
}
