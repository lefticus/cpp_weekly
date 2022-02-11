// smallpt, a Path Tracer by Kevin Beason, 2008
// Remove "-fopenmp" for g++ version < 4.2
// Make : g++ -O3 -fopenmp smallpt.cpp -o smallpt
// Usage: time ./smallpt 5000 && xv image.ppm
// modernized by Dvir Yitzchaki dvirtz@gmail.com
#include <cmath>   
#include <concepts>
#include <tuple>
#include <vector>
#include <fmt/ostream.h>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <random>
#include <iostream>
#include <numeric>
#include <execution>
#include <future>
#include <range/v3/all.hpp>

//// concepts ////

/// VecLike is either Vec or anything with a tuple_size of 3 and all elements being convertible to double
struct Vec;

template <typename T>
concept VecLike = std::same_as<T, Vec> || requires(const T& t) {
    requires (std::tuple_size<T>() == 3);
    { std::get<0>(t) } -> concepts::convertible_to<double>;
    { std::get<1>(t) } -> concepts::convertible_to<double>;
    { std::get<2>(t) } -> concepts::convertible_to<double>;
};

static_assert(VecLike<std::tuple<double, double, double>>);
static_assert(VecLike<std::array<double, 3>>);
static_assert(not VecLike<std::vector<double>>);

/// RayLike is either Ray or anything with a tuple_size of 2 and all elements being VecLike
struct Ray;

template <typename T>
concept RayLike = std::same_as<T, Ray> || requires(const T& t) {
    requires (std::tuple_size<T>() == 3);
    { std::get<0>(t) } -> VecLike;
    { std::get<1>(t) } -> VecLike;
};

template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

//// constexpr alternatives ////

/// a constexpr PRNG to replace erand48
template<uint64_t a, uint64_t c, uint64_t m>
class linear_congruential_engine {
public:
    constexpr linear_congruential_engine(const uint64_t seed = from_time()) : state_{seed}
    {}

    constexpr double operator()() {
        state_ = (a * state_ + c) % m;
        return static_cast<double>(state_) / m;
    }

    constexpr static uint64_t from_time() {
        constexpr auto& time = __TIME__;
        return std::accumulate(std::begin(time), std::end(time) - 1, uint64_t{}, [](const auto current, const auto ch){
            return (current << 8) | static_cast<uint64_t>(ch);
        });
    }

private:
    uint64_t state_;
};

using rand48 = linear_congruential_engine<0x5DEECE66DULL, 0xBULL, 0x1000000000000ULL>;

/// a constexpr square root
constexpr auto csqrt(const arithmetic auto d)
{
    if (std::is_constant_evaluated())
    {
        const auto t = std::is_floating_point_v<std::remove_cvref_t<decltype(d)>> ? d : static_cast<double>(d);
#if 0
        // GCC 11.1 ICE on this code
        namespace rv = ranges::views;
        auto approxes = rv::generate([t, next = t / 2 + 1]() mutable{
            const auto x = next;
            next = std::midpoint(x, t / x);
            return std::make_pair(x, next);
        }) | rv::drop_while([](const auto &p){
            return p.first != p.second;
        });
        return (*ranges::begin(approxes)).first;
#else
        auto next = t / 2 + 1, x = next;
        do {
            x = next;
            next = std::midpoint(x, t / x);
        } while (x != next);
        return x;
#endif
    }
    else
    {
        return std::sqrt(d);
    }
}

//// range algorithms /////

/// rangified reduction
template<ranges::range R>
constexpr auto reduce(R&& r) {
    if (std::is_constant_evaluated()) {
        /// range-v3 is not constexpr friendly :-(
        auto c = ranges::views::common(std::forward<R>(r));
        return std::reduce(c.begin(), c.end());
    } else {
        return ranges::accumulate(std::forward<R>(r), ranges::range_value_t<R>{});
    }
}

/// transforms r by f (possibly in parallel) and join the resulting ranges in reverse order
template<ranges::range R,
          concepts::copy_constructible F>
constexpr auto transform_reverse_join(R&& r, F&& f)
    requires ranges::regular_invocable<F&, ranges::range_reference_t<R>> 
        && ranges::range<std::invoke_result_t<F&, ranges::range_reference_t<R>>> 
{
    using T = ranges::range_value_t<std::invoke_result_t<F&, ranges::range_reference_t<R>>>;

    // use parallel STL if it exists and we're not in a constexpr context
#if __cpp_lib_parallel_algorithm
    if (not std::is_constant_evaluated()) {
#else
    if constexpr (false) {
#endif
        constexpr auto policy = []{
#if __cpp_lib_parallel_algorithm
    return std::execution::par;
#else
    return 0;
#endif
        }();
        namespace rv = ranges::views;
        // use parallel STL
        const auto v = ranges::to<std::vector>(r);
        return std::transform_reduce(policy,
            v.begin(), v.end(), std::vector<T>{}, [](const auto& lhs, const auto& rhs) {
                return rv::concat(rhs, lhs) | ranges::to<std::vector>;
        }, std::forward<F>(f));
    } else {
        // range-v3 is not constexpr friendly :-(
        const auto c = ranges::views::common(std::forward<R>(r));
        const auto v = [&]{
            if constexpr(std::same_as<typename std::iterator_traits<ranges::iterator_t<decltype(c)>>::iterator_category, std::random_access_iterator_tag>) {
                return c;
            } else {
                return std::vector<ranges::range_value_t<decltype(c)>>{c.begin(), c.end()};
            }
        }();
        return std::transform_reduce(v.begin(), v.end(), std::vector<T>{}, [](const auto& lhs, const auto& rhs) {
            std::vector<T> res;
            res.reserve(ranges::size(lhs) + ranges::size(rhs));
            res.insert(res.end(), rhs.begin(), rhs.end());
            res.insert(res.end(), lhs.begin(), lhs.end());
            return res;
        }, std::forward<F>(f));
    }
}

//// constatnts /////

constexpr auto inf = 1e20, eps = 1e-4;

enum Refl_t { DIFF, SPEC, REFR };  // material types, used in radiance()

//// utility functions ////
inline constexpr auto clamp(const arithmetic auto x) { return std::clamp<decltype(x)>(x, 0, 1); }
inline constexpr auto toInt(const arithmetic auto x) {
    return int(pow(clamp(x), 1 / 2.2) * 255 + .5);
}

/// check a VecLike range equal to il
template<typename T>
constexpr bool test_result(const ranges::range auto& r, std::initializer_list<T> il)
    requires VecLike<ranges::range_value_t<decltype(r)>> 
        && concepts::equality_comparable_with<std::invoke_result_t<decltype([](const arithmetic auto x) { return toInt(x); }), double>, T> 
{
        namespace rv = ranges::views;
    auto ints = std::forward<decltype(r)>(r) | rv::transform([](const VecLike auto& v){
        const auto& [x, y, z] = v;
        return rv::concat(rv::single(toInt(x)), rv::single(toInt(y)), rv::single(toInt(z)));
    }) | rv::join;
    return ranges::equal(ints, il);
}

//// data structures ////
struct Vec {
    double x = 0, y = 0, z = 0;  // position, also color (r,g,b)
    constexpr auto operator+(const VecLike auto &b) const {
        const auto& [bx, by, bz] = b;
        return Vec{x + bx, y + by, z + bz};
    }
    constexpr auto operator-(const VecLike auto &b) const {
        const auto& [bx, by, bz] = b;
        return Vec{x - bx, y - by, z - bz};
    }
    constexpr auto operator*(const arithmetic auto b) const { return Vec{x * b, y * b, z * b}; }
    constexpr auto mult(const VecLike auto &b) const {
        const auto& [bx, by, bz] = b;
        return Vec{x * bx, y * by, z * bz};
    }
    constexpr auto norm() const {
        return *this * (1 / csqrt(x * x + y * y + z * z)); 
    }
    constexpr auto dot(const VecLike auto &b) const {
        const auto& [bx, by, bz] = b;
        return x * bx + y * by + z * bz;
    }  // cross:
    constexpr auto operator%(const VecLike auto &b) const {
        const auto& [bx, by, bz] = b;
        return Vec{y * bz - z * by, z * bx - x * bz, x * by - y * bx};
    }
};

static_assert(VecLike<Vec>);

/// teach fmt how to print Vec
template <> struct fmt::formatter<Vec> {
  constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
    const auto it = ctx.begin();
    if (it != ctx.end() && *it != '}')
      throw fmt::format_error("Vec doesn't support format flags");

    return it;
  }

  template <typename FormatContext>
  auto format(const Vec &v, FormatContext &ctx) -> decltype(ctx.out())
  {
      return fmt::format_to(
          ctx.out(),
          "{} {} {}",
          toInt(v.x), toInt(v.y), toInt(v.z));
  }
};

struct Ray {
    Vec o, d;
};

static_assert(RayLike<Ray>);

struct Sphere {
    double rad;   // radius
    Vec p, e, c;  // position, emission, color
    Refl_t refl;  // reflection type (DIFFuse, SPECular, REFRactive)
    constexpr Sphere(const double rad_, const Vec p_, const Vec e_, const Vec c_, const Refl_t refl_)
        : rad(rad_), p(p_), e(e_), c(c_), refl(refl_) {}
    // no spheres are copied in this process
    Sphere(const Sphere&) = delete;
    Sphere& operator=(const Sphere&) = delete;
    // returns distance, 0 if no hit
    constexpr auto intersect(const RayLike auto &r) const {
        const auto& [o, d] = r;
        // Solve t^2*d.d + 2*t*(p-o).d + (p-o).(p-o)-R^2 = 0
        const auto op = p - o;  
        const auto b = op.dot(d);
        const auto det = [&]{
            const auto res = b * b - op.dot(op) + rad * rad;
            return (res > 0) ? csqrt(res) : res;
        }();
        if (det < 0)
            return inf;
        if (const auto t = b - det; t > eps) {
            return t;
        }
        if (const auto t = b + det; t > eps) {
            return t;
        }
        return inf;
    }
};

constexpr Sphere spheres[] = {
    // Scene: radius, position, emission, color, material
    {1e5, {1e5 + 1, 40.8, 81.6}, {}, {.75, .25, .25}, DIFF},    // Left
    {1e5, {-1e5 + 99, 40.8, 81.6}, {}, {.25, .25, .75}, DIFF},  // Rght
    {1e5, {50, 40.8, 1e5}, {}, {.75, .75, .75}, DIFF},          // Back
    {1e5, {50, 40.8, -1e5 + 170}, {}, {}, DIFF},                // Frnt
    {1e5, {50, 1e5, 81.6}, {}, {.75, .75, .75}, DIFF},          // Botm
    {1e5, {50, -1e5 + 81.6, 81.6}, {}, {.75, .75, .75}, DIFF},  // Top
    {16.5, {27, 16.5, 47}, {}, {.999, .999, .999}, SPEC},       // Mirr
    {16.5, {73, 16.5, 78}, {}, {.999, .999, .999}, REFR},       // Glas
    {600, {50, 681.6 - .27, 81.6}, {12, 12, 12}, {}, DIFF}      // Lite
};

inline constexpr auto intersect(const RayLike auto &r) {
    const auto it = ranges::min_element(ranges::views::reverse(spheres), {}, [&](const auto& s){
      return s.intersect(r);
    });
    assert(it != ranges::rend(spheres));
    const auto t = (*it).intersect(r);
    return std::make_tuple(t < inf, t, std::ref(*it));
}

auto constexpr radiance(const RayLike auto &r, auto& prng, const int depth = 1) {
    // t is distance to intersection
    // obj is the intersected object
    const auto [intersects, t, obj] = intersect(r);
    if (!intersects) return Vec();  // if miss, return black
    const auto& [o, d] = r;
    const auto x = o + d * t, n = (x - obj.p).norm(),
         nl = n.dot(d) < 0 ? n : n * -1;
    const auto [bail, f] = [&, ff = obj.c]{
        if (depth > 5) {
            const auto p = std::max({ff.x, ff.y, ff.z});  // max refl
            if (prng() < p)
                return std::make_tuple(false, ff * (1 / p));
            
            return std::make_tuple(true, ff);
        }
        return std::make_tuple(false, ff);
    }();
    if (bail) {
        return obj.e;  // R.R.
    }
    if (obj.refl == DIFF) {  // Ideal DIFFUSE reflection
        const auto r1 = 2 * M_PI * prng(), r2 = prng(), r2s = csqrt(r2);
        const auto w = nl, u = ((fabs(w.x) > .1 ? Vec{0, 1} : Vec{1}) % w).norm(),
            v = w % u;
        const auto new_d =
            (u * cos(r1) * r2s + v * sin(r1) * r2s + w * csqrt(1 - r2)).norm();
        return obj.e + f.mult(radiance(Ray{x, new_d}, prng, depth + 1));
    } else if (obj.refl == SPEC)  // Ideal SPECULAR reflection
        return obj.e +
               f.mult(radiance(Ray{x, d - n * 2 * n.dot(d)}, prng, depth + 1));
    const auto reflRay =
        Ray{x, d - n * 2 * n.dot(d)};  // Ideal dielectric REFRACTION
    const auto into = n.dot(nl) > 0;             // Ray from outside going in?
    const auto nc = 1.0, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = d.dot(nl);
    if (auto cos2t = 1 - nnt * nnt * (1 - ddn * ddn);
        cos2t < 0) {  // Total internal reflection
        return obj.e + f.mult(radiance(reflRay, prng, depth + 1));
    } else {
        const auto tdir =
            (d * nnt - n * ((into ? 1 : -1) * (ddn * nnt + csqrt(cos2t))))
                .norm();
        const auto a = nt - nc, b = nt + nc, R0 = a * a / (b * b),
             c = 1 - (into ? -ddn : tdir.dot(n));
        const auto Re = R0 + (1 - R0) * c * c * c * c * c, Tr = 1 - Re,
             P = .25 + .5 * Re, RP = Re / P, TP = Tr / (1 - P);
        return obj.e +
               f.mult(depth > 2 ? (prng() < P
                                       ?  // Russian roulette
                                       radiance(reflRay, prng, depth + 1) * RP
                                       : radiance(Ray{x, tdir}, prng, depth + 1) * TP)
                                : radiance(reflRay, prng, depth + 1) * Re +
                                      radiance(Ray{x, tdir}, prng, depth + 1) * Tr);
    }
}

constexpr auto create_image(concepts::integral auto height, concepts::integral auto width, concepts::integral auto samples) {
    namespace rv = ranges::views;
    // create a single row
    const auto create_row = [=](auto y){
        auto prng = rand48{static_cast<uint64_t>(y*y*y) << 32};
        constexpr auto o = Vec{50, 52, 295.6}, d = Vec{0, -0.042612, -1}.norm();  // cam pos, dir
        const auto cx = Vec{width * .5135 / height}, cy = (cx % d).norm() * .5135;
        return rv::iota(0, width) | rv::transform([&](const auto x) { // Loop cols
            return reduce(rv::iota(0, 2) | rv::transform([&](const auto sy) { // 2x2 subpixel rows
                return reduce(rv::iota(0, 2) | rv::transform([&](const auto sx) { // 2x2 subpixel cols
                    const auto r = reduce(rv::iota(0, samples)
                        | rv::transform([&, scale = 1. / samples](const auto) {
                        const auto r1 = 2 * prng(),
                            dx = r1 < 1 ? csqrt(r1) - 1 : 1 - csqrt(2 - r1);
                        const auto r2 = 2 * prng(),
                            dy = r2 < 1 ? csqrt(r2) - 1 : 1 - csqrt(2 - r2);
                        const auto dd = (cx * (((sx + .5 + dx) / 2 + x) / width - .5) +
                                cy * (((sy + .5 + dy) / 2 + y) / height - .5) +
                                d).norm();
                        return radiance(Ray{o + dd * 140, dd}, prng) * scale;
                    }));  // Camera rays are pushed ^^^^^ forward to start in interior
                    return Vec{clamp(r.x), clamp(r.y), clamp(r.z)} * .25;
                }));
            }));
        }) | ranges::to<std::vector>;
    };
    
    if (std::is_constant_evaluated()) {
        return transform_reverse_join(rv::iota(0, height), create_row);  // Loop over image rows
    } else {
        std::atomic<int> done = 0;
        return transform_reverse_join(rv::iota(0, height), [&](const auto y) {  // Loop over image rows
            const auto res = create_row(y);
            fmt::print(std::cerr, "\rRendering ({} spp) {:5.2f}%", samples * 4,
                    100. * done.fetch_add(1, std::memory_order_relaxed) / (height - 1));
            return res;
        });
    }
}

#if __cpp_lib_constexpr_vector
static_assert(test_result(create_image(2, 2, 1), {136, 136, 136, 0, 0, 0, 92, 12, 34, 0, 0, 0}}));
#endif

int main(int argc, char *argv[]) {
    // sanity checks
#ifndef __clang__
    assert(test_result(create_image(2, 2, 1), {136, 136, 136, 0, 0, 0, 92, 12, 34, 0, 0, 0}));
    assert(test_result(create_image(3, 3, 1), {186, 186, 186, 136, 136, 136, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 136, 136, 136}));
#endif
    
    constexpr auto h = 768, w = 1024;
    const auto samps = argc == 2 ? atoi(argv[1]) / 4 : 1;  // # samples
    const auto c = create_image(h, w, samps);
    std::ofstream f{"image.ppm"};  // Write image to PPM file.
    fmt::print(f, "P3\n{} {}\n{}\n{} ", w, h, 255, fmt::join(c, " "));
}
