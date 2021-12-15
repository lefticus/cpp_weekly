#pragma once

#include <math.h>   // smallpt, a Path Tracer by Kevin Beason, 2008
//#include <stdio.h>  //        Remove "-fopenmp" for g++ version < 4.2
#include <stdlib.h> // Make : g++ -O3 -fopenmp smallpt.cpp -o smallpt
#include <array>
#include <memory>
#include <numeric>
#include <cstdint>
#include <limits>

struct PCG
{
  struct pcg32_random_t { std::uint64_t state=2791;  std::uint64_t inc=123; };
  pcg32_random_t rng;
  typedef std::uint32_t result_type;

  constexpr result_type operator()()
  {
    return pcg32_random_r();
  }

  static result_type constexpr min()
  {
    return std::numeric_limits<result_type>::min();
  }

  static result_type constexpr max()
  {
    return std::numeric_limits<result_type>::min();
  }

  private:
  constexpr std::uint32_t pcg32_random_r()
  {
    std::uint64_t oldstate = rng.state;
    // Advance internal state
    rng.state = oldstate * 6364136223846793005ULL + (rng.inc|1);
    // Calculate output function (XSH RR), uses old state for max ILP
    std::uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    std::uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
  }

};

constexpr double get_random(int count)
{
  PCG pcg;
  while(count > 0){
    pcg();
    --count;
  }
  return static_cast<double>(pcg())/std::numeric_limits<std::uint32_t>::max();
}

namespace Detail
{
    double constexpr sqrtNewtonRaphson(double x, double curr, double prev)
    {
        return curr == prev
            ? curr
            : sqrtNewtonRaphson(x, 0.5 * (curr + x / curr), curr);
    }
}

/*
* Constexpr version of the square root
* Return value:
*   - For a finite and non-negative value of "x", returns an approximation for the square root of "x"
*   - Otherwise, returns NaN
*/
double constexpr sqrt(double x)
{
    return x >= 0 && x < std::numeric_limits<double>::infinity()
        ? Detail::sqrtNewtonRaphson(x, x, 0)
        : std::numeric_limits<double>::quiet_NaN();
}


struct Vec {        // Usage: time ./smallpt 5000 && xv image.ppm
  double x;
  double y;
  double z;   // position, also color (r,g,b)
  constexpr Vec(const double x_ = 0, const double y_ = 0, const double z_ = 0):x(x_),y(y_),z(z_){}
  constexpr Vec(const Vec& o) = default;
  constexpr Vec(Vec&& o) = default;
  constexpr Vec operator+(const Vec &b) const { return Vec(x + b.x, y + b.y, z + b.z); }
  constexpr Vec operator-(const Vec &b) const { return Vec(x - b.x, y - b.y, z - b.z); }
  constexpr Vec operator*(const double b) const { return Vec(x * b, y * b, z * b); }
  constexpr Vec mult(const Vec &b) const { return Vec(x * b.x, y * b.y, z * b.z); }
  constexpr Vec norm() const { return Vec(operator* (1/sqrt(x*x+y*y+z*z))); } 
  constexpr double dot(const Vec &b) const {
    return x * b.x + y * b.y + z * b.z;
  } // cross:
  constexpr Vec operator%(const Vec &b) const {
    return Vec(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);
  }
};

struct Ray {
  Vec o, d;
  constexpr Ray(const Vec& o_, const Vec& d_) : o(o_), d(d_) {}
  constexpr Ray(const Ray& other) = default;
  constexpr Ray(Ray&& other) = default;
};

enum Refl_t { DIFF, SPEC, REFR }; // material types, used in radiance()

struct Sphere {
  double rad;  // radius
  Vec p, e, c; // position, emission, color
  Refl_t refl; // reflection type (DIFFuse, SPECular, REFRactive)
  constexpr Sphere(const double rad_,const Vec& p_, const Vec& e_, const Vec& c_, const Refl_t& refl_): rad(rad_), p(p_), e(e_), c(c_), refl(refl_) {}
  constexpr Sphere(const Sphere& other) = default;
  constexpr Sphere(Sphere&& other) = default;
  constexpr double intersect(const Ray &r) const { // returns distance, 0 if nohit
    const Vec op = p - r.o; // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
    constexpr double eps = 1e-4;
    const double b = op.dot(r.d);
    const double det = b * b - op.dot(op) + rad * rad;
    if (det < 0)
      return 0;
    const double det2 = sqrt(det);
    const double t1 = b - det2;
    if (t1 > eps)
      return t1;
    const double t2 = b + det2;
    if (t2 > eps)
      return t2;
    return 0;
  }
};

constexpr std::array<const Sphere,9> spheres = {
    // Scene: radius, position, emission, color, material
    Sphere(1e5, Vec(1e5 + 1, 40.8, 81.6), Vec(), Vec(.75, .25, .25), DIFF), // Left
    Sphere(1e5, Vec(-1e5 + 99, 40.8, 81.6), Vec(), Vec(.25, .25, .75), DIFF),                                                     // Rght
    Sphere(1e5, Vec(50, 40.8, 1e5), Vec(), Vec(.75, .75, .75), DIFF), // Back
    Sphere(1e5, Vec(50, 40.8, -1e5 + 170), Vec(), Vec(), DIFF),       // Frnt
    Sphere(1e5, Vec(50, 1e5, 81.6), Vec(), Vec(.75, .75, .75), DIFF), // Botm
    Sphere(1e5, Vec(50, -1e5 + 81.6, 81.6), Vec(), Vec(.75, .75, .75), DIFF),                                                      // Top
    Sphere(16.5, Vec(27, 16.5, 47), Vec(), Vec(1, 1, 1) * .999, SPEC), // Mirr
    Sphere(16.5, Vec(73, 16.5, 78), Vec(), Vec(1, 1, 1) * .999, REFR), // Glas
    Sphere(600, Vec(50, 681.6 - .27, 81.6), Vec(12, 12, 12), Vec(), DIFF) // Lite
};

constexpr inline double clamp(const double x) { return x < 0 ? 0 : x > 1 ? 1 : x; }
constexpr inline int toInt(const double x) { return int(pow(clamp(x), 1 / 2.2) * 255 + .5); }

constexpr Vec radiance(const Ray &r, int depth, const std::uint64_t Xi) {
  const double inf = 1e20;
  double t = inf;  // distance to intersection
  int id = 0; // id of intersected object
  for (int i = int(spheres.size()); i--;)
  {
    const double d = spheres[i].intersect(r);
    if (d && d < t) {
      t = d;
      id = i;
    }
  }
  if (t >= inf)
  {
    return Vec();                  // if miss, return black
  }
  
  const Sphere &obj = spheres[id]; // the hit object
  
  const Vec x = r.o + r.d * t;
  const Vec n = (x - obj.p).norm();
  const Vec nl = n.dot(r.d) < 0 ? n : n * -1;
  const Vec f0 = obj.c;

  const double p = f0.x > f0.y && f0.x > f0.z ? f0.x : f0.y > f0.z ? f0.y : f0.z; // max refl
  if (++depth > 5)
  {
    if (get_random(Xi) >= p)
    {
      return obj.e;       // R.R.
    }
  }
  const Vec f = (depth > 5) ? f0 * (1 / p) : f0;
  
  if (obj.refl == DIFF) { // Ideal DIFFUSE reflection
    const double r1 = 2 * M_PI * get_random(Xi+1);
    const double r2 = get_random(Xi+2);
    const double r2s = sqrt(r2);
    const Vec w = nl;
    const Vec u = ((fabs(w.x) > .1 ? Vec(0, 1) : Vec(1)) % w).norm();
    const Vec v = w % u;
    const Vec d = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).norm();
    return obj.e + f.mult(radiance(Ray(x, d), depth,Xi+3));
  }
  else if (obj.refl == SPEC) // Ideal SPECULAR reflection
  {
    return obj.e + f.mult(radiance(Ray(x, r.d - n * 2 * n.dot(r.d)), depth, Xi+1));
  }
  const Ray reflRay(x, r.d - n * 2 * n.dot(r.d)); // Ideal dielectric REFRACTION
  const bool into = n.dot(nl) > 0;                // Ray from outside going in?
  const double nc = 1;
  const double nt = 1.5;
  const double nnt = into ? nc / nt : nt / nc;
  const double ddn = r.d.dot(nl);
  const double cos2t = 1 - nnt * nnt * (1 - ddn * ddn);
  if (cos2t < 0) // Total internal reflection
  {
    return obj.e + f.mult(radiance(reflRay, depth, Xi+1));
  }
  const Vec tdir = (r.d * nnt - n * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).norm();
  const double a = nt - nc;
  const double b = nt + nc;
  const double R0 = a * a / (b * b);
  const double c = 1 - (into ? -ddn : tdir.dot(n));
  const double Re = R0 + (1 - R0) * c * c * c * c * c;
  const double Tr = 1 - Re;
  const double P = .25 + .5 * Re;
  const double RP = Re / P;
  const double TP = Tr / (1 - P);
  return obj.e +
         f.mult(depth > 2
                    ? (get_random(Xi+1) < P ? // Russian roulette
                           radiance(reflRay, depth, Xi+2) * RP
                                       : radiance(Ray(x, tdir), depth, Xi+2) * TP)
                    : radiance(reflRay, depth, Xi+2) * Re +
                          radiance(Ray(x, tdir), depth, Xi+2) * Tr);
}

constexpr Vec rCompute(const int x, const int y, const int sx, const int sy, const uint64_t Xi, int s=0)
{
  Ray cam {Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm()}; // cam pos, dir
  Vec cx {WIDTH * .5135 / HEIGHT};
  Vec cy {(cx % cam.d).norm() * .5135};
  double r1 = 2 * get_random(Xi);
  double dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
  double r2 = 2 * get_random(Xi+1);
  double dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
  Vec d = cx * (((sx + .5 + dx) / 2 + x) / WIDTH - .5) +
                    cy * (((sy + .5 + dy) / 2 + y) / HEIGHT - .5) + cam.d;
  Vec dNorm = d.norm();
  return ( radiance(Ray(cam.o + dNorm * 140, dNorm), 0, Xi+2) * (1. / SAMPS) + ((s<SAMPS)?rCompute(x,y,sx,sy,Xi+2, s+1):Vec()));
}

constexpr Vec calculatePoint(std::uint64_t N) {
  Vec r00{rCompute(N%WIDTH,N/HEIGHT,0,0,{N})};
  Vec r01{rCompute(N%WIDTH,N/HEIGHT,0,1,{N+7})};
  Vec r10{rCompute(N%WIDTH,N/HEIGHT,1,0,{N+11})};
  Vec r11{rCompute(N%WIDTH,N/HEIGHT,1,1,{N+17})};
  // Camera rays are pushed ^^^^^ forward to start in interior
  return Vec((Vec(clamp(r00.x), clamp(r00.y), clamp(r00.z))
        + Vec(clamp(r01.x), clamp(r01.y), clamp(r01.z))
        + Vec(clamp(r10.x), clamp(r10.y), clamp(r10.z))
        + Vec(clamp(r11.x), clamp(r11.y), clamp(r11.z)))
                  * .25);
}

template <int p>
struct point {
    static const int x;
    static const int y;
    static const int z;
};

