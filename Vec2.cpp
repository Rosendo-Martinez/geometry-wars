#include <cmath>
#include "Vec2.h"

Vec2::Vec2(double x, double y)
    : x(x)
    , y(y)
{
}

Vec2::Vec2()
{
}

bool Vec2::operator == (const Vec2& rhs) const
{
    return x == rhs.x && y == rhs.y;
}

bool Vec2::operator != (const Vec2& rhs) const
{
    return x != rhs.x || y == rhs.y;
}

Vec2 Vec2::operator + (const Vec2& rhs) const
{
    return Vec2(x + rhs.x, y + rhs.y);
}

Vec2 Vec2::operator * (const Vec2& rhs) const
{
    return Vec2(x * rhs.x, y * rhs.y);
}

Vec2 Vec2::operator / (const Vec2& rhs) const
{
    return Vec2(x / rhs.x, y / rhs.y);
}

void Vec2::normalize()
{
    double l = length();
    x = x/l;
    y = y/l;
}

double Vec2::length()
{
    return std::sqrt(x*x + y*y);
}