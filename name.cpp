#include "name.hpp"

#include <boost/algorithm/string.hpp>

void name::set(std::string_view str)
{
  const auto len = str.size();
  assert(len <= 13 && "Name is longer than 13 characters");
  value = string_to_uint64_t(str);
  assert(to_string() == str && "Name not properly normalized ");
}

std::string name::to_string() const
{
  static const char *charmap = ".12345abcdefghijklmnopqrstuvwxyz";

  std::string str(13, '.');

  uint64_t tmp = value;
  for (uint32_t i = 0; i <= 12; ++i)
  {
    char c = charmap[tmp & (i == 0 ? 0x0f : 0x1f)];
    str[12 - i] = c;
    tmp >>= (i == 0 ? 4 : 5);
  }

  boost::algorithm::trim_right_if(str, [](char c) { return c == '.'; });
  return str;
}
