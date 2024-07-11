#pragma once

class Vec2
{
public:
    double x = 0;
    double y = 0;

    bool operator == (const Vec2& rhs) const;
    bool operator != (const Vec2& rhs) const;
    Vec2 operator +  (const Vec2& rhs) const;
    Vec2 operator *  (const Vec2& rhs) const;
    Vec2 operator /  (const Vec2& rhs) const;

    Vec2();
    Vec2(double x, double y);

    void   normalize();
    double length   ();
};