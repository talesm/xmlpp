#pragma once

#include <cassert>
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
    m_code = code;
    next();
  }

  bool next() {
    using namespace std;
    while (strchr(BLANKS, *m_code)) {
      ++m_code;
    }
    if (*m_code == '<') {
      nextTag();
    } else if (strchr(BLANKS, *m_code) == nullptr) {
      throw runtime_error("Invalid char '"s + *m_code + "'");
    }
    return true;
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
  //   using params_map = std::unordered_map<std::string, std::string>;
  struct params_map {
    size_t m_size = 0;
    size_t size() const { return m_size; }
  };

  /**
   * @brief Return the current parameters.
   *
   * The reference is valid until the next call of next() or operator++().
   */
  const params_map &params() const { return m_params; }

private:
  const char *m_code;
  std::string m_value;
  params_map m_params;

private:
  void nextTag() {
    using namespace std;
    assert(*m_code == '<');
    auto valuebeg = ++m_code;
    for (; *m_code != 0; ++m_code) {
      if (*m_code == '>' || *m_code == '/') {
        m_value.assign(valuebeg, m_code);
        return;
      } else if (strchr(BLANKS, *m_code)) {
        m_value.assign(valuebeg, m_code);
        goto PARAMETERS;
      }
    }
    throw runtime_error("Unclosed tag.");
  PARAMETERS:
    bool inParameter = false;
    for (; *m_code != 0; ++m_code) {
      if (*m_code == '>' || *m_code == '/') {
        return;
      }
      if (!bool(strchr(BLANKS, *m_code)) != inParameter) {
        if (inParameter) {
          inParameter = false;
        } else {
          inParameter = true;
          m_params.m_size++;
        }
      }
    }
  }
};
}
