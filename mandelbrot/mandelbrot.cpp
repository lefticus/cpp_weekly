#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <iostream>
#include <thread>
#include <future>
#include <complex>
#include <cassert>
#include <execution>

constexpr std::size_t max_max_iterations      = 2000;
constexpr std::size_t max_iteration_increment = 200;
constexpr std::size_t start_max_iterations    = 400;

template<typename T> struct Point
{
  T x{};
  T y{};
  constexpr bool operator!=(const Point &p) const = default;
  constexpr bool operator==(const Point &p) const = default;
};

template<typename T> Point(T x, T y) -> Point<T>;

struct Settings
{
  Point<double> center{ 0.001643721971153, -0.822467633298876 };
  double scale                   = 3.0;
  double power                   = 2.0;
  double do_abs                  = false;
  std::size_t cur_max_iterations = start_max_iterations;
  bool canceling                 = false;

  constexpr bool operator!=(const Settings &) const = default;
  constexpr bool operator==(const Settings &) const = default;
};

struct Size
{
  unsigned int width{};
  unsigned int height{};
};

struct SizeIterator
{
  Size size;
  std::pair<std::size_t, std::size_t> loc{ 0, 0 };


  constexpr SizeIterator &operator++() noexcept
  {
    ++loc.first;
    if (loc.first >= size.width) {
      loc.first = 0;
      ++loc.second;
    }
    return *this;
  }

  constexpr SizeIterator operator++(int) noexcept
  {
    const auto prev = *this;
    ++(*this);
    return prev;
  }


  [[nodiscard]] constexpr bool operator!=(const SizeIterator &other) const noexcept { return loc != other.loc; }

  [[nodiscard]] constexpr const std::pair<std::size_t, std::size_t> &operator*() const noexcept { return loc; }
  [[nodiscard]] constexpr std::pair<std::size_t, std::size_t> &operator*() noexcept { return loc; }
};

[[nodiscard]] constexpr SizeIterator begin(const Size &t_s) noexcept
{
  return SizeIterator{ t_s };
}

[[nodiscard]] constexpr SizeIterator end(const Size &t_s) noexcept
{
  return SizeIterator{ t_s, { 0, t_s.height } };
}


template<typename T> struct Color
{
  T r{};
  T g{};
  T b{};
};

template<typename T> Color(T r, T g, T b) -> Color<T>;

template<std::size_t Power, typename Value> constexpr auto pow(Value t_val)
{
  auto result = t_val;
  for (std::size_t itr = 1; itr < Power; ++itr) { result *= t_val; }
  return result;
}

template<typename ComplexType, typename PowerType> constexpr auto opt_pow(const std::complex<ComplexType> &t_val, PowerType t_power)
{
  if (t_power == static_cast<PowerType>(1.0)) {
    return t_val;
  } else if (t_power == static_cast<PowerType>(2.0)) {
    return std::complex{ pow<2>(std::real(t_val)) - pow<2>(std::imag(t_val)), 2 * std::real(t_val) * std::imag(t_val) };
  } else if (t_power == static_cast<decltype(t_power)>(3.0)) {
    const auto a = std::real(t_val);
    const auto b = std::imag(t_val);
    return std::complex{ -3 * a * pow<2>(b) + pow<3>(a), 3 * pow<2>(a) * b - pow<3>(b) };
    //  } else if (t_power == static_cast<decltype(t_power)>(4.0)) {
    //    const auto a = std::real(t_val);
    //    const auto b = std::imag(t_val);
    //    return std::complex{ pow<4>(a) + pow<4>(b) - 6 * pow<2>(a) * pow<2>(b), 4 * pow<3>(a) * b - 4 * a * pow<3>(b) };
  } else {
    return std::pow(t_val, t_power);
  }
}

template<typename PointType, typename CenterType, typename ScaleType>
constexpr auto get_color(const Point<PointType> t_point,
                         const Point<CenterType> t_center,
                         const Size t_size,
                         const ScaleType t_scale,
                         std::size_t max_iteration,
                         const CenterType power,
                         const bool do_abs) noexcept
{
  const std::complex scaled{ t_point.x / (t_size.width / t_scale) + (t_center.x - (t_scale / static_cast<CenterType>(2.0))),
                             t_point.y / (t_size.height / t_scale) + (t_center.y - (t_scale / static_cast<CenterType>(2.0))) };

  auto current = scaled;

  auto iteration      = 0u;
  auto stop_iteration = max_iteration;

  while (iteration < stop_iteration) {
    if (std::norm(current) > (2.0 * 2.0) && stop_iteration == max_iteration) { stop_iteration = iteration + 5; }

    if (do_abs) { current = std::complex{ std::abs(std::real(current)), std::abs(std::imag(current)) }; }

    current = opt_pow(current, power);
    current += scaled;

    ++iteration;
  }

  if (iteration == max_iteration) {
    return Color{ 0.0, 0.0, 0.0 };
  } else {
    const auto value    = ((iteration + 1) - (std::log(std::log(std::abs(std::real(current) * std::imag(current))))) / std::log(power));
    const auto colorval = std::abs(static_cast<int>(std::floor(value * 10.0)));

    const auto colorband = colorval % (256 * 7) / 256;
    const auto mod256    = colorval % 256;
    const auto to_1      = mod256 / 255.0;
    const auto to_0      = 1.0 - to_1;

    switch (colorband) {
    case 0: return Color{ to_1, 0.0, 0.0 };
    case 1: return Color{ 1.0, to_1, 0.0 };
    case 2: return Color{ to_0, 1.0, 0.0 };
    case 3: return Color{ 0.0, 1.0, to_1 };
    case 4: return Color{ 0.0, to_0, 1.0 };
    case 5: return Color{ to_1, 0.0, 1.0 };
    case 6: return Color{ to_0, 0.0, to_0 };
    default: return Color{ .988, .027, .910 };
    }
  }
}

template<typename PointType, typename ColorType> void set_pixel(sf::Image &img, const Point<PointType> &t_point, const Color<ColorType> &t_color)
{
  const auto to_sf_color = [](const auto &color) {
    const auto to_8bit = [](const auto &f) { return static_cast<std::uint8_t>(std::floor(f * 255)); };
    return sf::Color(to_8bit(color.r), to_8bit(color.g), to_8bit(color.b));
  };
  img.setPixel(t_point.x, t_point.y, to_sf_color(t_color));
}

template<std::size_t Width, std::size_t Height> struct Image
{
  std::array<Color<double>, Width * Height> colors;

  const auto &operator[](const std::pair<std::size_t, std::size_t> &loc) const { return colors[loc.second * Width + loc.first]; }
  auto &operator[](const std::pair<std::size_t, std::size_t> &loc) { return colors[loc.second * Width + loc.first]; }
  //  auto &operator
};

template<std::size_t Width, std::size_t Height> constexpr auto get_indicies()
{
  std::array<std::pair<std::size_t, std::size_t>, Width * Height> indicies{};
  std::generate(begin(indicies), end(indicies), [index = begin(Size{ Width, Height })]() mutable { return *(index++); });
  return indicies;
}

// this entire interface should be redesigned, it's really not safe, but it
// works for this demonstration
template<std::size_t Width, std::size_t Height> void run(Image<Width, Height> *img, const Settings *global_settings)
{
  auto localImg                  = std::make_unique<Image<Width, Height>>(*img);
  auto settings                  = *global_settings;
  static constexpr auto indicies = get_indicies<Width, Height>();

  auto cur_max_iterations = settings.cur_max_iterations;

  while (!settings.canceling) {
    const auto start = std::chrono::system_clock::now();

    if (cur_max_iterations <= max_max_iterations) {
      constexpr Size size{ Width, Height };
      std::transform(std::execution::par_unseq, begin(indicies), end(indicies), begin(localImg->colors), [=](const auto &location) {
        return get_color(
          Point{ location.first, location.second }, settings.center, size, settings.scale, cur_max_iterations, settings.power, settings.do_abs);
      });

      // this is almost certainly UB, writing into shared data with no mutexes at all
      *img = *localImg;

      if (cur_max_iterations + max_iteration_increment >= max_max_iterations) {
        std::cout << "Max iterations rendered in " << std::chrono::duration<double>{ std::chrono::system_clock::now() - start }.count() << "s\n";
      }
    }

    const auto new_settings = *global_settings;

    if (new_settings != settings) {
      settings           = new_settings;
      cur_max_iterations = settings.cur_max_iterations;
    } else {
      cur_max_iterations += max_iteration_increment;
    }

    std::this_thread::yield();
  }
}

constexpr static Size size{ 640u, 640u };


int main()
{
  sf::RenderWindow window(sf::VideoMode(640u, 640u), "Tilemap");
  window.setVerticalSyncEnabled(true);
  window.display();


  sf::Image img;
  img.create(size.width, size.height);
  sf::Texture texture;
  sf::Sprite bufferSprite(texture);
  texture.loadFromImage(img);


  bufferSprite.setTexture(texture);
  bufferSprite.setTextureRect(sf::IntRect(0, 0, size.width, size.height));
  bufferSprite.setPosition(0, 0);


  Settings settings{};

  auto img_colors = std::make_unique<Image<640u, 640u>>();

  std::thread worker(run<640u, 640u>, img_colors.get(), &settings);

  while (window.isOpen()) {
    const auto img_copy = std::make_unique<Image<640u, 640u>>(*img_colors);
    for (const auto &loc : size) { set_pixel(img, Point{ loc.first, loc.second }, (*img_copy)[loc]); }

    texture.loadFromImage(img);

    window.draw(bufferSprite);
    window.display();

    settings = [settings = Settings(settings)]() mutable {
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::PageUp)) { settings.scale *= 0.9; }
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::PageDown)) { settings.scale *= 1.1; }
      auto move_offset = settings.scale / 640;

      if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) { move_offset *= 10; }
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) { settings.center.x -= move_offset; }
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) { settings.center.x += move_offset; }
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) { settings.center.y -= move_offset; }
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) { settings.center.y += move_offset; }
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
          settings.power += 0.1;
        } else {
          settings.power -= 0.1;
        }
      }
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) { settings.do_abs = !settings.do_abs; }

      return settings;
    }();
  }

  Settings canceling  = settings;
  canceling.canceling = true;
  settings            = canceling;
  worker.join();
}
