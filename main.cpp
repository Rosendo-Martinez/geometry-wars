#include <iostream>
#include "Vec2.h"

int main() 
{
    Vec2 v1(4,2);
    Vec2 v2(1,2);

    std::cout << v1.length() << "\n";
    
    v1.normalize();

    std::cout << v1.x << " " << v1.y << "\n";
}