#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "open-simplex-noise.h"

#include "Color.h"
#include "Queue.h"

#include <stdio.h>
#include <float.h>
#include <string.h>

float randFloat(float min, float max) {
    float value = rand() / (float) RAND_MAX;
    return min + value * (max - min);
}

/*
 * Generates a height map of floats in the range [0, 1]
 */
float **generateNoise(int seed, int width, int height, float scale, int octaves, float lacunarity, float persistence) {
    float **heightMap = (float **)malloc(width * sizeof(float *));

    float octaveOffsets[2 * octaves];
    float amplitude;
    float frequency;

    float minHeight = FLT_MAX;
    float maxHeight = FLT_MIN;

    srand(seed);

    for (int i = 0; i < octaves; i++) {
        octaveOffsets[i * 2] = randFloat(-100, 100);
        octaveOffsets[i * 2 + 1] = randFloat(-100, 100);
    }

    for (int i = 0; i < width; i++) {
        heightMap[i] = (float *)malloc(height * sizeof(float));
    }

    struct osn_context *context;
    open_simplex_noise(seed, &context);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float value = 0;
            amplitude = 1;
            frequency = 1;
            for (int i = 0; i < octaves; i++) {
                float sampleX = x * frequency / scale + octaveOffsets[2 * i];
                float sampleY = y * frequency / scale + octaveOffsets[2 * i + 1];
                float distortion = 0;//open_simplex_noise3(context, sampleX, sampleY, 0) * amplitude;
                value += fabs(open_simplex_noise3(context, sampleX + distortion, sampleY + distortion, 0) * amplitude);

                amplitude *= persistence;
                frequency *= lacunarity;
            }
            heightMap[x][y] = value;

            minHeight = value < minHeight ? value : minHeight;
            maxHeight = value > maxHeight ? value : maxHeight;
        }
    }
    open_simplex_noise_free(context);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            heightMap[x][y] = (heightMap[x][y] - minHeight) / (maxHeight - minHeight);
        }
    }

    return heightMap;
}

float evaluateFalloff(int width, int height, int x, int y) {
    float midX = width / 2.0;
    float midY = height / 2.0;
    float farRange = (midX + midY) / 2;
    farRange *= farRange;

    float dist2 = (x - midX) * (x - midX) + (y - midY) * (y - midY);

    return dist2 / farRange;
}

float **squareMap(int width, int height) {
    float **map = malloc(width * sizeof(float *));
    for (int i = 0; i < width; i++) {
        map[i] = malloc(height * sizeof(float));
    }

    int offset = width / 4;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (x < offset || y < offset || x > width - offset || y > width - offset) {
                map[x][y] = 0;
            }
            else {
                map[x][y] = 1;
            }
        }
    }

    return map;
}

void colorIslands(float **heightMap, Color *pixels, int width, int height) {
    int islandNumber = 0;
    int largestIsland = 0;
    int largestSize = 0;
    int currentSize;

    Color colorChoices[10];

    colorChoices[0] = ColorFromFloats(0, 0, 0);
    colorChoices[1] = ColorFromFloats(1, 0, 0);
    colorChoices[2] = ColorFromFloats(0, 1, 0);
    colorChoices[3] = ColorFromFloats(0, 0, 1);
    colorChoices[4] = ColorFromFloats(1, 1, 0);
    colorChoices[5] = ColorFromFloats(1, 0, 1);
    colorChoices[6] = ColorFromFloats(0, 1, 1);
    colorChoices[7] = ColorFromFloats(0.5, 0, 0);
    colorChoices[8] = ColorFromFloats(0, 0.5, 0);
    colorChoices[9] = ColorFromFloats(0, 0.5, 0);

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            // just hit a new island
            if (heightMap[x][y] == -1) {
                Queue *q = createQueue(2048);
                enqueue(q, (Point) {x, y});
                islandNumber++;
                currentSize = 0;
                heightMap[x][y] = -2;

                while (hasNext(q)) {
                    Point p = dequeue(q);
                    if (heightMap[p.x][p.y] == -2) {
                        heightMap[p.x][p.y] = islandNumber;
                        currentSize++;

                        for (int j = p.y - 1; j <= p.y + 1; j++) {
                            for (int i = p.x - 1; i <= p.x + 1; i++) {
                                if (heightMap[i][j] == -1) {
                                    int success = enqueue(q, (Point) {i, j});
                                    if (!success) {
                                        exit(1);
                                    }
                                    heightMap[i][j] = -2;
                                }
                            }
                        }
                    }
                }
                freeQueue(q);
                if (currentSize > largestSize) {
                    largestSize = currentSize;
                    largestIsland = islandNumber;
                }
            }
        }
    }

    printf("Largest island is island %d with area of %d\n", largestIsland, largestSize);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (heightMap[x][y] != 0) {
                pixels[y * width + x] = colorChoices[(int) heightMap[x][y]];
            }
            else {
                pixels[y * width + x] = (Color) {255, 255, 255};
            }
        }
    }
}

int pixelIsBorder(float **heightMap, int width, int height, int x, int y) {
    int border = 0;
    int pixelType = (heightMap[x][y] != 0);

    for (int j = y - 1; j <= y + 1; j++) {
        for (int i = x - 1; i <= x + 1; i++) {
            int otherPixel = heightMap[i][j] != 0;

            if (pixelType != otherPixel) {
                border = 1;
            }
        }
    }

    return border;
}

void outlineIslands(float **heightMap, Color *pixels, int width, int height) {
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {

            if (/*heightMap[x][y] != 0 && */pixelIsBorder(heightMap, width, height, x, y)) {
                pixels[y * width + x] = (Color) {0, 0, 0};
            }
        }
    }
}

float **loadTexture(char *fileName) {
    int width, height, n;
    unsigned char *data = stbi_load(fileName, &width, &height, &n, 4);

    float **texture = malloc(width * sizeof(float *));
    for (int i = 0; i < width; i++) {
        texture[i] = malloc(height * sizeof(float));
    }

    int i = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            texture[x][y] = data[i + 3] / 255.0;
            i += 4;
        }
    }

    stbi_image_free(data);

    return texture;
}

void freeTexture(float **texture, int width, int height) {
    for (int i = 0; i < width; i++) {
        free(texture[i]);
    }
    free(texture);
}

void drawTexture(Color *pixels, float **texture, int width, int height, int texWidth, int texHeight, int x, int y) {
    for (int j = 0; j < texHeight; j++) {
        for (int i = 0; i < texWidth; i++) {
            if (x + i < width && y + j < height && texture[i][j] != 0) {
                pixels[(y + j) * height + x + i] = (Color) {0, 0, 0};
            }
        }
    }
}

void drawMountains(float **heightMap, Color *pixels, int width, int height) {
    float **mountain = loadTexture("Mountain.png");

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (heightMap[x][y] > 0.4 && randFloat(0, 1) < 0.0005) {
                drawTexture(pixels, mountain, width, height, 32, 32, x, y);
                // drawPeak(pixels, mountain, width, height, x, y);
            }
        }
    }

    freeTexture(mountain, 32, 32);
}

void fillBackground(Color *pixels, int width, int height) {
    int w, h, n;

    unsigned char *data = stbi_load("Paper.png", &w, &h, &n, 4);

    int i = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pixels[y * width + x] = (Color) {data[i], data[i + 1], data[i + 2]};
            i += 4;
        }
    }

    stbi_image_free(data);
}

int waveFits(float **heightMap, int width, int height, int x, int y) {
    if (heightMap[x][y] != 0) {
        return 0;
    }
    if (x + 64 >= width || y + 32 >= height) {
        return 0;
    }
    if (heightMap[x + 64][y] != 0) {
        return 0;
    }
    if (heightMap[x][y + 32] != 0) {
        return 0;
    }
    if (heightMap[x + 64][y + 32] != 0) {
        return 0;
    }

    return 1;
}

void drawWaves(float **heightMap, Color *pixels, int width, int height) {
    float **wave = loadTexture("Waves.png");

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (waveFits(heightMap, width, height, x, y) && randFloat(0, 1) < 0.000025) {
                drawTexture(pixels, wave, width, height, 64, 32, x, y);
            }
        }
    }

    freeTexture(wave, 64, 32);
}

void drawHeightMap(float **heightMap, Color *pixels, int width, int height, int stage) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float val = heightMap[x][y];
            pixels[y * width + x] = ColorFromFloats(val, val, val);
        }
    }

    char *fileName;

    if (stage == 1) {
        fileName = "heightMap.png";
    }
    else if (stage == 2) {
        fileName = "falloffMap.png";
    }

    stbi_write_png(fileName, width, height, 3, pixels, width * sizeof(Color));
}

int main(int argc, char const *argv[]) {
    int width = 1024;
    int height = 1024;
    int seed = 10;
    float scale = 512;
    int octaves = 6;
    float lacunarity = 2;
    float persistence = 0.5;

    float **heightMap;

    if (argc > 1 && strcmp(argv[1], "-c") == 0) {
        // printf("Size (larger sizes will take longer): ");
        // scanf(" %d", &width);
        // height = width;

        printf("Seed (integer, default is 10): ");
        scanf(" %d", &seed);

        printf("Scale (float, default is 512.0): ");
        scanf(" %f", &scale);

        printf("Octaves (integer, default is 6): ");
        scanf(" %d", &octaves);

        printf("Lacunarity (float, default is 2.0): ");
        scanf(" %f", &lacunarity);

        printf("Persistence (float, default is 0.5): ");
        scanf(" %f", &persistence);
    }

    Color *pixels = (Color *)malloc(width * height * sizeof(Color));

    heightMap = generateNoise(seed, width, height, scale, octaves, lacunarity, persistence);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float value = heightMap[x][y] - evaluateFalloff(width, height, x, y);
            if (value < 0.2) {
                value = 0;
            }
            // Need this to color the islands
            /*else {
                value = -1;
            }*/
            heightMap[x][y] = value;
        }
    }

    fillBackground(pixels, width, height);
    outlineIslands(heightMap, pixels, width, height);
    drawMountains(heightMap, pixels, width, height);
    drawWaves(heightMap, pixels, width, height);

    char fileName[20];

    sprintf(fileName, "map%d.png", seed);

    stbi_write_png(fileName, width, height, 3, pixels, width * sizeof(Color));

    for (int i = 0; i < width; i++) {
        free(heightMap[i]);
    }
    free(heightMap);

    free(pixels);

    return 0;
}
