#pragma once
#include <stdexcept>

namespace xmlpp {
class ParserError : public std::runtime_error
{
  using std::runtime_error::runtime_error;
};
}