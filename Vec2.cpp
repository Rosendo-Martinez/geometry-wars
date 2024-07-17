#include <cmath>
#include "Vec2.h"

Vec2::Vec2(float x, float y)
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

Vec2 Vec2::operator - (const Vec2& rhs) const
{
    return Vec2(x - rhs.x, y - rhs.y);
}

Vec2 Vec2::operator * (const Vec2& rhs) const
{
    return Vec2(x * rhs.x, y * rhs.y);
}

Vec2 Vec2::operator / (const Vec2& rhs) const
{
    return Vec2(x / rhs.x, y / rhs.y);
}

Vec2 Vec2::operator * (const float val) const
{
    return Vec2(x*val, y*val);
}

void Vec2::operator += (const Vec2& rhs)
{
    x += rhs.x;
    y += rhs.y;
}

void Vec2::operator -= (const Vec2& rhs)
{
    x -= rhs.x;
    y -= rhs.y;
}

void Vec2::operator *= (const float val)
{
    x *= val;
    y *= val;
}

void Vec2::operator /= (const float val)
{
    x /= val;
    y /= val;
}

void Vec2::normalize()
{
    float l = length();
    x = x/l;
    y = y/l;
}

float Vec2::length() const
{
    return std::sqrt(x*x + y*y);
}

float Vec2::dist(const Vec2& v) const
{
    return std::sqrt((x - v.x) * (x - v.x) + (y - v.y) * (y - v.y));
}

float Vec2::distSqr(const Vec2& v) const
{
    return (x - v.x) * (x - v.x) + (y - v.y) * (y - v.y);
}