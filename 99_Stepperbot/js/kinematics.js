// Kinematics, workspace sampling, and fill-path helpers for the 4-bar linkage

function circleIntersection(x1, y1, r1, x2, y2, r2) {
  let d = dist(x1, y1, x2, y2);

  if (d > r1 + r2 || d < Math.abs(r1 - r2) || d === 0) {
    return null;
  }

  let a = (r1 * r1 - r2 * r2 + d * d) / (2 * d);
  let h = sqrt(r1 * r1 - a * a);
  let cx = x1 + (a * (x2 - x1)) / d;
  let cy = y1 + (a * (y2 - y1)) / d;

  let intersect1 = {
    x: cx + (h * (y2 - y1)) / d,
    y: cy - (h * (x2 - x1)) / d,
  };

  let intersect2 = {
    x: cx - (h * (y2 - y1)) / d,
    y: cy + (h * (x2 - x1)) / d,
  };

  return intersect1.y > intersect2.y ? intersect1 : intersect2;
}

function getMotor1AngleLimits() {
  return {
    min: radians(MOTOR_1_MIN_ANGLE_DEG),
    max: radians(MOTOR_1_MAX_ANGLE_DEG),
  };
}

function getMotor2AngleLimits() {
  return {
    min: radians(MOTOR_2_MIN_ANGLE_DEG),
    max: radians(MOTOR_2_MAX_ANGLE_DEG),
  };
}

function calculateWorkspace() {
  workspacePolygon = [];
  workspacePoints = [];

  const centerX = MOTOR1_X + (MOTOR_SEPARATION * SCALE) / 2;
  const centerY = MOTOR1_Y;
  const testRadius =
    (MOTOR_ARM_LENGTH + LINK_ARM_2_LENGTH + PEN_OFFSET) * SCALE;
  const step = 6;

  for (let y = centerY + step; y <= centerY + testRadius; y += step) {
    for (let x = centerX - testRadius; x <= centerX + testRadius; x += step) {
      let debug = canReachWithIKDebug(x, y);
      if (!debug.reachable) continue;

      workspacePoints.push({ x, y });
    }
  }

  workspacePoints = buildCoherentWorkspaceSurface(workspacePoints, step);

  for (let point of workspacePoints) {
    let angle = degrees(atan2(point.y - centerY, point.x - centerX));
    if (angle < 0) angle += 360;

    workspacePolygon.push({
      x: point.x,
      y: point.y,
      angle,
      radius: dist(centerX, centerY, point.x, point.y),
    });
  }

  if (workspacePolygon.length === 0) return;

  workspacePolygon.sort((a, b) => a.angle - b.angle);

  let cleaned = [];
  let currentAngle = -1;
  let maxRadius = 0;
  let maxPoint = null;

  for (let p of workspacePolygon) {
    if (Math.abs(p.angle - currentAngle) > 0.1) {
      if (maxPoint) cleaned.push(maxPoint);
      currentAngle = p.angle;
      maxRadius = p.radius;
      maxPoint = p;
    } else if (p.radius > maxRadius) {
      maxRadius = p.radius;
      maxPoint = p;
    }
  }

  if (maxPoint) cleaned.push(maxPoint);
  workspacePolygon = cleaned;
}

function buildCoherentWorkspaceSurface(points, step) {
  if (!points || points.length === 0) {
    return [];
  }

  function key(ix, iy) {
    return `${ix},${iy}`;
  }

  function pointToIndex(value) {
    return Math.round(value / step);
  }

  function keyToPoint(gridKey) {
    let [ix, iy] = gridKey.split(",").map(Number);
    return { x: ix * step, y: iy * step };
  }

  let occupied = new Map();
  for (let point of points) {
    let ix = pointToIndex(point.x);
    let iy = pointToIndex(point.y);
    occupied.set(key(ix, iy), { x: ix * step, y: iy * step });
  }

  function countNeighbors(ix, iy, sourceMap) {
    let count = 0;
    for (let [dx, dy] of neighbors8) {
      if (sourceMap.has(key(ix + dx, iy + dy))) {
        count++;
      }
    }
    return count;
  }

  function isLowAngleMargin(debug) {
    return (
      debug.minAngleMarginDeg !== undefined &&
      debug.minAngleMarginDeg < WORKSPACE_MIN_ANGLE_MARGIN_DEG
    );
  }

  let candidateSupport = new Map();
  let neighbors8 = [
    [1, 0],
    [-1, 0],
    [0, 1],
    [0, -1],
    [1, 1],
    [1, -1],
    [-1, 1],
    [-1, -1],
  ];
  let neighbors4 = [
    [1, 0],
    [-1, 0],
    [0, 1],
    [0, -1],
  ];

  for (let gridKey of occupied.keys()) {
    let [ix, iy] = gridKey.split(",").map(Number);
    for (let [dx, dy] of neighbors8) {
      let nk = key(ix + dx, iy + dy);
      if (occupied.has(nk)) continue;
      candidateSupport.set(nk, (candidateSupport.get(nk) || 0) + 1);
    }
  }

  for (let [candidateKey, support] of candidateSupport.entries()) {
    if (support < 6) continue;
    let candidatePoint = keyToPoint(candidateKey);
    let debug = canReachWithIKDebug(candidatePoint.x, candidatePoint.y);
    if (!debug.reachable) continue;

    occupied.set(candidateKey, candidatePoint);
  }

  let rowMap = new Map();
  for (let gridKey of occupied.keys()) {
    let [ix, iy] = gridKey.split(",").map(Number);
    if (!rowMap.has(iy)) rowMap.set(iy, []);
    rowMap.get(iy).push(ix);
  }

  for (let [iy, xIndices] of rowMap.entries()) {
    xIndices.sort((a, b) => a - b);
    for (let i = 0; i < xIndices.length - 1; i++) {
      let left = xIndices[i];
      let right = xIndices[i + 1];
      let gap = right - left - 1;
      if (gap <= 0 || gap > 2) continue;

      for (let ix = left + 1; ix < right; ix++) {
        let candidateKey = key(ix, iy);
        if (occupied.has(candidateKey)) continue;
        let candidatePoint = { x: ix * step, y: iy * step };
        let debug = canReachWithIKDebug(candidatePoint.x, candidatePoint.y);
        if (!debug.reachable) continue;
        occupied.set(candidateKey, candidatePoint);
      }
    }
  }

  let pruned = new Map();
  for (let gridKey of occupied.keys()) {
    let [ix, iy] = gridKey.split(",").map(Number);
    let neighbors = countNeighbors(ix, iy, occupied);
    if (neighbors <= 1) continue;

    let point = occupied.get(gridKey);
    let debug = canReachWithIKDebug(point.x, point.y);
    if (!debug.reachable) continue;
    if (isLowAngleMargin(debug) && neighbors < 5) {
      continue;
    }

    pruned.set(gridKey, point);
  }

  occupied = pruned;

  let secondPrune = new Map();
  for (let gridKey of occupied.keys()) {
    let [ix, iy] = gridKey.split(",").map(Number);
    if (countNeighbors(ix, iy, occupied) < WORKSPACE_MIN_NEIGHBORS) continue;
    secondPrune.set(gridKey, occupied.get(gridKey));
  }

  occupied = secondPrune;

  for (let pass = 0; pass < 2; pass++) {
    let spurPruned = new Map();
    for (let gridKey of occupied.keys()) {
      let [ix, iy] = gridKey.split(",").map(Number);
      let neighbors = 0;
      for (let [dx, dy] of neighbors4) {
        if (occupied.has(key(ix + dx, iy + dy))) {
          neighbors++;
        }
      }

      if (neighbors <= 1) continue;
      spurPruned.set(gridKey, occupied.get(gridKey));
    }
    occupied = spurPruned;
    if (occupied.size === 0) {
      return [];
    }
  }

  let thirdPrune = new Map();
  for (let gridKey of occupied.keys()) {
    let [ix, iy] = gridKey.split(",").map(Number);
    if (countNeighbors(ix, iy, occupied) < Math.max(3, WORKSPACE_MIN_NEIGHBORS)) {
      continue;
    }
    thirdPrune.set(gridKey, occupied.get(gridKey));
  }

  occupied = thirdPrune;

  if (occupied.size === 0) {
    return [];
  }

  let visited = new Set();
  let largestComponent = [];

  for (let startKey of occupied.keys()) {
    if (visited.has(startKey)) continue;

    let queue = [startKey];
    visited.add(startKey);
    let component = [];

    while (queue.length > 0) {
      let currentKey = queue.shift();
      component.push(currentKey);

      let [ix, iy] = currentKey.split(",").map(Number);
      for (let [dx, dy] of neighbors4) {
        let nk = key(ix + dx, iy + dy);
        if (!occupied.has(nk) || visited.has(nk)) continue;
        visited.add(nk);
        queue.push(nk);
      }
    }

    if (component.length > largestComponent.length) {
      largestComponent = component;
    }
  }

  let componentSet = new Set(largestComponent);
  let rowToX = new Map();
  for (let gridKey of componentSet) {
    let [ix, iy] = gridKey.split(",").map(Number);
    if (!rowToX.has(iy)) rowToX.set(iy, []);
    rowToX.get(iy).push(ix);
  }

  let sortedRows = Array.from(rowToX.keys()).sort((a, b) => a - b);
  let rebuilt = new Map();
  let previousSpan = null;

  for (let iy of sortedRows) {
    let xIndices = rowToX.get(iy).sort((a, b) => a - b);
    let segments = [];
    let start = xIndices[0];
    let prev = xIndices[0];

    for (let i = 1; i < xIndices.length; i++) {
      let current = xIndices[i];
      if (current - prev <= 1) {
        prev = current;
        continue;
      }
      segments.push({ left: start, right: prev });
      start = current;
      prev = current;
    }
    segments.push({ left: start, right: prev });

    let chosen = null;
    if (!previousSpan) {
      segments.sort(
        (a, b) => b.right - b.left + 1 - (a.right - a.left + 1),
      );
      chosen = segments[0];
    } else {
      let bestScore = -Infinity;
      for (let segment of segments) {
        let overlap =
          Math.min(segment.right, previousSpan.right) -
          Math.max(segment.left, previousSpan.left) +
          1;
        let center = (segment.left + segment.right) * 0.5;
        let prevCenter = (previousSpan.left + previousSpan.right) * 0.5;
        let distancePenalty = Math.abs(center - prevCenter);
        let width = segment.right - segment.left + 1;
        let score = overlap * 4 + width - distancePenalty * 1.5;

        if (score > bestScore) {
          bestScore = score;
          chosen = segment;
        }
      }
    }

    if (!chosen) {
      continue;
    }

    if (chosen.right - chosen.left + 1 < 3) {
      continue;
    }

    let left = chosen.left;
    let right = chosen.right;
    if (previousSpan) {
      left = Math.max(left, previousSpan.left - 3);
      right = Math.min(right, previousSpan.right + 3);
    }

    let accepted = [];
    for (let ix = left; ix <= right; ix++) {
      let candidatePoint = { x: ix * step, y: iy * step };
      let debug = canReachWithIKDebug(candidatePoint.x, candidatePoint.y);
      if (!debug.reachable) continue;
      if (isLowAngleMargin(debug) && previousSpan) {
        let nearLeftEdge = ix - left <= 1;
        let nearRightEdge = right - ix <= 1;
        if (nearLeftEdge || nearRightEdge) {
          continue;
        }
      }
      if (isLowAngleMargin(debug) && accepted.length < 2 && right - left >= 4) {
        continue;
      }
      accepted.push(ix);
      rebuilt.set(key(ix, iy), candidatePoint);
    }

    if (accepted.length > 0) {
      previousSpan = { left: accepted[0], right: accepted[accepted.length - 1] };
    }
  }

  let result = Array.from(rebuilt.values());
  result.sort((a, b) => (a.y === b.y ? a.x - b.x : a.y - b.y));
  return result;
}

function isStableWorkspacePoint(x, y, neighborStep) {
  let debug = canReachWithIKDebug(x, y);
  if (!debug.reachable) {
    return false;
  }

  if (
    debug.minAngleMarginDeg !== undefined &&
    debug.minAngleMarginDeg < WORKSPACE_MIN_ANGLE_MARGIN_DEG
  ) {
    return false;
  }

  return true;
}

function buildFillPath() {
  if (workspacePoints.length === 0) {
    fillPath = [];
    fillIndex = 0;
    return;
  }

  let rows = new Map();

  for (let point of workspacePoints) {
    let rowKey = Math.round(point.y);
    if (!rows.has(rowKey)) {
      rows.set(rowKey, []);
    }
    rows.get(rowKey).push({ x: point.x, y: point.y });
  }

  let sortedRows = Array.from(rows.keys()).sort((a, b) => a - b);
  let ordered = [];

  for (let i = 0; i < sortedRows.length; i++) {
    let row = rows.get(sortedRows[i]);
    row.sort((a, b) => a.x - b.x);

    if (i % 2 === 1) {
      row.reverse();
    }

    ordered.push(...row);
  }

  fillPath = ordered;
  fillIndex = 0;
}

function isPointReachableWithMargin(x, y) {
  return canReachWithIK(x, y);
}

function isCircleReachable(centerX, centerY, radius, sampleCount = 24) {
  if (radius <= 0) {
    return isPointReachableWithMargin(centerX, centerY);
  }

  for (let i = 0; i < sampleCount; i++) {
    let angle = (TWO_PI * i) / sampleCount;
    let x = centerX + cos(angle) * radius;
    let y = centerY + sin(angle) * radius;

    if (!isPointReachableWithMargin(x, y)) {
      return false;
    }
  }

  return true;
}

function findSafeCircleRadius(centerX, centerY, desiredRadius) {
  for (let radius = desiredRadius; radius >= 0; radius -= 2) {
    if (isCircleReachable(centerX, centerY, radius)) {
      return radius;
    }
  }

  return 0;
}

function findSafeCircleRadiusFromTop(topX, topY, desiredRadius) {
  for (let radius = desiredRadius; radius >= 0; radius -= 2) {
    let centerX = topX;
    let centerY = topY + radius;

    if (isCircleReachable(centerX, centerY, radius)) {
      return radius;
    }
  }

  return 0;
}

function isSquareReachable(centerX, centerY, size, step = 8) {
  let halfSize = size / 2;
  let left = centerX - halfSize;
  let right = centerX + halfSize;
  let top = centerY - halfSize;
  let bottom = centerY + halfSize;

  for (let x = left; x <= right; x += step) {
    if (!isPointReachableWithMargin(x, top)) return false;
    if (!isPointReachableWithMargin(x, bottom)) return false;
  }

  for (let y = top; y <= bottom; y += step) {
    if (!isPointReachableWithMargin(left, y)) return false;
    if (!isPointReachableWithMargin(right, y)) return false;
  }

  return (
    isPointReachableWithMargin(left, top) &&
    isPointReachableWithMargin(right, top) &&
    isPointReachableWithMargin(left, bottom) &&
    isPointReachableWithMargin(right, bottom)
  );
}

function findSafeSquareSize(centerX, centerY, desiredSize) {
  for (let size = desiredSize; size >= 0; size -= 4) {
    if (isSquareReachable(centerX, centerY, size)) {
      return size;
    }
  }

  return 0;
}

function isSquareReachableFromTopLeft(topLeftX, topLeftY, size, step = 8) {
  let right = topLeftX + size;
  let bottom = topLeftY + size;

  for (let x = topLeftX; x <= right; x += step) {
    if (!isPointReachableWithMargin(x, topLeftY)) return false;
    if (!isPointReachableWithMargin(x, bottom)) return false;
  }

  for (let y = topLeftY; y <= bottom; y += step) {
    if (!isPointReachableWithMargin(topLeftX, y)) return false;
    if (!isPointReachableWithMargin(right, y)) return false;
  }

  return (
    isPointReachableWithMargin(topLeftX, topLeftY) &&
    isPointReachableWithMargin(right, topLeftY) &&
    isPointReachableWithMargin(topLeftX, bottom) &&
    isPointReachableWithMargin(right, bottom)
  );
}

function findSafeSquareSizeFromTopLeft(topLeftX, topLeftY, desiredSize) {
  for (let size = desiredSize; size >= 0; size -= 4) {
    if (isSquareReachableFromTopLeft(topLeftX, topLeftY, size)) {
      return size;
    }
  }

  return 0;
}

function isSquareReachableFromTopRight(topRightX, topRightY, size, step = 8) {
  return isSquareReachableFromTopLeft(topRightX - size, topRightY, size, step);
}

function findSafeSquareSizeFromTopRight(topRightX, topRightY, desiredSize) {
  for (let size = desiredSize; size >= 0; size -= 4) {
    if (isSquareReachableFromTopRight(topRightX, topRightY, size)) {
      return size;
    }
  }

  return 0;
}

function getTiltedSquarePointsFromTop(topX, topY, sideLength) {
  let halfDiagonal = sideLength / Math.sqrt(2);
  return [
    { x: topX, y: topY },
    { x: topX + halfDiagonal, y: topY + halfDiagonal },
    { x: topX, y: topY + halfDiagonal * 2 },
    { x: topX - halfDiagonal, y: topY + halfDiagonal },
  ];
}

function getTiltedSquareHorizontalSpan(topX, topY, sideLength, y) {
  let points = getTiltedSquarePointsFromTop(topX, topY, sideLength);
  let intersections = [];

  for (let i = 0; i < points.length; i++) {
    let p1 = points[i];
    let p2 = points[(i + 1) % points.length];

    if (Math.abs(p1.y - p2.y) < 1e-6) {
      continue;
    }

    let minY = Math.min(p1.y, p2.y);
    let maxY = Math.max(p1.y, p2.y);
    if (y < minY || y > maxY) {
      continue;
    }

    let t = (y - p1.y) / (p2.y - p1.y);
    let x = lerp(p1.x, p2.x, t);
    intersections.push(x);
  }

  if (intersections.length < 2) {
    return null;
  }

  intersections.sort((a, b) => a - b);
  return {
    left: intersections[0],
    right: intersections[intersections.length - 1],
  };
}

function isTiltedSquareReachableFromTop(topX, topY, sideLength, step = 8) {
  let halfDiagonal = sideLength / Math.sqrt(2);
  let bottomY = topY + halfDiagonal * 2;

  for (let y = topY; y <= bottomY; y += step) {
    let span = getTiltedSquareHorizontalSpan(topX, topY, sideLength, y);
    if (!span) {
      continue;
    }

    for (let x = span.left; x <= span.right; x += step) {
      if (!isPointReachableWithMargin(x, y)) {
        return false;
      }
    }

    if (!isPointReachableWithMargin(span.right, y)) {
      return false;
    }
  }

  let points = getTiltedSquarePointsFromTop(topX, topY, sideLength);
  return points.every((point) => isPointReachableWithMargin(point.x, point.y));
}

function findSafeTiltedSquareSizeFromTop(topX, topY, desiredSize) {
  for (let size = desiredSize; size >= 0; size -= 4) {
    if (isTiltedSquareReachableFromTop(topX, topY, size)) {
      return size;
    }
  }

  return 0;
}

function findSafeDiamondSizeFromTop(topX, topY, desiredSize) {
  return findSafeTiltedSquareSizeFromTop(topX, topY, desiredSize);
}

function getWorkspaceBounds() {
  if (workspacePoints.length === 0) {
    return null;
  }

  let minX = Infinity;
  let maxX = -Infinity;
  let minY = Infinity;
  let maxY = -Infinity;

  for (let point of workspacePoints) {
    if (point.x < minX) minX = point.x;
    if (point.x > maxX) maxX = point.x;
    if (point.y < minY) minY = point.y;
    if (point.y > maxY) maxY = point.y;
  }

  return { minX, maxX, minY, maxY };
}

function getRotatedRectangleCorners(centerX, centerY, width, height, angleRad) {
  let halfW = width / 2;
  let halfH = height / 2;
  let cosA = cos(angleRad);
  let sinA = sin(angleRad);

  let localCorners = [
    { x: -halfW, y: -halfH },
    { x: halfW, y: -halfH },
    { x: halfW, y: halfH },
    { x: -halfW, y: halfH },
  ];

  return localCorners.map((corner) => ({
    x: centerX + corner.x * cosA - corner.y * sinA,
    y: centerY + corner.x * sinA + corner.y * cosA,
  }));
}

function isRotatedRectangleReachable(
  centerX,
  centerY,
  width,
  height,
  angleRad,
  sampleStep = 8,
) {
  let cosA = cos(angleRad);
  let sinA = sin(angleRad);
  let halfW = width / 2;
  let halfH = height / 2;

  for (let localY = -halfH; localY <= halfH; localY += sampleStep) {
    for (let localX = -halfW; localX <= halfW; localX += sampleStep) {
      let x = centerX + localX * cosA - localY * sinA;
      let y = centerY + localX * sinA + localY * cosA;

      if (!isPointReachableWithMargin(x, y)) {
        return false;
      }
    }

    let edgeX = centerX + halfW * cosA - localY * sinA;
    let edgeY = centerY + halfW * sinA + localY * cosA;
    if (!isPointReachableWithMargin(edgeX, edgeY)) {
      return false;
    }
  }

  let corners = getRotatedRectangleCorners(
    centerX,
    centerY,
    width,
    height,
    angleRad,
  );
  return corners.every((corner) => isPointReachableWithMargin(corner.x, corner.y));
}

// Approximate search for the largest tilted rectangle that fits in the reachable area.
// Returns { centerX, centerY, width, height, angleDeg, area, corners } or null.
function findLargestTiltedRectangleInWorkspace(options = {}) {
  if (workspacePoints.length === 0) {
    return null;
  }

  let angleDeg = options.angleDeg ?? 45;
  let angleRad = radians(angleDeg);
  let sizeStep = options.sizeStep ?? 8;
  let sampleStep = options.sampleStep ?? 8;
  let centerStride = options.centerStride ?? 3;
  let minWidth = options.minWidth ?? sizeStep;
  let minHeight = options.minHeight ?? sizeStep;
  let aspectRatio = options.aspectRatio ?? null;

  let bounds = getWorkspaceBounds();
  if (!bounds) {
    return null;
  }

  let maxWidth = options.maxWidth ?? bounds.maxX - bounds.minX;
  let maxHeight = options.maxHeight ?? bounds.maxY - bounds.minY;
  let centers = workspacePoints.filter((_, index) => index % centerStride === 0);

  let candidateSizes = [];
  if (aspectRatio && aspectRatio > 0) {
    let constrainedMaxWidth = Math.min(maxWidth, maxHeight * aspectRatio);

    for (let width = constrainedMaxWidth; width >= minWidth; width -= sizeStep) {
      let height = width / aspectRatio;
      if (height < minHeight || height > maxHeight) {
        continue;
      }

      candidateSizes.push({ width, height, area: width * height });
    }
  } else {
    for (let width = maxWidth; width >= minWidth; width -= sizeStep) {
      for (let height = maxHeight; height >= minHeight; height -= sizeStep) {
        candidateSizes.push({ width, height, area: width * height });
      }
    }
  }

  candidateSizes.sort((a, b) => b.area - a.area);

  let best = null;

  for (let candidate of candidateSizes) {
    if (best && candidate.area <= best.area) {
      break;
    }

    for (let center of centers) {
      if (
        isRotatedRectangleReachable(
          center.x,
          center.y,
          candidate.width,
          candidate.height,
          angleRad,
          sampleStep,
        )
      ) {
        best = {
          centerX: center.x,
          centerY: center.y,
          width: candidate.width,
          height: candidate.height,
          angleDeg,
          area: candidate.area,
          corners: getRotatedRectangleCorners(
            center.x,
            center.y,
            candidate.width,
            candidate.height,
            angleRad,
          ),
        };
        break;
      }
    }
  }

  return best;
}

function startFillMode() {
  buildFillPath();
  if (fillPath.length === 0) {
    console.warn("⚠ No workspace dots available for fill mode");
    return;
  }

  fillIndex = 0;
  mode = 3;
}

function getFillTarget() {
  if (fillPath.length === 0) {
    return [homePos.x, homePos.y];
  }

  fillIndex = constrain(fillIndex, 0, fillPath.length - 1);
  let target = fillPath[fillIndex];
  let fillCanAdvance =
    !arduinoBusy &&
    commandQueue.length === 0 &&
    millis() - lastCommandTime >= MIN_COMMAND_INTERVAL;

  if (
    dist(penPos.x, penPos.y, target.x, target.y) < 4 &&
    fillCanAdvance &&
    fillIndex < fillPath.length - 1
  ) {
    fillIndex++;
    target = fillPath[fillIndex];
  } else if (
    fillIndex >= fillPath.length - 1 &&
    dist(penPos.x, penPos.y, target.x, target.y) < 4 &&
    fillCanAdvance
  ) {
    manualPreviewTarget = createVector(target.x, target.y);
    mode = 6;
    return [target.x, target.y];
  }

  return [target.x, target.y];
}

function canReachWithIK(targetX, targetY) {
  return canReachWithIKDebug(targetX, targetY).reachable;
}

function canReachWithIKDebug(targetX, targetY) {
  let motor1Limits = getMotor1AngleLimits();
  let motor2Limits = getMotor2AngleLimits();
  let midTheta1 = (motor1Limits.min + motor1Limits.max) * 0.5;
  let midTheta2 = (motor2Limits.min + motor2Limits.max) * 0.5;
  let seeds = [
    { theta1: lastAngles.theta1, theta2: lastAngles.theta2, label: "last" },
    { theta1: midTheta1, theta2: midTheta2, label: "mid" },
    {
      theta1: motor1Limits.min,
      theta2: motor2Limits.min,
      label: "min/min",
    },
    {
      theta1: motor1Limits.max,
      theta2: motor2Limits.max,
      label: "max/max",
    },
    {
      theta1: motor1Limits.min,
      theta2: motor2Limits.max,
      label: "min/max",
    },
    {
      theta1: motor1Limits.max,
      theta2: motor2Limits.min,
      label: "max/min",
    },
  ];

  let sawJointSolve = false;
  let sawJointReach = false;
  let sawMotor1 = false;
  let sawMotor2 = false;

  for (let seed of seeds) {
    let solvedJoint = solveJointTargetForPen(
      targetX,
      targetY,
      seed.theta1,
      seed.theta2,
    );
    if (!solvedJoint) {
      continue;
    }

    sawJointSolve = true;

    let jointX = solvedJoint.x;
    let jointY = solvedJoint.y;

    if (!isReachable(jointX, jointY)) {
      continue;
    }

    sawJointReach = true;

    let motor1Solution = solveMotorAngle(
      motor1Pos.x,
      motor1Pos.y,
      jointX,
      jointY,
      MOTOR_ARM_LENGTH * SCALE,
      LINK_ARM_1_LENGTH * SCALE,
      seed.theta1,
      motor1Limits.min,
      motor1Limits.max,
    );

    let motor2Solution = solveMotorAngle(
      motor2Pos.x,
      motor2Pos.y,
      jointX,
      jointY,
      MOTOR_ARM_LENGTH * SCALE,
      LINK_ARM_2_LENGTH * SCALE,
      seed.theta2,
      motor2Limits.min,
      motor2Limits.max,
    );

    if (motor1Solution) sawMotor1 = true;
    if (motor2Solution) sawMotor2 = true;

    if (motor1Solution && motor2Solution) {
      let motor1MarginDeg = degrees(
        Math.min(
          Math.abs(motor1Solution.angle - motor1Limits.min),
          Math.abs(motor1Limits.max - motor1Solution.angle),
        ),
      );
      let motor2MarginDeg = degrees(
        Math.min(
          Math.abs(motor2Solution.angle - motor2Limits.min),
          Math.abs(motor2Limits.max - motor2Solution.angle),
        ),
      );

      return {
        reachable: true,
        reason: `ok (${seed.label})`,
        motor1AngleDeg: degrees(motor1Solution.angle),
        motor2AngleDeg: degrees(motor2Solution.angle),
        motor1MarginDeg,
        motor2MarginDeg,
        minAngleMarginDeg: Math.min(motor1MarginDeg, motor2MarginDeg),
      };
    }
  }

  if (!sawJointSolve) {
    return { reachable: false, reason: "pen->joint solve failed" };
  }
  if (!sawJointReach) {
    return { reachable: false, reason: "joint outside geometric reach" };
  }
  if (!sawMotor1 || !sawMotor2) {
    return { reachable: false, reason: "motor angle limits reject solution" };
  }

  return { reachable: false, reason: "branch selection failed" };
}

function solveJointTargetForPen(
  targetX,
  targetY,
  seedTheta1 = lastAngles.theta1,
  seedTheta2 = lastAngles.theta2,
) {
  let motor1Limits = getMotor1AngleLimits();
  let motor2Limits = getMotor2AngleLimits();
  const penOffsetPx = PEN_OFFSET * SCALE;

  function solveWithInitialGuess(initialJointX, initialJointY) {
    let jointX = initialJointX;
    let jointY = initialJointY;
    let preferredTheta1 = seedTheta1;
    let preferredTheta2 = seedTheta2;

    for (let i = 0; i < 8; i++) {
      if (!isReachable(jointX, jointY)) {
        return null;
      }

      let motor1Solution = solveMotorAngle(
        motor1Pos.x,
        motor1Pos.y,
        jointX,
        jointY,
        MOTOR_ARM_LENGTH * SCALE,
        LINK_ARM_1_LENGTH * SCALE,
        preferredTheta1,
        motor1Limits.min,
        motor1Limits.max,
      );

      let motor2Solution = solveMotorAngle(
        motor2Pos.x,
        motor2Pos.y,
        jointX,
        jointY,
        MOTOR_ARM_LENGTH * SCALE,
        LINK_ARM_2_LENGTH * SCALE,
        preferredTheta2,
        motor2Limits.min,
        motor2Limits.max,
      );

      if (!motor1Solution || !motor2Solution) {
        return null;
      }

      let joint2X =
        motor2Pos.x + cos(motor2Solution.angle) * MOTOR_ARM_LENGTH * SCALE;
      let joint2Y =
        motor2Pos.y + sin(motor2Solution.angle) * MOTOR_ARM_LENGTH * SCALE;
      let dx = jointX - joint2X;
      let dy = jointY - joint2Y;
      let len = Math.hypot(dx, dy);

      if (len < 1e-6) {
        return null;
      }

      dx /= len;
      dy /= len;

      preferredTheta1 = motor1Solution.angle;
      preferredTheta2 = motor2Solution.angle;

      let nextJointX = targetX - dx * penOffsetPx;
      let nextJointY = targetY - dy * penOffsetPx;
      if (dist(nextJointX, nextJointY, jointX, jointY) < 0.25) {
        jointX = nextJointX;
        jointY = nextJointY;
        break;
      }

      jointX = nextJointX;
      jointY = nextJointY;
    }

    return { x: jointX, y: jointY };
  }

  let fromMotor2Dx = targetX - motor2Pos.x;
  let fromMotor2Dy = targetY - motor2Pos.y;
  let fromMotor2Len = Math.hypot(fromMotor2Dx, fromMotor2Dy);
  if (fromMotor2Len < 1e-6) {
    fromMotor2Len = 1;
  }
  fromMotor2Dx /= fromMotor2Len;
  fromMotor2Dy /= fromMotor2Len;

  let centerX = (motor1Pos.x + motor2Pos.x) * 0.5;
  let centerY = (motor1Pos.y + motor2Pos.y) * 0.5;
  let fromCenterDx = targetX - centerX;
  let fromCenterDy = targetY - centerY;
  let fromCenterLen = Math.hypot(fromCenterDx, fromCenterDy);
  if (fromCenterLen < 1e-6) {
    fromCenterLen = 1;
  }
  fromCenterDx /= fromCenterLen;
  fromCenterDy /= fromCenterLen;

  let attempts = [
    {
      x: targetX - fromMotor2Dx * penOffsetPx,
      y: targetY - fromMotor2Dy * penOffsetPx,
    },
    {
      x: targetX - fromCenterDx * penOffsetPx,
      y: targetY - fromCenterDy * penOffsetPx,
    },
    { x: targetX, y: targetY },
  ];

  for (let attempt of attempts) {
    let solved = solveWithInitialGuess(attempt.x, attempt.y);
    if (solved) {
      return solved;
    }
  }

  return null;
}

function updatePenTipPosition(targetX, targetY) {
  if (!jointPos) {
    jointPos = createVector(targetX, targetY);
  } else {
    jointPos.set(targetX, targetY);
  }

  const penOffsetPx = PEN_OFFSET * SCALE;
  let dx = targetX - linkArm2.pivot.x;
  let dy = targetY - linkArm2.pivot.y;
  let len = Math.hypot(dx, dy);

  if (!penPos) {
    penPos = createVector(targetX, targetY);
  }

  if (len < 1e-6) {
    penPos.set(targetX, targetY);
    return;
  }

  dx /= len;
  dy /= len;
  penPos.set(targetX + dx * penOffsetPx, targetY + dy * penOffsetPx);
}

function syncLinkageToMotorAngles() {
  let joint = circleIntersection(
    motorArm1.endpoint.x,
    motorArm1.endpoint.y,
    LINK_ARM_1_LENGTH * SCALE,
    motorArm2.endpoint.x,
    motorArm2.endpoint.y,
    LINK_ARM_2_LENGTH * SCALE,
  );

  if (!joint) {
    return false;
  }

  linkArm1.pivot.set(motorArm1.endpoint.x, motorArm1.endpoint.y);
  linkArm1.setAngle(
    atan2(joint.y - motorArm1.endpoint.y, joint.x - motorArm1.endpoint.x),
  );

  linkArm2.pivot.set(motorArm2.endpoint.x, motorArm2.endpoint.y);
  linkArm2.setAngle(
    atan2(joint.y - motorArm2.endpoint.y, joint.x - motorArm2.endpoint.x),
  );

  updatePenTipPosition(joint.x, joint.y);
  return true;
}

function isReachable(targetX, targetY) {
  let d1 = dist(motor1Pos.x, motor1Pos.y, targetX, targetY);
  let d2 = dist(motor2Pos.x, motor2Pos.y, targetX, targetY);

  const minReach1 = Math.abs(MOTOR_ARM_LENGTH - LINK_ARM_1_LENGTH) * SCALE;
  const maxReach1 = (MOTOR_ARM_LENGTH + LINK_ARM_1_LENGTH) * SCALE;
  const minReach2 = Math.abs(MOTOR_ARM_LENGTH - LINK_ARM_2_LENGTH) * SCALE;
  const maxReach2 = (MOTOR_ARM_LENGTH + LINK_ARM_2_LENGTH) * SCALE;

  let reachable1 =
    d1 >= minReach1 * INNER_REACH_MARGIN &&
    d1 <= maxReach1 * OUTER_REACH_MARGIN;
  let reachable2 =
    d2 >= minReach2 * INNER_REACH_MARGIN &&
    d2 <= maxReach2 * OUTER_REACH_MARGIN;

  return reachable1 && reachable2;
}

function calculateIK(targetX, targetY) {
  let motor1Limits = getMotor1AngleLimits();
  let motor2Limits = getMotor2AngleLimits();
  let motor1Solutions = solveMotorAngle(
    motor1Pos.x,
    motor1Pos.y,
    targetX,
    targetY,
    MOTOR_ARM_LENGTH * SCALE,
    LINK_ARM_1_LENGTH * SCALE,
    lastAngles.theta1,
    motor1Limits.min,
    motor1Limits.max,
  );

  let motor2Solutions = solveMotorAngle(
    motor2Pos.x,
    motor2Pos.y,
    targetX,
    targetY,
    MOTOR_ARM_LENGTH * SCALE,
    LINK_ARM_2_LENGTH * SCALE,
    lastAngles.theta2,
    motor2Limits.min,
    motor2Limits.max,
  );

  if (!motor1Solutions || !motor2Solutions) {
    return false;
  }

  motorArm1.setAngle(motor1Solutions.angle);
  motorArm2.setAngle(motor2Solutions.angle);

  linkArm1.pivot.set(motorArm1.endpoint.x, motorArm1.endpoint.y);
  let linkAngle1 = atan2(
    targetY - motorArm1.endpoint.y,
    targetX - motorArm1.endpoint.x,
  );
  linkArm1.setAngle(linkAngle1);

  linkArm2.pivot.set(motorArm2.endpoint.x, motorArm2.endpoint.y);
  let linkAngle2 = atan2(
    targetY - motorArm2.endpoint.y,
    targetX - motorArm2.endpoint.x,
  );
  linkArm2.setAngle(linkAngle2);

  updatePenTipPosition(targetX, targetY);

  lastAngles.theta1 = motor1Solutions.angle;
  lastAngles.theta2 = motor2Solutions.angle;

  return true;
}

function fitAngleToRange(rawAngle, preferredAngle, minAngle, maxAngle) {
  let candidates = [rawAngle, rawAngle - TWO_PI, rawAngle + TWO_PI];
  let best = null;
  let nearMinLimit = Math.abs(preferredAngle - minAngle) <= IK_ANGLE_EPSILON;
  let nearMaxLimit = Math.abs(preferredAngle - maxAngle) <= IK_ANGLE_EPSILON;

  for (let candidate of candidates) {
    let finalAngle = null;

    if (candidate >= minAngle && candidate <= maxAngle) {
      finalAngle = candidate;
    } else if (
      candidate < minAngle &&
      candidate >= minAngle - IK_ANGLE_EPSILON &&
      nearMinLimit
    ) {
      finalAngle = minAngle;
    } else if (
      candidate > maxAngle &&
      candidate <= maxAngle + IK_ANGLE_EPSILON &&
      nearMaxLimit
    ) {
      finalAngle = maxAngle;
    }

    if (finalAngle === null) continue;

    let diff = Math.abs(finalAngle - preferredAngle);
    if (!best || diff < best.diff) {
      best = { angle: finalAngle, diff };
    }
  }

  return best;
}

function solveMotorAngle(
  motorX,
  motorY,
  targetX,
  targetY,
  motorArmLen,
  linkArmLen,
  preferredAngle,
  minAngle,
  maxAngle,
) {
  let distToTarget = dist(motorX, motorY, targetX, targetY);

  if (
    distToTarget > motorArmLen + linkArmLen ||
    distToTarget < Math.abs(motorArmLen - linkArmLen)
  ) {
    return null;
  }

  let d = distToTarget;
  let r1 = motorArmLen;
  let r2 = linkArmLen;
  let a = (r1 * r1 - r2 * r2 + d * d) / (2 * d);
  let h = sqrt(r1 * r1 - a * a);
  let cx = motorX + (a * (targetX - motorX)) / d;
  let cy = motorY + (a * (targetY - motorY)) / d;

  let intersect1 = {
    x: cx + (h * (targetY - motorY)) / d,
    y: cy - (h * (targetX - motorX)) / d,
  };

  let intersect2 = {
    x: cx - (h * (targetY - motorY)) / d,
    y: cy + (h * (targetX - motorX)) / d,
  };

  let angle1 = atan2(intersect1.y - motorY, intersect1.x - motorX);
  let angle2 = atan2(intersect2.y - motorY, intersect2.x - motorX);

  let validSolutions = [];
  let solution1 = fitAngleToRange(angle1, preferredAngle, minAngle, maxAngle);
  let solution2 = fitAngleToRange(angle2, preferredAngle, minAngle, maxAngle);

  if (solution1) validSolutions.push(solution1);
  if (solution2) validSolutions.push(solution2);
  if (validSolutions.length === 0) return null;

  validSolutions.sort((a, b) => a.diff - b.diff);
  return { angle: validSolutions[0].angle };
}

function findNearestReachablePoint(targetX, targetY) {
  let minDist = Infinity;
  let nearest = null;

  let candidates = workspacePoints.length > 0 ? workspacePoints : workspacePolygon;

  for (let p of candidates) {
    let d = dist(p.x, p.y, targetX, targetY);
    if (d < minDist) {
      minDist = d;
      nearest = p;
    }
  }

  return nearest;
}
