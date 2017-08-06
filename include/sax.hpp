#pragma once

#include <cassert>
#include <cstring>
#include <string>
#include <unordered_map>

namespace xmlpp {
enum class entity_type { TAG, TAG_ENDING, COMMENT, TEXT };

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
    if (m_singletag) {
      m_type = entity_type::TAG_ENDING;
      m_singletag = false;
      return true;
    }
    if (*m_code == 0) {
      return false;
    }
    ignoreBlanks();
    if (*m_code == '<') {
      if (*(m_code + 1) == '!') {
        nextComment();
      } else {
        nextTag();
      }
    } else {
      nextText();
    }
    return true;
  }

  sax &operator++() {
    next();
    return *this;
  }

  sax operator++(int) {
    auto temp = *this;
    next();
    return temp;
  }

  /**
   * @brief returns the type of the current node.
   */
  entity_type type() const { return m_type; }

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
  const char *m_code;
  entity_type m_type;
  std::string m_value;
  params_map m_params;
  bool m_singletag = false;

private:
  void nextTag() {
    using namespace std;
    assert(*m_code == '<');
    auto tag_beg = ++m_code;
    bool closing = false;
    if (*m_code == '/') {
      closing = true;
      ++m_code;
      m_type = entity_type::TAG_ENDING;
    } else {
      m_type = entity_type::TAG;
    }
    for (; *m_code != 0; ++m_code) {
      if (strchr(BLANKS, *m_code) || *m_code == '>' || *m_code == '/') {
        m_value.assign(tag_beg, m_code);
        parameters();
        break;
      }
    }
    if (*m_code == '/') {
      ++m_code;
      m_singletag = true;
    }
    if (*m_code == '>') {
      ++m_code;
    } else {
      throw runtime_error("Unclosed tag.");
    }
  }

  void nextComment() {
    assert(*m_code++ == '<');
    assert(*m_code++ == '!');
    expect('-');
    expect('-');
    auto comment_beg = m_code;
    size_t level = 0;
    for (; *m_code != 0; ++m_code) {
      if (*m_code == '-')
        ++level;
      else if (*m_code == '>' && level >= 2) {
        m_type = entity_type::COMMENT;
        m_value.assign(comment_beg, m_code - 2);
        ++m_code;
        return;
      } else
        level = 0;
    }
    throw std::runtime_error("Expected '-->' before end of the buffer");
  }

  void nextText() {
    auto text_beg = m_code;
    for (; *m_code != 0; ++m_code) {
      if (*m_code == '<') {
        break;
      }
    }
    m_type = entity_type::TEXT;
    m_value.assign(text_beg, m_code);
  }

  void parameters() {
    using namespace std;
  PARAM_NAME:
    ignoreBlanks();
    string pname = "";
    auto pname_beg = m_code;
    for (; *m_code != 0; ++m_code) {
      if (*m_code == '>' || *m_code == '/') {
        return;
      }
      if (*m_code == '=' || strchr(BLANKS, *m_code)) {
        if (m_code == pname_beg) {
          throw runtime_error(
              "Invalid Parameter. A name is expected before the '='");
        }
        pname.assign(pname_beg, m_code);
        goto PARAM_VALUE;
      }
    }
    throw runtime_error("Expected close tag or parameter definition");
  PARAM_VALUE:
    ignoreBlanks();
    if (*m_code != '=') {
      m_params[pname] = pname;
      goto PARAM_NAME;
    }
    ++m_code;
    ignoreBlanks();
    if (!strchr("\"\'", *m_code)) {
      throw runtime_error("Invalid Parameter '" + pname +
                          "'. The parameter value must be "
                          "surrounded by \' or \", we got: '" +
                          *m_code + "'");
    }
    char endToken = *m_code++;
    auto pvalue_beg = m_code++;
    for (; *m_code != 0; ++m_code) {
      if (*m_code == '>') {
        throw runtime_error("Expected a \' or \" before <");
      }
      if (*m_code == endToken) {
        m_params[pname].assign(pvalue_beg, m_code);
        ++m_code;
        goto PARAM_NAME;
      }
    }
    throw runtime_error("Unclosed parameter value");
  }

  void expect(char expected) {
    if (*m_code == expected) {
      ++m_code;
    } else {
      using namespace std;
      throw std::runtime_error("Expected char '"s + expected + "', got '" +
                               *m_code + "'.");
    }
  }

  size_t ignoreBlanks() {
    auto initial = m_code;
    while (strchr(BLANKS, *m_code)) {
      ++m_code;
    }
    return m_code - initial;
  }
};
}
