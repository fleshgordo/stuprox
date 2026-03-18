void PrintValue(float value, const int minWidth)
{
  // Initialize variables
  int numDigits = 2;  // For the 2 decimal places
  int numSpaces = 0;
  float senseSpaces = value;
  
  // Handle negative values
  if (value < 0)
  {
    numSpaces--;  // Adjust for negative sign
    senseSpaces = -value;  // Work with the positive value for space calculation
  }
  
  // Handle the special case for 0.0
  if (value == 0.0)
  {
    senseSpaces = 1.0;  // To avoid log(0) and handle zero
  }
  
  // Calculate the number of digits in the integer part
  // log10 gives the exponent of the number in scientific notation, which corresponds to the number of digits - 1
  int numIntDigits = (int)log10(senseSpaces) + 1;
  
  // Calculate total spaces needed
  numSpaces += minWidth - numIntDigits - numDigits - 1;  // Subtract digits and the decimal point
  
  // Print leading spaces for alignment
  for (int i = 0; i < numSpaces; i++)
  {
    Serial.print(" ");
  }
  
  // Print the float with 2 decimal places
  Serial.print(value, 2);
  //Serial.println();  // Move to the next line
}

void PrintPoint(Point pt)
{
  PrintPoint(pt,true);
}

void PrintPoint(Point pt, bool fullInfo)  // Accepts by value, no pointer needed
{
  if (fullInfo) Serial.print("x: ");
  PrintValue(pt.x,7);
  //Serial.print(pt.x, 2);
  if (fullInfo)
  {
    Serial.print(", y: ");
  } else {
    Serial.print(" ");
  }
  PrintValue(pt.y,7);
  Serial.println();
  //Serial.println(pt.y, 2);
}

void PrintPtArray(Point* ptrPts, const char* name)
{
  // Print the name of the array
  Serial.print(name);
  Serial.println(" :");  // Prints the name passed as argument
  
  // Print all points in the array
  for (int i = 0; i < NUMPTS; i++) {
    Serial.print(i);  // Print the index of the point
    Serial.print(": ");
    PrintPoint(ptrPts[i],false);  // Call PrintPoint for each element in the array
  }
}

