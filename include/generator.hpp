#pragma once
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <string>

namespace xmlpp {

class generator_error : public std::runtime_error {
  using std::runtime_error::runtime_error;
};
/**
 * Safely write a string.
 */
char *writeText(char *it, const char *limit, const char *text);
/**
 * Safely write a word.
 */
char *writeWord(char *it, const char *limit, const char *wordBeg,
                const char *wordEnd);
/**
 * Represents a tag.
 */
class TagGenerator {
public:
  TagGenerator(char *&current, const char *name, const char *limit,
               TagGenerator *parent = nullptr)
      : m_current(current), m_limit(limit), m_name(name), m_parent(parent) {
    if (m_parent) {
      m_parent->m_lastOpenChild = this;
    }
    writeText("<");
    writeText(name);
  }

  TagGenerator(const TagGenerator &) = delete;
  TagGenerator &operator=(const TagGenerator &) = delete;
  TagGenerator(TagGenerator &&rhs)
      : m_current(rhs.m_current), m_limit(rhs.m_limit), m_name(rhs.m_name),
        m_descedants(rhs.m_descedants), m_open(rhs.m_open),
        m_parent(rhs.m_parent), m_lastOpenChild(rhs.m_lastOpenChild) {
    rhs.m_open = false;
    if (m_descedants) {
      if (m_lastOpenChild) {
        m_lastOpenChild->m_parent = this;
      }
    }
    if (m_parent && m_parent->m_lastOpenChild == &rhs) {
      m_parent->m_lastOpenChild = this;
    }
  }
  TagGenerator &operator=(TagGenerator &&rhs) {
    close();
    new (this) TagGenerator(std::move(rhs));
    return *this;
  }

  ~TagGenerator() { close(); }

  /**
   * @brief Adds parameter
   */
  void addParameter(const char *name, const char *value) {
    if (!m_open) {
      throw generator_error("Can not add descedant to a closed tag");
    }
    if (m_descedants) {
      using namespace std;
      throw generator_error("Can not create parameter '"s + name +
                            "' because the tag already wrote a descedant.");
    }
    // TODO Sanitize name
    writeText(" ");
    writeText(name);
    writeText("='");
    // TODO Escape value
    writeText(value);
    writeText("'");
  }

  /**
   * @brief Flushes and write contents
   */
  void close() {
    if (!m_open) {
      return;
    }
    if (m_descedants) {
      checkSubTagClosed();

      writeText("</");
      writeText(m_name.c_str());
      writeText(">");
    } else {
      writeText("/>");
    }
    if (m_parent) {
      m_parent->m_lastOpenChild = nullptr;
    }
    m_open = false;
  }

  /**
   * @brief Adds a (sub)tag
   */
  TagGenerator addTag(const char *name) {
    checkDescendants();
    return TagGenerator{m_current, name, m_limit, this};
  }

  /**
   * @brief Adds a (sub)tag
   */
  void addText(const char *text) {
    checkDescendants();
    auto beg = text;
    for (auto it = text; *it != '\0'; ++it) {
      switch (*it) {
      case '<':
        writeWord(beg, it);
        writeText("&lt;");
        beg = it + 1;
        break;
      case '>':
        writeWord(beg, it);
        writeText("&gt;");
        beg = it + 1;
        break;
      case '&':
        writeWord(beg, it);
        writeText("&amp;");
        beg = it + 1;
        break;
      case '\n':
      case '\r':
      case '\t':
        break;
      default:
        if (*it <= 0) { // Sometimes char is signed u.u'.
          break;
        }
        if (*it < 0x20) {
          writeWord(beg, it);
          char t[] = "&#x00;";
          if (*it >= 0x10) {
            t[3] = '1';
          }
          auto d = *it % 0x10;
          if (d < 0xA) {
            t[4] = '0' | d;
          } else if (d != 0) {
            t[4] = 'A' + d - 0xA;
          }
          writeText(t);
          beg = it + 1;
        }
        break;
      }
    }
    // TODO check escaping.
    writeText(beg);
  }

  void addComment(const char *comment) {
    checkDescendants();
    writeText("<!--");
    writeText(comment); // TODO check for --
    writeText("-->");
  }

private:
  void checkDescendants() {
    if (!m_open) {
      throw generator_error("Can not add descedant to a closed tag");
    }
    if (!m_descedants) {
      m_descedants = true;
      writeText(">");
    }
    checkSubTagClosed();
  }

  void checkSubTagClosed() {
    if (m_lastOpenChild) {
      m_lastOpenChild->close();
    }
  }

  void writeText(const char *text) {
    m_current = xmlpp::writeText(m_current, m_limit, text);
  }
  void writeWord(const char *beg, const char *end) {
    m_current = xmlpp::writeWord(m_current, m_limit, beg, end);
  }

private:
  char *&m_current;
  const char *m_limit;
  std::string m_name;
  bool m_descedants = false;
  bool m_open = true;
  TagGenerator *m_parent;
  TagGenerator *m_lastOpenChild = nullptr;
};

/**
 * @brief generates markup.
 */
class generator {
public:
  generator(char *buffer, size_t buffer_size)
      : m_current(buffer), m_limit(buffer + buffer_size) {}

  TagGenerator rootTag(const char *name) {
    if (m_root) {
      throw generator_error("Already wrote root");
    }
    m_root = true;
    writeHeader();
    return TagGenerator{m_current, name, m_limit};
  }

  generator &version(const char *version) {
    m_version = version;
    return *this;
  }

  generator &encoding(const char *encoding) {
    m_encoding = encoding;
    return *this;
  }

private:
  void writeHeader() {
    m_current = writeText(m_current, m_limit, "<?xml version='");
    m_current = writeText(m_current, m_limit,
                          m_version.size() ? m_version.c_str() : "1.0");
    m_current = writeText(m_current, m_limit, "' encoding='");
    m_current = writeText(m_current, m_limit,
                          m_encoding.size() ? m_encoding.c_str() : "UTF-8");
    m_current = writeText(m_current, m_limit, "'?>");
  }

private:
  char *m_current;
  const char *m_limit;
  std::string m_version;
  std::string m_encoding;
  bool m_root = false;
};

inline char *writeWord(char *it, const char *limit, const char *wordBeg,
                       const char *wordEnd) {
  size_t bufSize = wordEnd - wordBeg;
  if (it + bufSize >= limit) {
    throw generator_error("Word too big.");
  }
  memcpy(it, wordBeg, bufSize);
  it += bufSize;
  *it = '\0';
  return it;
}

inline char *writeText(char *it, const char *limit, const char *text) {
  size_t bufSize = strlen(text);
  if (it + bufSize >= limit) {
    throw generator_error("Word too big.");
  }
  memcpy(it, text, bufSize + 1);
  return it + bufSize;
}
}
