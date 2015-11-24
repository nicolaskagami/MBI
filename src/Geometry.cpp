
#include "Geometry.h"

bool Point::operator== (Point b)
{ 
    if((x == b.x)&&(y == b.y))
        return true; 
    else 
        return false;
}
float Point::distance(Point b)
{//Manhattan Distance
	return fabs((float)x-b.x) + fabs((float)y-b.y);
}



