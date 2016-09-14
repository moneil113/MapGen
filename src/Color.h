typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} Color;

float clamp01(float a);

Color ColorFromFloats(float r, float g, float b);

Color ColorLerp(Color a, Color b, float t);
