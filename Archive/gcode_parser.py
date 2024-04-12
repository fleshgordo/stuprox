import sys
import re

# Gcode number parser
# Reads a gcode file and rounds the floating numbers (coordinates) to two decimal numbers

def format_gcode(input_file):
    # Regular expression pattern to match floating-point numbers
    float_pattern = r'[-+]?\d*\.\d+'
    
    with open(input_file, 'r') as f:
        lines = f.readlines()

    with open(input_file, 'w') as f:
        for line in lines:
            # Find all floating-point numbers in the line
            floats = re.findall(float_pattern, line)
            # Format each floating-point number to have two decimal places
            formatted_line = re.sub(float_pattern, lambda x: '{:.2f}'.format(float(x.group())), line)
            # Write the formatted line to the output file
            f.write(formatted_line)

# Check if the script is called with the input filename as a command-line argument
if len(sys.argv) != 2:
    print("Usage: python script.py <input_filename>")
    sys.exit(1)

input_file = sys.argv[1]
format_gcode(input_file)
