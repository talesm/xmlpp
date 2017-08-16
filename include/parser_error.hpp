#pragma once
#include <stdexcept>

namespace xmlpp {
class parser_error : public std::runtime_error {
  using std::runtime_error::runtime_error;
};
}