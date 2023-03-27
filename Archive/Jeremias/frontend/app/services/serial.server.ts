import { SerialPort } from 'serialport';
import { ReadlineParser } from '@serialport/parser-readline';

const config = {
  path: '/dev/tty.usbmodem101',
  baudRate: 115200,
  autoOpen: false,
};

let index = 0;
let distortionVal = 0;
let commands: string[] = [];
const packetSize = 70;

/** REMIX Specific init (only establish serial connection once)*/
let port: SerialPort;
declare global {
  var __port: SerialPort | undefined;
}
if (!global.__port) {
  global.__port = new SerialPort(config);
  global.__port.pipe(new ReadlineParser());
  global.__port.on('open', () => {
    console.log('PORT OPENED');
  });
}
port = global.__port;
/** END REMIX Specific init */

port.on('data', (data) => {
  console.log(data.toString());
  if (data.toString() === 'XX') {
    index += packetSize;
    run(commands);
  }
});

export const openPort = () => {
  if (!port.isOpen) {
    port.open();
  }
};

let lastStepsL: number[] = [];
let lastStepsR: number[] = [];

const MICROSTEPS = 16;
const MOTOR_STEPS = 200;

const ARM_LENGTH = 130.5; //130.5 for pen holder arm
const MAX_X = 105;
const MAX_Y = 250; //245
const OFFSET_1 = 27.5;
const OFFSET_2 = 77.5;

const { PI, atan, acos, sqrt, floor } = Math;

const degreesToSteps = (deg: number) => {
  const factor = deg / 360;
  return (
    floor(MICROSTEPS * MOTOR_STEPS * factor) - (MICROSTEPS * MOTOR_STEPS) / 2
  );
};

const getDistorted = () => {
  return Math.floor(
    Math.random() * (distortionVal - -distortionVal + 1) + -distortionVal
  );
};

const cartesianToSteps = ({ x, y }: { x: number; y: number }) => {
  let l = 0;
  let r = 0;
  const randX = getDistorted();
  const randY = getDistorted();
  x = Math.min(x + randX, 100);
  y = Math.min(y + randY, 143);

  let angleL;
  let angleR;

  const distanceL = sqrt((OFFSET_1 - x) ** 2 + (MAX_Y - y) ** 2);
  const distanceR = sqrt((OFFSET_2 - x) ** 2 + (MAX_Y - y) ** 2);

  if (x > OFFSET_1) {
    angleL =
      PI +
      acos(Math.min(1, distanceL / (2 * ARM_LENGTH))) -
      atan((x - OFFSET_1) / (MAX_Y - y));
  } else {
    angleL =
      PI +
      acos(Math.min(1, distanceL / (2 * ARM_LENGTH))) +
      atan((OFFSET_1 - x) / (MAX_Y - y));
  }

  if (x > OFFSET_2) {
    angleR =
      PI -
      acos(Math.min(1, distanceR / (2 * ARM_LENGTH))) -
      atan((x - OFFSET_2) / (MAX_Y - y));
  } else {
    angleR =
      PI -
      acos(Math.min(1, distanceR / (2 * ARM_LENGTH))) +
      atan((OFFSET_2 - x) / (MAX_Y - y));
  }

  l =
    degreesToSteps(angleL * (180 / PI)) -
    lastStepsL.reduce((prev, cur) => (prev += cur), 0);
  r =
    degreesToSteps(angleR * (180 / PI)) -
    lastStepsR.reduce((prev, cur) => (prev += cur), 0);

  lastStepsL.push(l);
  lastStepsR.push(r);
  return { l, r };
};

const run = (runCommands: string[]) => {
  const commandString = runCommands.slice(index, index + packetSize).join(',');

  if (commandString) {
    port.write(`${commandString}\n`, (err) => {
      if (err) {
        console.error(err.message);
      }
      console.log('message written, ');
    });
  } else {
    return;
  }
};

export default async function sendCommand(
  coords: { x: number; y: number }[],
  distortion?: number
) {
  index = 0;
  distortionVal = distortion ?? 0;
  commands = [];
  for (const coord of coords) {
    const { l, r } = cartesianToSteps(coord);
    const command = `${l}_${r}`;
    commands.push(command);
  }
  run(commands);
}
