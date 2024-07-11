#include "Vec2.h"

class CTransform
{
public:
    Vec2   pos   = {0.0, 0.0};
    Vec2   speed = {0.0, 0.0};
    Vec2   scale = {0, 0};
    double angle = 0;

    CTransform(Vec2 p, Vec2 sp, Vec2 sc, double a)
        : pos(p)
        , speed(sp)
        , scale(sc)
        , angle(a)
    {}
};

class CCollision
{
public:
    float radius = 0;

    CCollision(float r): radius(r) {}
};

class CScore
{
public:
    int score = 0;

    CScore(int s): score(s) {}
};
