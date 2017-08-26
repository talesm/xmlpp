#pragma once
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <string>

namespace xmlpp {

class GeneratorError : public std::runtime_error
{
  using std::runtime_error::runtime_error;
};
/**
 * Safely write a string.
 */
char* WriteText(char* aIterator, const char* aLimit, const char* aText);
/**
 * Safely write a word.
 */
char* WriteWord(char*       aIterator,
                const char* aLimit,
                const char* aWordBeg,
                const char* aWordEnd);
/**
 * Represents a tag.
 */
class TagGenerator
{
public:
  TagGenerator(char*&        aCurrent,
               const char*   aName,
               const char*   aLimit,
               TagGenerator* aParent = nullptr)
    : mCurrent(aCurrent)
    , mLimit(aLimit)
    , mName(aName)
    , mParent(aParent)
  {
    if (mParent) {
      mParent->mLastOpenChild = this;
    }
    mWriteText("<");
    mWriteText(aName);
  }

  TagGenerator(const TagGenerator&) = delete;
  TagGenerator& operator=(const TagGenerator&) = delete;
  TagGenerator(TagGenerator&& aSource)
    : mCurrent(aSource.mCurrent)
    , mLimit(aSource.mLimit)
    , mName(aSource.mName)
    , mDescedants(aSource.mDescedants)
    , mOpen(aSource.mOpen)
    , mParent(aSource.mParent)
    , mLastOpenChild(aSource.mLastOpenChild)
  {
    aSource.mOpen = false;
    if (mDescedants) {
      if (mLastOpenChild) {
        mLastOpenChild->mParent = this;
      }
    }
    if (mParent && mParent->mLastOpenChild == &aSource) {
      mParent->mLastOpenChild = this;
    }
  }
  TagGenerator& operator=(TagGenerator&& rhs)
  {
    Close();
    new (this) TagGenerator(std::move(rhs));
    return *this;
  }

  ~TagGenerator() { Close(); }

  /**
   * @brief Adds parameter
   */
  void AddParameter(const char* aName, const char* aValue)
  {
    if (!mOpen) {
      throw GeneratorError("Can not add descedant to a closed tag");
    }
    if (mDescedants) {
      throw GeneratorError(std::string("Can not create parameter '") + aName +
                           "' because the tag already wrote a descedant.");
    }
    // TODO Sanitize name
    mWriteText(" ");
    mWriteText(aName);
    mWriteText("='");
    // TODO Escape value
    mWriteText(aValue);
    mWriteText("'");
  }

  /**
   * @brief Flushes and write contents
   */
  void Close()
  {
    if (!mOpen) {
      return;
    }
    if (mDescedants) {
      mCheckSubTagClosed();

      mWriteText("</");
      mWriteText(mName.c_str());
      mWriteText(">");
    } else {
      mWriteText("/>");
    }
    if (mParent) {
      mParent->mLastOpenChild = nullptr;
    }
    mOpen = false;
  }

  /**
   * @brief Adds a (sub)tag
   */
  TagGenerator AddTag(const char* aName)
  {
    mCheckDescendants();
    return TagGenerator{mCurrent, aName, mLimit, this};
  }

  /**
   * @brief Adds a (sub)tag
   */
  void AddText(const char* aText)
  {
    mCheckDescendants();
    auto beg = aText;
    for (auto it = aText; *it != '\0'; ++it) {
      switch (*it) {
        case '<':
          mWriteWord(beg, it);
          mWriteText("&lt;");
          beg = it + 1;
          break;
        case '>':
          mWriteWord(beg, it);
          mWriteText("&gt;");
          beg = it + 1;
          break;
        case '&':
          mWriteWord(beg, it);
          mWriteText("&amp;");
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
            mWriteWord(beg, it);
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
            mWriteText(t);
            beg = it + 1;
          }
          break;
      }
    }
    // TODO check escaping.
    mWriteText(beg);
  }

  void AddComment(const char* comment)
  {
    mCheckDescendants();
    mWriteText("<!--");
    mWriteText(comment); // TODO check for --
    mWriteText("-->");
  }

private:
  void mCheckDescendants()
  {
    if (!mOpen) {
      throw GeneratorError("Can not add descedant to a closed tag");
    }
    if (!mDescedants) {
      mDescedants = true;
      mWriteText(">");
    }
    mCheckSubTagClosed();
  }

  void mCheckSubTagClosed()
  {
    if (mLastOpenChild) {
      mLastOpenChild->Close();
    }
  }

  void mWriteText(const char* text)
  {
    mCurrent = xmlpp::WriteText(mCurrent, mLimit, text);
  }
  void mWriteWord(const char* beg, const char* end)
  {
    mCurrent = xmlpp::WriteWord(mCurrent, mLimit, beg, end);
  }

private:
  char*&        mCurrent;
  const char*   mLimit;
  std::string   mName;
  bool          mDescedants = false;
  bool          mOpen       = true;
  TagGenerator* mParent;
  TagGenerator* mLastOpenChild = nullptr;
};

/**
 * @brief generates markup.
 */
class Generator
{
public:
  Generator(char* aBuffer, size_t aBufferSize)
    : mCurrent(aBuffer)
    , mLimit(aBuffer + aBufferSize)
  {
  }

  TagGenerator RootTag(const char* aName)
  {
    if (mRoot) {
      throw GeneratorError("Already wrote root");
    }
    mRoot = true;
    mWriteHeader();
    return TagGenerator{mCurrent, aName, mLimit};
  }

  Generator& Version(const char* aVersion)
  {
    mVersion = aVersion;
    return *this;
  }

  Generator& Encoding(const char* aEncoding)
  {
    mEncoding = aEncoding;
    return *this;
  }

private:
  void mWriteHeader()
  {
    mCurrent = WriteText(mCurrent, mLimit, "<?xml version='");
    mCurrent = WriteText(mCurrent, mLimit,
                          mVersion.size() ? mVersion.c_str() : "1.0");
    mCurrent = WriteText(mCurrent, mLimit, "' encoding='");
    mCurrent = WriteText(mCurrent, mLimit,
                          mEncoding.size() ? mEncoding.c_str() : "UTF-8");
    mCurrent = WriteText(mCurrent, mLimit, "'?>");
  }

private:
  char*       mCurrent;
  const char* mLimit;
  std::string mVersion;
  std::string mEncoding;
  bool        mRoot = false;
};

inline char*
WriteWord(char* aIterator, const char* aLimit, const char* aWordBeg, const char* aWordEnd)
{
  size_t bufSize = aWordEnd - aWordBeg;
  if (aIterator + bufSize >= aLimit) {
    throw GeneratorError("Word too big.");
  }
  memcpy(aIterator, aWordBeg, bufSize);
  aIterator += bufSize;
  *aIterator = '\0';
  return aIterator;
}

inline char*
WriteText(char* aIterator, const char* aLimit, const char* aText)
{
  size_t bufSize = strlen(aText);
  if (aIterator + bufSize >= aLimit) {
    throw GeneratorError("Word too big.");
  }
  memcpy(aIterator, aText, bufSize + 1);
  return aIterator + bufSize;
}
}
