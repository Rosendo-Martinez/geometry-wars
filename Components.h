#include "Vec2.h"
#include <SFML/Graphics.hpp>

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

    CCollision(float r)
        : radius(r) {}
};

class CScore
{
public:
    int score = 0;

    CScore(int s)
        : score(s) {}
};

class CShape
{
public:
    sf::CircleShape circle;

    CShape(float radius, int points, const sf::Color & fill, const sf::Color & outline, float thickness)
        : circle(radius, points) 
    {
        circle.setFillColor(fill);
        circle.setOutlineColor(outline);
        circle.setOutlineThickness(thickness);
        circle.setOrigin(radius, radius);
    }
};

class CLifespan
{
public:
    int remaining = 0; // remaining lifespan
    int total = 0; // initial amount of lifespan

    CLifespan(int total)
        : remaining(total), total(total) {}
};

class CInput
{
public:
    bool up = false;
    bool left = false;
    bool right = false;
    bool down = false;
    bool shoot = false;

    CInput() {}
};
