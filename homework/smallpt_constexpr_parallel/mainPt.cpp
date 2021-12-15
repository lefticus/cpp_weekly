#include <math.h>   // smallpt, a Path Tracer by Kevin Beason, 2008
#include <stdio.h>  //        Remove "-fopenmp" for g++ version < 4.2
#include <array>
#include "smallpt_parallel.h"

template<int... Is>
struct seq {};
template<int I, int... Is>
struct gen_seq : gen_seq<I-1, I, Is...> {};
template<int... Is>
struct gen_seq<0, Is...> : seq<0, Is...> {};

template<int... Is>
constexpr auto gatherPointForSequence(seq<Is...>) -> std::array<std::array<int,3>, WIDTH * HEIGHT>
{
  return {{ {point<Is>::x, point<Is>::y, point<Is>::z}... }};
}

template<int N>
void gatherPoints(FILE* f)
{
  const auto points = gatherPointForSequence(gen_seq<N-1>{});
  for (int h=HEIGHT-1; h>=0; --h)
  {
    for(int w=0; w<WIDTH; w++)
    {
      auto p = points[h*WIDTH + w];
      fprintf(f, "%d %d %d ", p[0], p[1], p[2]);
    }
  }
}

int main(const int , const char *[])
{
  FILE * const f = fopen("image2.ppm", "w"); // Write image to PPM file.
  fprintf(f, "P3\n%d %d\n%d\n", WIDTH, HEIGHT, 255);
  gatherPoints<WIDTH * HEIGHT>(f);
}
