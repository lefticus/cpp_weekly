#include <math.h>   // smallpt, a Path Tracer by Kevin Beason, 2008
#include <stdio.h>  //        Remove "-fopenmp" for g++ version < 4.2
#include <stdlib.h> // Make : g++ -O3 -fopenmp smallpt.cpp -o smallpt
#include <array>
#include <memory>

/*
  // Tested for fun with corroutine for double generation
  // Add too much complexity without allowing to add any const
  // +8.7% execution time (on total for samps = 128)
  // (based on cppreference example)

class DoubleGenerator {
public:
    struct promise_type {
        ... (same impl. from cppref example)
        double current_value;
    };
  
    ... (same impl. from cppref example)
    double next() {
        m_coroutine.resume();
        return m_coroutine.promise().current_value;
    }
 
private:
    std::coroutine_handle<promise_type> m_coroutine;
};
 
DoubleGenerator randGenerator(unsigned short Xi3)
{
  unsigned short Xi[3] {0,0,Xi3};
  while(true)
  {
    co_yield erand48(&Xi[0]);
  }
}
*/


struct Vec {        // Usage: time ./smallpt 5000 && xv image.ppm
  const double x, y, z;   // position, also color (r,g,b)
  constexpr Vec(const double x_ = 0, const double y_ = 0, const double z_ = 0):x(x_),y(y_),z(z_){}
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
  const Vec o, d;
  constexpr Ray(const Vec& o_, const Vec& d_) : o(o_), d(d_) {}
};

enum Refl_t { DIFF, SPEC, REFR }; // material types, used in radiance()

struct Sphere {
  const double rad;  // radius
  const Vec p, e, c; // position, emission, color
  const Refl_t refl; // reflection type (DIFFuse, SPECular, REFRactive)
  constexpr Sphere(const double rad_,const Vec& p_, const Vec& e_, const Vec& c_, const Refl_t& refl_): rad(rad_), p(p_), e(e_), c(c_), refl(refl_) {}
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

inline double clamp(const double x) { return x < 0 ? 0 : x > 1 ? 1 : x; }
inline int toInt(const double x) { return int(pow(clamp(x), 1 / 2.2) * 255 + .5); }
inline bool intersect(const Ray &r, double &t, int &id) {
  constexpr double n = spheres.size(), inf = 1e20;
  t = inf;
  for (int i = int(n); i--;)
  {
    const double d = spheres[i].intersect(r);
    if (d && d < t) {
      t = d;
      id = i;
    }
  }
  return t < inf;
}

const Vec radiance(const Ray &r, int depth, unsigned short *Xi) {
  double t;   // distance to intersection
  int id = 0; // id of intersected object
  
  if (!intersect(r, t, id))
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
    if (erand48(Xi) >= p)
    {
      return obj.e;       // R.R.
    }
  }
  const Vec f = (depth > 5) ? f0 * (1 / p) : f0;
  
  if (obj.refl == DIFF) { // Ideal DIFFUSE reflection
    const double r1 = 2 * M_PI * erand48(Xi);
    const double r2 = erand48(Xi);
    const double r2s = sqrt(r2);
    const Vec w = nl;
    const Vec u = ((fabs(w.x) > .1 ? Vec(0, 1) : Vec(1)) % w).norm();
    const Vec v = w % u;
    const Vec d = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).norm();
    return obj.e + f.mult(radiance(Ray(x, d), depth, Xi));
  }
  else if (obj.refl == SPEC) // Ideal SPECULAR reflection
  {
    return obj.e + f.mult(radiance(Ray(x, r.d - n * 2 * n.dot(r.d)), depth, Xi));
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
    return obj.e + f.mult(radiance(reflRay, depth, Xi));
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
                    ? (erand48(Xi) < P ? // Russian roulette
                           radiance(reflRay, depth, Xi) * RP
                                       : radiance(Ray(x, tdir), depth, Xi) * TP)
                    : radiance(reflRay, depth, Xi) * Re +
                          radiance(Ray(x, tdir), depth, Xi) * Tr);
}

const Vec rCompute(const int x, const int y, const int sx, const int sy, const int samps, unsigned short* Xi, int s=0)
{
  constexpr int w = 1024, h = 768;
  constexpr Ray cam {Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm()}; // cam pos, dir
  constexpr Vec cx {w * .5135 / h};
  constexpr Vec cy {(cx % cam.d).norm() * .5135};
  const double r1 = 2 * erand48(Xi);
  const double dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
  const double r2 = 2 * erand48(Xi);
  const double dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
  const Vec d = cx * (((sx + .5 + dx) / 2 + x) / w - .5) +
                cy * (((sy + .5 + dy) / 2 + y) / h - .5) + cam.d;
  /********************************************
  * Original version radiance(Ray(cam.o + d * 140, d.norm()), 0, Xi)
  * implies that d.norm() modify d internal state in order to be normed
  * then cam.o + d * 140 is builded... with d normalized
  * was it a deature or a bug? For same behavior, had to normalize before
  * PS: probably another bug of such kind exist but i don't (have the will to) find it
  * (because the generator is deterministic and then the images should be exactly the sames for same samps
  * at least they converge (between original version and the one from this version))
  *********************************************/
  const Vec dNorm = d.norm(); 
  return (radiance(Ray(cam.o + dNorm * 140, dNorm), 0, Xi) * (1. / samps) + ((s<samps)?rCompute(x,y,sx,sy,samps,Xi,s+1):Vec()));
}


int main(const int argc, const char *argv[])
{
  constexpr int w = 1024, h = 768;
  const int samps{argc == 2 ? atoi(argv[1]) / 4 : 1}; // # samples
  
  std::array<std::unique_ptr<const Vec>, w * h> c;

#pragma omp parallel for schedule(dynamic, 1) // OpenMP
  for (int y = 0 ; y < h; y++ ) {                          // Loop over image rows
    fprintf(stderr, "\rRendering (%d spp) %5.2f%%", samps * 4, 100. * y / (h - 1));
    for (unsigned short x = 0, Xi[3] = {0, 0, static_cast<unsigned short>(y * y * y)};
         x < w;
         x++) // Loop cols
    {
      const Vec r00{rCompute(x,y,0,0,samps,Xi)};
      const Vec r01{rCompute(x,y,0,1,samps,Xi)};
      const Vec r10{rCompute(x,y,1,0,samps,Xi)};
      const Vec r11{rCompute(x,y,1,1,samps,Xi)};
      // Camera rays are pushed ^^^^^ forward to start in interior
      const int i = (h - y - 1) * w + x;
      c[i] = std::unique_ptr<const Vec>{new Vec((Vec(clamp(r00.x), clamp(r00.y), clamp(r00.z))
            + Vec(clamp(r01.x), clamp(r01.y), clamp(r01.z))
            + Vec(clamp(r10.x), clamp(r10.y), clamp(r10.z))
            + Vec(clamp(r11.x), clamp(r11.y), clamp(r11.z)))
                      * .25)};
    }
  }
  FILE *f = fopen("image2.ppm", "w"); // Write image to PPM file.
  fprintf(f, "P3\n%d %d\n%d\n", w, h, 255);
  for (int i = 0; i < w * h; i++)
    fprintf(f, "%d %d %d ", toInt(c[i]->x), toInt(c[i]->y), toInt(c[i]->z));
}
