#ifndef PERLIN_H
#define PERLIN_H

class Perlin {
public:
    Perlin(unsigned int seed);
    double noise(double x, double y, double z);
private:
    double fade(double t);
    double lerp(double t, double a, double b);
    double grad(int hash, double x, double y, double z);
    int p[512];
};

#endif // PERLIN_H