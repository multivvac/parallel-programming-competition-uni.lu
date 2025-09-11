#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <istream>
#include <fstream>

inline std::string pair2str(const std::pair<int,int>& p, std::string_view sep = ",") {
    return std::to_string(p.first) + std::string(sep) + std::to_string(p.second);
}

struct Grid
{
  std::size_t width = 0;    // columns
  std::size_t height = 0;   // rows
  std::vector<int> data;

  void clear() {
    width = 0;
    height = 0;
    data.clear();
  }

  int w() {
    return static_cast<int>(width);
  }

  int h() {
    return static_cast<int>(height);
  }
};

namespace detail
{

  // read exactly n bytes or throw
  inline void read_exact(std::istream &in, char *dst, std::size_t n)
  {
    in.read(dst, static_cast<std::streamsize>(n));
    if (in.gcount() != static_cast<std::streamsize>(n))
    {
      throw std::runtime_error("Unexpected EOF while reading");
    }
  }

  inline uint64_t load_le_u64(const unsigned char *p)
  {
    return (uint64_t)p[0] | ((uint64_t)p[1] << 8) | ((uint64_t)p[2] << 16) | ((uint64_t)p[3] << 24) | ((uint64_t)p[4] << 32) | ((uint64_t)p[5] << 40) | ((uint64_t)p[6] << 48) | ((uint64_t)p[7] << 56);
  }

  inline int load_le_s8(const unsigned char *p)
  {
    return static_cast<int>(static_cast<int8_t>(p[0]));
  }
  inline int load_le_s16(const unsigned char *p)
  {
    int16_t v = (int16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
    return static_cast<int>(v);
  }
  inline int load_le_s32(const unsigned char *p)
  {
    int32_t v = (int32_t)((uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
    return static_cast<int>(v);
  }
  inline int load_le_s64(const unsigned char *p)
  {
    uint64_t u = load_le_u64(p);
    int64_t v = static_cast<int64_t>(u);
    if (v < std::numeric_limits<int>::min() || v > std::numeric_limits<int>::max())
    {
      throw std::runtime_error("Value does not fit in int");
    }
    return static_cast<int>(v);
  }

} // namespace detail

inline Grid parse_grid_from_bin(std::istream &in, int elem_bytes)
{
  if (elem_bytes != 1 && elem_bytes != 2 && elem_bytes != 4 && elem_bytes != 8)
  {
    throw std::invalid_argument("elem_bytes must be 1, 2, 4, or 8");
  }

  unsigned char hdr[16];
  detail::read_exact(in, reinterpret_cast<char *>(hdr), 16);
  uint64_t width = detail::load_le_u64(hdr + 0);
  uint64_t height = detail::load_le_u64(hdr + 8);

  if (width == 0 || height == 0)
  {
    throw std::runtime_error("Invalid header: width/height must be >0");
  }

  Grid M;
  M.width = static_cast<std::size_t>(width);
  M.height = static_cast<std::size_t>(height);
  M.data.resize(static_cast<std::size_t>(width * height));

  const std::size_t total = M.data.size();
  std::vector<unsigned char> buf(elem_bytes);

  for (std::size_t i = 0; i < total; i++)
  {
    detail::read_exact(in, reinterpret_cast<char *>(buf.data()), elem_bytes);
    switch (elem_bytes)
    {
    case 1:
      M.data[i] = detail::load_le_s8(buf.data());
      break;
    case 2:
      M.data[i] = detail::load_le_s16(buf.data());
      break;
    case 4:
      M.data[i] = detail::load_le_s32(buf.data());
      break;
    case 8:
      M.data[i] = detail::load_le_s64(buf.data());
      break;
    }
  }

  return M;
}

// Convenience: parse directly from a file
inline Grid parse_grid_from_bin_file(const std::string &path, int elem_bytes)
{
  std::ifstream fin(path, std::ios::binary);
  if (!fin)
    throw std::runtime_error("Cannot open file: " + path);
  return parse_grid_from_bin(fin, elem_bytes);
}

class Timer
{
private:
  using Clock = std::chrono::steady_clock;
  using Second = std::chrono::duration<double, std::ratio<1>>;

  std::chrono::time_point<Clock> m_beg{Clock::now()};

public:
  void reset()
  {
    m_beg = Clock::now();
  }

  double elapsed() const
  {
    return std::chrono::duration_cast<Second>(Clock::now() - m_beg).count();
  }
};

template <typename Func, typename... Args>
double benchmark(Func &&func, int runs = 5, Args &&...args)
{
  Timer t;
  for (int i = 0; i < runs; ++i)
  {
    func(std::forward<Args>(args)...);
  }
  return t.elapsed() / runs;
}
