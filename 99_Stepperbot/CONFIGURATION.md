# 5-Bar Linkage Configuration Guide

## Physical Dimensions

### Measuring Your Robot

Before configuring the software, measure these critical dimensions:

```
        [L]         
    O-------●       O = Motor pivot (base)
    |        \      ● = End effector
    |         \     [L] = armLength
   [B]         \    [B] = baseLength  
    |           \
    O------------●
    
    |<---B---->|
```

**armLength** (`segArm.len` in code)
- Measure from the motor shaft center to the end effector joint
- Both arms should have the same length
- Typical range: 100-150mm for desktop robots

**baseLength** (`segBase.len` in code)  
- Measure the horizontal distance between the two motor shaft centers
- Affects the workspace width
- Typical range: 40-80mm

### Update in sketch.js

```javascript
let armLength = 75;   // YOUR MEASUREMENT (e.g., 75mm)
let baseLength = 25;   // YOUR MEASUREMENT (e.g., 25mm)
```

**Example with actual measurements:**
- Arm length from motor shaft to joint: **75mm**
- Distance between motor shafts: **25mm**

## Workspace Calculation

The reachable workspace is an intersection of two circles:

### Maximum Reach
The furthest point the end effector can reach:

```
r_max = 2 × armLength - baseLength
```

Example with armLength=75mm and baseLength=25mm:
```
r_max = 2 × 75 - 25 = 125mm
```

### Minimum Reach  
The closest point to the base:

```
r_min = baseLength
```

Example with baseLength=25mm:
```
r_min = 25mm
```

### Optimal Working Area

The best drawing quality occurs in the center region where:
- Both arms are roughly 45-135° from horizontal
- The arms don't fully extend or collapse
- Mechanical advantage is good

**Recommended workspace**: 60-80% of maximum reach

```
r_optimal_min = r_min + 0.2 × (r_max - r_min)
r_optimal_max = r_min + 0.8 × (r_max - r_min)
```

Example workspace with armLength=75mm, baseLength=25mm:
```
r_optimal_min = 25 + 0.2 × (125 - 25) = 45mm
r_optimal_max = 25 + 0.8 × (125 - 25) = 105mm
```

**Your robot's workspace:**
- **Maximum reach:** 125mm from center
- **Minimum reach:** 25mm from center
- **Optimal drawing area:** 45-105mm from center (recommended for best accuracy)

## Angle Limits

You can set angle limits in [segment.js](js/segment.js) to prevent mechanical collisions:

```javascript
this.maxDeg = 180;  // Maximum angle in degrees
this.minDeg = 0;    // Minimum angle in degrees
```

These are checked in the `checkLimits()` function.

## Stepper Motor Configuration

### Steps Per Revolution

Calculate total steps for one complete rotation:

```
steps_per_rev = MOTOR_STEPS × MICROSTEPS × GEAR_RATIO
```

Examples:
- **Direct drive, 1/4 microstepping**: `200 × 4 × 1 = 800 steps/rev`
- **1:3 gear ratio, 1/8 microstepping**: `200 × 8 × 3 = 4800 steps/rev`

### Steps Per Degree

```
steps_per_degree = steps_per_rev / 360
```

Example with 1/4 microstepping:
```
steps_per_degree = 800 / 360 = 2.222 steps/degree
```

### Update in Arduino Code

Edit [99_Stepperbot.ino](99_Stepperbot.ino):

```cpp
#define MOTOR_STEPS 200      // Steps per revolution (typically 200)
#define MICROSTEPS 4         // Match your driver jumper settings!
#define GEAR_RATIO 1.0       // Direct = 1.0, geared = ratio
#define ANGLE_TO_STEPS true  // Uncomment to enable angle conversion
```

## Serial Communication

### Command Format

The p5.js sketch sends commands in this format:

```
X<angle>,Y<angle>\n
```

Examples:
- `X45,Y90\n` - Move left motor to 45°, right motor to 90°
- `X0,Y0\n` - Move both to zero position
- `E\n` - Toggle enable/disable

### Current Behavior vs Angle Mode

**Default (ANGLE_TO_STEPS undefined):**
- Numbers are treated as raw step positions
- `X50,Y100` = move 50 steps left, 100 steps right

**With ANGLE_TO_STEPS enabled:**
- Numbers are treated as degrees
- `X50,Y100` = move to 50° left, 100° right
- Automatically converted using `stepsPerDegree`

## Calibration Procedure

### 1. Zero Position

Manually position both arms at 0° (horizontal) and upload Arduino code. This sets the reference point.

### 2. Test Range of Motion

Send test commands via Serial Monitor:

```
X90,Y90   - Should move to 90° if ANGLE_TO_STEPS enabled
X45,Y45   - Move to 45°
X0,Y0     - Return to zero
```

### 3. Check Workspace

In the web interface:
- Mode 0 (mouse): Move mouse around expected workspace
- Mode 1 (square): Should draw a square path
- Mode 2 (circle): Should draw a circular path

If movements don't match expectations:
- Verify `armLength` and `baseLength` measurements
- Check `MICROSTEPS` matches driver jumpers
- Enable `ANGLE_TO_STEPS` if using degree-based control

### 4. Fine-Tuning

Adjust the `offset` parameter in [arm.js](js/arm.js#L11):

```javascript
this.offset = 0;  // Add/subtract degrees to calibrate zero position
```

This compensates for mechanical installation offsets.

## Common Issues

### Arms Don't Reach Expected Positions
- ❌ Incorrect `armLength` or `baseLength`
- ✅ Re-measure and update configuration

### Jittery Movement
- ❌ Microstepping mismatch
- ✅ Check Arduino `MICROSTEPS` matches driver jumpers
- ✅ Reduce `RPM` in Arduino code

### Workspace Too Small/Large
- ❌ Wrong units (px vs mm)
- ✅ Keep units consistent between hardware measurements and code
- ✅ Scale canvas or adjust measurements

### Serial Connection Fails
- ❌ Wrong port in `sketch.js`
- ✅ Use `serial.list()` to find correct port
- ✅ Check p5.serial server is running

### Motors Move Wrong Direction  
- ❌ Wiring or angle calculation issue
- ✅ Swap motor wires or change sign in `moveServo()`:
  ```javascript
  let cmd = `X${-int(degrees(angle_1))},Y${int(degrees(angle_2))}\n`
  ```

## Advanced Configuration

### Acceleration Profile

Enable smooth acceleration in Arduino code:

```cpp
// In setup()
stepperL.setSpeedProfile(stepperL.LINEAR_SPEED, 500, 500);
stepperR.setSpeedProfile(stepperR.LINEAR_SPEED, 500, 500);
```

Parameters: `(profile_type, acceleration, deceleration)` in steps/sec²

### Custom Drawing Paths

Add new patterns in [shapes.js](js/shapes.js). Each shape returns `[x, y]` coordinates:

```javascript
class CustomPath {
  constructor(x, y, size) {
    this.x = x;
    this.y = y;
    this.size = size;
    this.t = 0;
  }
  
  draw() {
    // Your path calculation
    let x = this.x + this.size * cos(this.t);
    let y = this.y + this.size * sin(this.t);
    return [x, y];
  }
  
  update() {
    this.t += 0.05;
  }
}
```

Then add to sketch.js setup and draw functions.

## References

- [Five-bar linkage (Wikipedia)](https://en.wikipedia.org/wiki/Five-bar_linkage)
- [Inverse Kinematics Tutorial](https://www.alanzucconi.com/2017/04/10/robotic-arms/)
- [A4988 Stepper Driver Guide](https://www.pololu.com/product/1182)
