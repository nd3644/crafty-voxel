#ifndef PERLIN_H
#define PERLIN_H

#include <iostream>
#include <vector>
#include <cmath>
#include <random>

#include <iostream>
#include <cmath>
#include <random>

template <typename T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
    return std::min(std::max(v, lo), hi);
}

// Perlin noise function
double noise(double x, double y, int seed) {
    int n = int(x + y * 57 + seed * 131);
    n = (n << 13) ^ n;
    return (1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

// Function to generate a 2D height map
void generateHeightMap(int width, int height, int seed, int octaves, double persistence, double scale, int* heightMap) {
    // Loop over all pixels in the height map
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double amplitude = 1.0;
            double frequency = 1.0;
            double noiseHeight = 0.0;

            // Generate multiple octaves of Perlin noise and combine them
            for (int i = 0; i < octaves; i++) {
                double sampleX = x / scale * frequency;
                double sampleY = y / scale * frequency;
                double perlinValue = noise(sampleX, sampleY, seed) * 2 - 1;
                noiseHeight += perlinValue * amplitude;

                amplitude *= persistence;
                frequency *= 2;
            }

            // Scale the height value to the desired range
            double scaledHeight = (noiseHeight + 1) / 2 * 128.0;

            // Store the integer height value in the height map
            heightMap[y * width + x] = int(scaledHeight);
        }
    }
}

#endif
