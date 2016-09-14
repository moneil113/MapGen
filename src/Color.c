#include "Color.h"

#include <stdlib.h>
#include <math.h>

float clamp01(float a) {
    if (a < 0) {
        a = 0;
    }
    else if (a > 1) {
        a = 1;
    }

    return a;
}

Color ColorFromFloats(float r, float g, float b) {
    Color color;

    color.r = clamp01(r) * 255;
    color.g = clamp01(g) * 255;
    color.b = clamp01(b) * 255;

    return color;
}

Color ColorLerp(Color a, Color b, float t) {
    float red = ((b.r - a.r) * t + a.r) / (float) 255;
    float green = ((b.g - a.g) * t + a.g) / (float) 255;
    float blue = ((b.b - a.b) * t + a.b) / (float) 255;

    return ColorFromFloats(red, green, blue);
}
