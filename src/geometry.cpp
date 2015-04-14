#include "geometry.hpp"

float anglediff(float a1, float a2) {
    float diff = a1 - a2;
    if (diff > M_PI) {
        diff -= 2*M_PI;
    } else if (diff < -M_PI) {
        diff += 2*M_PI;
    }
    return diff;
}
