#define EXPORT
#include "smallpt_parallel.h"

template <> const int point<H*WIDTH+W>::x{toInt(calculatePoint(H*WIDTH+W).x)};
template <> const int point<H*WIDTH+W>::y{toInt(calculatePoint(H*WIDTH+W).y)};
template <> const int point<H*WIDTH+W>::z{toInt(calculatePoint(H*WIDTH+W).z)};
