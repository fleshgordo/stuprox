let img; // Variable to hold the image

function preload() {
  // Load the image
  img = loadImage('bunny32.png');
}

function setup() {
  // Create a canvas
  createCanvas(32, 32);

  // Display the image
  image(img, 0, 0);

  // Get grayscale values for every pixel
  let grayscaleValues = getGrayscaleValues(img);


  let output = "byte grayscaleValues[] = {";
  let output_hex = "const byte largeArray[ARRAY_SIZE] PROGMEM = {";

  for (let i = 0; i < grayscaleValues.length; i++) {
    output_hex += "0x0" + grayscaleValues[i].toString(16);
    output += grayscaleValues[i];
    if (i < grayscaleValues.length - 1) {
      output += ",";
      output_hex += ",";
    }
  }
  output += "};";
  output_hex += "};";
  console.log(output);
  console.log(output_hex);
  console.log(`grayscale value length: ${grayscaleValues.length}`);
}

function getGrayscaleValues(img) {
  // Create an empty array to store grayscale values
  let grayscaleValues = [];

  // Load pixels of the image
  img.loadPixels();

  // Loop through each pixel of the image
  for (let y = 0; y < img.height; y++) {
    for (let x = 0; x < img.width; x++) {
      // Get the index of the pixel in the pixel array
      let index = (x + y * img.width) * 4;

      // Extract RGB values from the pixel
      let r = img.pixels[index];
      let g = img.pixels[index + 1];
      let b = img.pixels[index + 2];

      // Calculate grayscale value using luminance formula
      let grayscale = (r * 0.2126 + g * 0.7152 + b * 0.0722);
      let white_or_black = grayscale > 128 ? 1 : 0;
      // Add grayscale value to the array
      grayscaleValues.push(white_or_black);
    }
  }

  // Return the array of grayscale values
  return grayscaleValues;
}