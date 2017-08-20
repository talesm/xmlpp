#pragma once
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <string>

namespace xmlpp {

/**
 * Safely write a word.
 */
inline char *writeWord(char *it, const char *word, const char *limit);
/**
 * Represents a tag.
 */
class TagGenerator {
public:
  TagGenerator(char *&current, const char *name, const char *limit)
      : m_current(current), m_limit(limit), m_name(name) {
    m_current = writeWord(m_current, "<", m_limit);
    m_current = writeWord(m_current, name, m_limit);
  }
  void addParameter(const char *name, const char *value) {
    //TODO Sanitize name
    m_current = writeWord(m_current, " ", m_limit);
    m_current = writeWord(m_current, name, m_limit);
    m_current = writeWord(m_current, "='", m_limit);
    //TODO Escape value
    m_current = writeWord(m_current, value, m_limit);
    m_current = writeWord(m_current, "'", m_limit);
      
  }

  TagGenerator addTag(const char *name) {
    if(!m_descedants) {
      m_descedants = true;
      m_current = writeWord(m_current, ">", m_limit);
    }
    return TagGenerator{m_current, name, m_limit};
  }

  void close() { 
    if(m_descedants) {
      m_current = writeWord(m_current, "</", m_limit); 
      m_current = writeWord(m_current, m_name.c_str(), m_limit); 
      m_current = writeWord(m_current, ">", m_limit); 
    } else {
      m_current = writeWord(m_current, "/>", m_limit); 
    }
}

private:
  char *&m_current;
  const char *m_limit;
  std::string m_name;
  bool m_descedants = false;
};

/**
 * @brief generates markup.
 */
class generator {
public:
  generator(char *buffer, size_t buffer_size)
      : m_current(buffer), m_limit(buffer + buffer_size) {}

  TagGenerator rootTag(const char *name) {
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
    m_current = writeWord(m_current, "<?xml version='", m_limit);
    m_current = writeWord(m_current, m_version.size() ? m_version.c_str() : "1.0", m_limit);
    m_current = writeWord(m_current, "' encoding='", m_limit);
    m_current = writeWord(m_current, m_encoding.size() ? m_encoding.c_str() : "UTF-8", m_limit);
    m_current = writeWord(m_current, "'?>", m_limit);
  }

private:
  char *m_current;
  const char *m_limit;
  std::string m_version;
  std::string m_encoding;
};

inline char *writeWord(char *it, const char *word, const char *limit) {
  size_t bufSize = strlen(word);
  if (it + bufSize >= limit) {
    throw std::runtime_error("Word too big.");
  }
  strcpy(it, word);
  return it + bufSize;
}
}
