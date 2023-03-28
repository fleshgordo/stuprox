class Square {
    constructor(x, y, size) {
        this.x = x;
        this.y = y;
        this.size = size;
        this.index = 0;
        this.interpolationTime = 50;
    }
    grow() {
        this.size += 5
    }
    draw() {
        let points = [
            [this.x, this.y],
            [this.x + this.size, this.y],
            [this.x + this.size, this.y + this.size],
            [this.x, this.y + this.size]
        ]
        stroke(0)
        strokeWeight(1)
        beginShape(POINTS);
        //if (frameCount % 200 === 0) this.grow()
        if (frameCount %  this.interpolationTime === 0) this.index += 1;
        let t = (frameCount %  this.interpolationTime) /  this.interpolationTime; // calculate the ratio between 0 and 1 based on frameCount
        let p1 = points[this.index % 4];
        let p2 = points[(this.index + 1) % 4];
        let x = lerp(p1[0], p2[0], t);
        let y = lerp(p1[1], p2[1], t);
        // let x = points[this.index % 4][0]
        // let y = points[this.index % 4][1]
        vertex(x, y)
        endShape();
        return [x, y]
    }
}

class myCircle {
    constructor(x, y, r, angle) {
        this.x = x;
        this.y = y;
        this.r = r;
        this.angle = angle;
    }
    update() {
        this.angle += 1;
    }
    draw() {
        let circle_x = this.x + this.r * sin(radians(this.angle));
        let circle_y = this.y + this.r * cos(radians(this.angle));
        fill(0, 255, 0)
        ellipse(circle_x, circle_y, 5);
        return [circle_x, circle_y];
    }
}

class allPoints {
    constructor(t) {
        this.servoAngle1 = 0;
        this.servoAngle2 = 0;
        this.minDeg = 50;
        this.maxDeg = 120;
        this.interpolationTime = t;
    }
    update() {
        let t = (frameCount %  this.interpolationTime) /  this.interpolationTime; // calculate the ratio between 0 and 1 based on frameCount
        let t2 = (frameCount %  (this.interpolationTime * this.maxDeg)) /  (this.interpolationTime * this.maxDeg);
        this.servoAngle1 = lerp(this.minDeg, this.maxDeg, t);
        this.servoAngle2 = lerp(this.minDeg, this.maxDeg, t2);
    }
    draw() {
        return [int(this.servoAngle1), int(this.servoAngle2)]
    }
}