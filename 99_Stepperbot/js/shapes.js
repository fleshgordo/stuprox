class Square {
  constructor(x, y, size, startFromRight = false, rotated45 = false) {
    this.x = x;
    this.y = y;
    this.size = size;
    this.startFromRight = startFromRight;
    this.rotated45 = rotated45;
    this.interpolationTime = 50;
    this.reset();
  }

  reset() {
    this.segmentIndex = 0;
    this.segmentFrame = 0;
    this.complete = false;
  }

  isComplete() {
    return this.complete;
  }

  getPathPoints() {
    let points = this.rotated45
      ? (() => {
          let halfDiagonal = this.size / Math.sqrt(2);
          return [
            [this.x, this.y],
            [this.x + halfDiagonal, this.y + halfDiagonal],
            [this.x, this.y + halfDiagonal * 2],
            [this.x - halfDiagonal, this.y + halfDiagonal],
          ];
        })()
      : this.startFromRight
        ? [
            [this.x + this.size, this.y],
            [this.x + this.size, this.y + this.size],
            [this.x, this.y + this.size],
            [this.x, this.y],
          ]
        : [
            [this.x, this.y],
            [this.x, this.y + this.size],
            [this.x + this.size, this.y + this.size],
            [this.x + this.size, this.y],
          ];

    return [...points, points[0]];
  }

  grow() {
    this.size += 5;
  }

  draw() {
    let points = this.getPathPoints();

    if (this.complete) {
      return points[points.length - 1];
    }

    stroke(0);
    strokeWeight(1);
    beginShape(POINTS);
    let t = this.segmentFrame / this.interpolationTime;
    let p1 = points[this.segmentIndex];
    let p2 = points[this.segmentIndex + 1];
    let x = lerp(p1[0], p2[0], t);
    let y = lerp(p1[1], p2[1], t);
    vertex(x, y);
    endShape();

    this.segmentFrame++;
    if (this.segmentFrame > this.interpolationTime) {
      this.segmentFrame = 0;
      this.segmentIndex++;

      if (this.segmentIndex >= points.length - 1) {
        this.segmentIndex = points.length - 2;
        this.complete = true;
      }
    }

    return [x, y];
  }
}

class myCircle {
  constructor(x, y, r, angle) {
    this.x = x;
    this.y = y;
    this.r = r;
    this.startAngle = angle;
    this.stepSize = 1;
    this.interpolationTime = 1;
    this.reset();
  }

  reset() {
    this.angle = this.startAngle;
    this.complete = false;
  }

  isComplete() {
    return this.complete;
  }

  getOutlinePoints(stepDeg = 10) {
    let points = [];
    for (
      let angle = this.startAngle;
      angle <= this.startAngle + 360;
      angle += stepDeg
    ) {
      points.push([
        this.x + this.r * sin(radians(angle)),
        this.y + this.r * cos(radians(angle)),
      ]);
    }
    return points;
  }

  draw() {
    let circle_x = this.x + this.r * sin(radians(this.angle));
    let circle_y = this.y + this.r * cos(radians(this.angle));
    fill(0, 255, 0);
    ellipse(circle_x, circle_y, 5);

    if (!this.complete) {
      this.angle += this.stepSize;
      if (this.angle >= this.startAngle + 360) {
        this.angle = this.startAngle + 360;
        this.complete = true;
      }
    }

    return [circle_x, circle_y];
  }
}

class PolylinePath {
  constructor(points, interpolationTime = 40) {
    this.points = (points || []).map((point) => [point.x, point.y]);
    this.interpolationTime = interpolationTime;
    this.reset();
  }

  reset() {
    this.segmentIndex = 0;
    this.segmentFrame = 0;
    this.complete = false;
  }

  isComplete() {
    return this.complete;
  }

  draw() {
    if (this.points.length === 0) {
      return [0, 0];
    }

    if (this.points.length === 1) {
      let [x, y] = this.points[0];
      return [x, y];
    }

    if (this.complete) {
      return this.points[this.points.length - 1];
    }

    stroke(255, 90, 0);
    strokeWeight(1);
    beginShape(POINTS);

    let t = this.segmentFrame / this.interpolationTime;
    let p1 = this.points[this.segmentIndex];
    let p2 = this.points[this.segmentIndex + 1];
    let x = lerp(p1[0], p2[0], t);
    let y = lerp(p1[1], p2[1], t);

    vertex(x, y);
    endShape();

    this.segmentFrame++;
    if (this.segmentFrame > this.interpolationTime) {
      this.segmentFrame = 0;
      this.segmentIndex++;

      if (this.segmentIndex >= this.points.length - 1) {
        this.segmentIndex = this.points.length - 2;
        this.complete = true;
      }
    }

    return [x, y];
  }
}

class RotatedRectanglePath {
  constructor(corners) {
    this.points = corners.map((corner) => [corner.x, corner.y]);
    this.points.push(this.points[0]);
    this.interpolationTime = 50;
    this.reset();
  }

  reset() {
    this.segmentIndex = 0;
    this.segmentFrame = 0;
    this.complete = false;
  }

  isComplete() {
    return this.complete;
  }

  draw() {
    if (this.complete) {
      return this.points[this.points.length - 1];
    }

    stroke(255, 140, 0);
    strokeWeight(1);
    beginShape(POINTS);

    let t = this.segmentFrame / this.interpolationTime;
    let p1 = this.points[this.segmentIndex];
    let p2 = this.points[this.segmentIndex + 1];
    let x = lerp(p1[0], p2[0], t);
    let y = lerp(p1[1], p2[1], t);

    vertex(x, y);
    endShape();

    this.segmentFrame++;
    if (this.segmentFrame > this.interpolationTime) {
      this.segmentFrame = 0;
      this.segmentIndex++;

      if (this.segmentIndex >= this.points.length - 1) {
        this.segmentIndex = this.points.length - 2;
        this.complete = true;
      }
    }

    return [x, y];
  }
}

function getStrokeGlyphDefinition(char) {
  switch (char) {
    case "S":
      return {
        width: 1,
        segments: [
          [
            { x: 1, y: 0 },
            { x: 0.2, y: 0 },
            { x: 0, y: 0.2 },
            { x: 0, y: 0.42 },
            { x: 0.2, y: 0.5 },
            { x: 0.8, y: 0.5 },
            { x: 1, y: 0.58 },
            { x: 1, y: 0.8 },
            { x: 0.8, y: 1 },
            { x: 0, y: 1 },
          ],
        ],
      };

    case "T":
      return {
        width: 1,
        segments: [
          [
            { x: 0, y: 0 },
            { x: 1, y: 0 },
          ],
          [
            { x: 0.5, y: 0 },
            { x: 0.5, y: 1 },
          ],
        ],
      };

    case "P":
      return {
        width: 1,
        segments: [
          [
            { x: 0, y: 1 },
            { x: 0, y: 0 },
            { x: 0.75, y: 0 },
            { x: 1, y: 0.18 },
            { x: 1, y: 0.35 },
            { x: 0.75, y: 0.5 },
            { x: 0, y: 0.5 },
          ],
        ],
      };

    case "X":
      return {
        width: 1,
        segments: [
          [
            { x: 0, y: 0 },
            { x: 1, y: 1 },
          ],
          [
            { x: 1, y: 0 },
            { x: 0, y: 1 },
          ],
        ],
      };

    case "2":
      return {
        width: 1,
        segments: [
          [
            { x: 0, y: 0.15 },
            { x: 0.2, y: 0 },
            { x: 0.8, y: 0 },
            { x: 1, y: 0.2 },
            { x: 1, y: 0.42 },
            { x: 0, y: 1 },
            { x: 1, y: 1 },
          ],
        ],
      };

    case "6":
      return {
        width: 1,
        segments: [
          [
            { x: 1, y: 0.1 },
            { x: 0.8, y: 0 },
            { x: 0.2, y: 0 },
            { x: 0, y: 0.25 },
            { x: 0, y: 0.82 },
            { x: 0.2, y: 1 },
            { x: 0.8, y: 1 },
            { x: 1, y: 0.82 },
            { x: 1, y: 0.6 },
            { x: 0.8, y: 0.5 },
            { x: 0.2, y: 0.5 },
            { x: 0, y: 0.62 },
          ],
        ],
      };

    default:
      return null;
  }
}

function getStrokeTextLayout(text, options = {}) {
  let lines = text
    .toUpperCase()
    .split("\n")
    .map((line) =>
      line
        .split("")
        .map((char) => getStrokeGlyphDefinition(char))
        .filter((glyph) => glyph !== null),
    )
    .filter((lineGlyphs) => lineGlyphs.length > 0);

  if (lines.length === 0) {
    return null;
  }

  let gapUnits = options.gapUnits ?? 0.28;
  let lineGapUnits = options.lineGapUnits ?? 0.45;

  let lineWidths = lines.map((glyphs) => {
    let widthUnits = glyphs.reduce((sum, glyph) => sum + glyph.width, 0);
    return widthUnits + gapUnits * Math.max(0, glyphs.length - 1);
  });

  let maxLineUnits = Math.max(...lineWidths);
  let totalHeightUnits =
    lines.length + lineGapUnits * Math.max(0, lines.length - 1);

  return {
    lines,
    lineWidths,
    maxLineUnits,
    totalHeightUnits,
    gapUnits,
    lineGapUnits,
    aspectRatio: maxLineUnits / totalHeightUnits,
  };
}

function buildStrokeTextPathInRotatedRectangle(text, rectangle, options = {}) {
  if (!rectangle || !rectangle.corners || rectangle.corners.length < 4) {
    return { segments: [], pathPoints: [] };
  }

  let layout = getStrokeTextLayout(text, options);
  if (!layout) {
    return { segments: [], pathPoints: [] };
  }

  let {
    lines,
    lineWidths,
    maxLineUnits,
    totalHeightUnits,
    gapUnits,
    lineGapUnits,
  } = layout;
  let paddingX = options.paddingX ?? rectangle.width * 0.08;
  let paddingY = options.paddingY ?? rectangle.height * 0.2;
  let textScale = options.textScale ?? 1;
  let offsetX = options.offsetX ?? 0;
  let offsetY = options.offsetY ?? 0;
  let usableWidth = Math.max(1, rectangle.width - paddingX * 2);
  let usableHeight = Math.max(1, rectangle.height - paddingY * 2);

  let scale =
    Math.min(usableWidth / maxLineUnits, usableHeight / totalHeightUnits) *
    textScale;
  let renderedHeight = totalHeightUnits * scale;
  let startY = (rectangle.height - renderedHeight) / 2 + offsetY;

  let topLeft = rectangle.corners[0];
  let topRight = rectangle.corners[1];
  let bottomLeft = rectangle.corners[3];
  let axisX = {
    x: (topRight.x - topLeft.x) / rectangle.width,
    y: (topRight.y - topLeft.y) / rectangle.width,
  };
  let axisY = {
    x: (bottomLeft.x - topLeft.x) / rectangle.height,
    y: (bottomLeft.y - topLeft.y) / rectangle.height,
  };

  let toWorldPoint = (localX, localY) => ({
    x: topLeft.x + axisX.x * localX + axisY.x * localY,
    y: topLeft.y + axisX.y * localX + axisY.y * localY,
  });

  let segments = [];
  let pathPoints = [];

  for (let lineIndex = 0; lineIndex < lines.length; lineIndex++) {
    let glyphs = lines[lineIndex];
    let lineWidth = lineWidths[lineIndex] * scale;
    let startX = (rectangle.width - lineWidth) / 2 + offsetX;
    let cursorUnits = 0;
    let glyphOffsetY = startY + lineIndex * (1 + lineGapUnits) * scale;

    for (let i = 0; i < glyphs.length; i++) {
      let glyph = glyphs[i];
      let glyphOffsetX = startX + cursorUnits * scale;

      for (let segment of glyph.segments) {
        let worldSegment = segment.map((point) =>
          toWorldPoint(
            glyphOffsetX + point.x * scale,
            glyphOffsetY + point.y * scale,
          ),
        );

        if (worldSegment.length > 0) {
          segments.push(worldSegment);
          pathPoints.push(...worldSegment);
        }
      }

      cursorUnits += glyph.width + gapUnits;
    }
  }

  return { segments, pathPoints };
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
    let t = (frameCount % this.interpolationTime) / this.interpolationTime; // calculate the ratio between 0 and 1 based on frameCount
    let t2 =
      (frameCount % (this.interpolationTime * this.maxDeg)) /
      (this.interpolationTime * this.maxDeg);
    this.servoAngle1 = lerp(this.minDeg, this.maxDeg, t);
    this.servoAngle2 = lerp(this.minDeg, this.maxDeg, t2);
  }
  draw() {
    return [int(this.servoAngle1), int(this.servoAngle2)];
  }
}
