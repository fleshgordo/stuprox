void InitPt(Point* ptrPts, int min, int max)
{
    // Initialize the first point (optional, but good to set a known value)
    ptrPts[0] = {0.0, 0.0}; 

    // Initialize all the other points
    for (int i = 1; i < NUMPTS; i++) {
        ptrPts[i].x = random(min, max);  // Random value for x
        ptrPts[i].y = random(min, max);  // Random value for y
    }
}

Point AddPoints(Point ptA, Point ptB)
{
  Point ptReturn = {ptA.x + ptB.x, ptA.y + ptB.y};
  return ptReturn;
}
Point SubtractPoints(Point ptA, Point ptB)
{
  Point ptReturn = {ptA.x-ptB.x, ptA.y - ptB.y};
  return ptReturn;
}