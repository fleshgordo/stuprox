const colors = require('tailwindcss/colors');

module.exports = {
  content: ['./app/**/*.{js,ts,jsx,tsx}'],
  theme: {
    fontFamily: {
      sans: 'Satoshi, sans-serif',
      mono: 'Space Grotesk, monospace',
    },
    colors: {
      transparent: 'transparent',
      current: 'currentColor',
      black: colors.black,
      white: colors.white,
      gray: colors.neutral,
      primary: colors.neutral,
    },
  },
  plugins: [require('@tailwindcss/forms')],
};
