#ifndef GEOMETRY_H
#define GEOMETRY_H

#include<cmath>

class Point
{
    public:
        float x;
        float y;

        bool operator== (Point b);
		float distance(Point b);
};
#endif