#pragma once

#include <cstring>
#include <string>
#include <unordered_map>

namespace xmlpp {
enum class entity_type { tag };

/**
 * @brief A parser adhering to SAX inteface.
 *
 * The class itself acts as an iterator, a foward one. So each
 * time you call next() (or operator++()) it will advance to next
 * entity, in a Depth first approach.
 */
class sax {
public:
  static constexpr const char *BLANKS = " \t\n\r";
  /**
   * @brief constructor that takes a c-string with the content.
   */
  sax(const char *code) {
    using namespace std;
    for (; *code != 0; ++code) {
      if (*code == '<') {
        for (auto valueend = code + 1; *valueend != 0; ++valueend) {
          if (strchr(">/ \t\n\r", *valueend)) {
            m_value.assign(code + 1, valueend);
            return;
          }
        }
        throw runtime_error("Unclosed tag.");
      } else if (strchr(BLANKS, *code) == nullptr) {
        throw runtime_error("Invalid char '"s + *code + "'");
      }
    }
  }

  /**
   * @brief returns the type of the current node.
   */
  entity_type type() const { return entity_type::tag; }

  /**
   * @brief Returns the value of the current node.
   *
   * The meaning of value is:
   * | type() | Meaning                                    |
   * | ------ | ------------------------------------------ |
   * | tag    | the name of tag ("<root/>"'s name is root) |
   */
  const std::string &value() const { return m_value; }

  /**
   * @brief the type of the parameters map.
   *
   * The underline type is implementation defined. Please use the aliased type.
   * We guaranteed that it defines an iterator, the operator[], the begin(),
   * end(), count() and size() and is able to be used with range-for.
   */
  using params_map = std::unordered_map<std::string, std::string>;

  /**
   * @brief Return the current parameters.
   *
   * The reference is valid until the next call of next() or operator++().
   */
  const params_map &params() const { return m_params; }

private:
  std::string m_value;
  params_map m_params;
};
}
