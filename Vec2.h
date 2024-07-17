#pragma once

class Vec2
{
public:
    float x = 0;
    float y = 0;

    bool operator == (const Vec2& rhs) const;
    bool operator != (const Vec2& rhs) const;

    Vec2 operator +  (const Vec2& rhs) const;
    Vec2 operator -  (const Vec2& rhs) const;
    Vec2 operator *  (const Vec2& rhs) const;
    Vec2 operator /  (const Vec2& rhs) const;
    Vec2 operator * (const float val) const;

    void operator += (const Vec2& rhs);
    void operator -= (const Vec2& rhs);
    void operator *= (const float val);
    void operator /= (const float val);

    Vec2();
    Vec2(float x, float y);

    void   normalize ();
    float  length    () const;
    float  dist      (const Vec2& v) const;
    float  distSqr   (const Vec2& v) const;
};