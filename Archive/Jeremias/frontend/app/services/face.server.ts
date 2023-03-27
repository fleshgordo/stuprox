import sharp from 'sharp';

export default async function (input: string) {
  const uri = input.split(';base64,').pop();
  let imgBuffer = Buffer.from(uri!, 'base64');
  const {
    channels,
    dominant: { r, g, b },
  } = await sharp(imgBuffer).stats();

  return {
    r,
    g,
    b,
    channels,
  };
}
