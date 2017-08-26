#pragma once

#include <cassert>
#include <cstring>
#include <cuchar>
#include <map>
#include <stack>
#include <string>
#include <unordered_map>

namespace xmlpp {
/**
 * @brief An error in parsing the XML.
 */
class ParserError : public std::runtime_error
{
  using std::runtime_error::runtime_error;
};

enum class EntityType
{
  TAG,
  TAG_ENDING,
  COMMENT,
  TEXT
};

/**
 * @brief A parser adhering to SAX inteface.
 *
 * The class itself acts as an iterator, a foward one. So each
 * time you call next() (or operator++()) it will advance to next
 * entity, in a Depth first approach.
 */
class Parser
{
public:
  static constexpr const char* BLANKS = " \t\n\r";
  /**
   * @brief constructor that takes a c-string with the content.
   */
  Parser(const char* aCode)
  {
    mCode = aCode;
    Next();
  }

  bool Next()
  {
    using namespace std;
    if (mSingletag) {
      mType      = EntityType::TAG_ENDING;
      mSingletag = false;
      return true;
    }
    if (*mCode == 0) {
      return false;
    }
    auto space = mIgnoreBlanks();
    if (*mCode == '<') {
      if (*(mCode + 1) == '!') {
        if (*(mCode + 2) == '-') {
          mNextComment();
        } else if (*(mCode + 2) == '[') {
          mNextText();
        }
      } else if (*(mCode + 1) == '?') {
        mNextDeclaration();
        return Next();
      } else {
        mNextTag();
      }
    } else {
      mCode -= space;
      mNextText();
    }
    return true;
  }

  Parser& operator++()
  {
    Next();
    return *this;
  }

  Parser operator++(int)
  {
    auto temp = *this;
    Next();
    return temp;
  }

  /**
   * @brief returns the type of the current node.
   */
  EntityType Type() const { return mType; }

  /**
   * @brief Returns the value of the current node.
   *
   * The meaning of value is:
   * | type() | Meaning                                    |
   * | ------ | ------------------------------------------ |
   * | tag    | the name of tag ("<root/>"'s name is root) |
   */
  const std::string& Value() const { return mValue; }

  /**
   * @brief the type of the parameters map.
   *
   * The underline type is implementation defined. Please use the aliased
   * type.
   * We guaranteed that it defines an iterator, the operator[], the begin(),
   * end(), count() and size() and is able to be used with range-for.
   */
  using ParamsMap = std::unordered_map<std::string, std::string>;

  /**
   * @brief Return the current parameters.
   *
   * The reference is valid until the next call of next() or operator++().
   */
  const ParamsMap& Parameters() const { return mParams; }

  /**
   * @brief Return the document's encoding.
   *
   *  Currently it always will return "UTF-8"
   */
  const std::string Encoding() const { return "UTF-8"; }

  /**
   * @brief Return the document's xml version.
   *
   * This will report whatever is informed on the XML declaration or
   * "1.0" if no declaration is provided.

   * In the future different behaviour can be enabled for different versions.
   */
  const std::string& Version() const { return mVersion; }

private:
  const char*             mCode;
  EntityType              mType;
  std::string             mValue;
  ParamsMap               mParams;
  bool                    mSingletag   = false;
  bool                    mInitialized = false;
  std::string             mVersion     = "1.0";
  std::stack<std::string> mTagStack;

private:
  void mNextTag()
  {
    using namespace std;
    assert(*mCode == '<');
    auto tag_beg = ++mCode;
    bool closing = false;
    if (*mCode == '/') {
      closing = true;
      tag_beg = ++mCode;
      mType   = EntityType::TAG_ENDING;
    } else {
      mType = EntityType::TAG;
    }
    for (; *mCode != 0; ++mCode) {
      if (strchr(BLANKS, *mCode) || *mCode == '>' || *mCode == '/') {
        mValue.assign(tag_beg, mCode);
        mReadParameters();
        break;
      }
    }
    if (mType == EntityType::TAG) {
      if (*mCode == '/') {
        ++mCode;
        mSingletag = true;
      } else {
        mTagStack.push(mValue);
      }
    } else {
      if (mTagStack.top() != mValue) {
        throw ParserError("Tag mismatch, opened with: " + mTagStack.top() +
                          ", but closed with: " + mValue);
      }
      mTagStack.pop();
    }
    if (*mCode == '>') {
      ++mCode;
    } else {
      throw ParserError("Unclosed tag.");
    }
  }

  void mNextComment()
  {
    assert(*mCode++ == '<');
    assert(*mCode++ == '!');
    mExpect('-');
    mExpect('-');
    auto   comment_beg = mCode;
    size_t level       = 0;
    for (; *mCode != 0; ++mCode) {
      if (*mCode == '-')
        ++level;
      else if (*mCode == '>' && level >= 2) {
        mType = EntityType::COMMENT;
        mValue.assign(comment_beg, mCode - 2);
        ++mCode;
        return;
      } else
        level = 0;
    }
    throw ParserError("Expected '-->' before end of the buffer");
  }

  void mNextText()
  {
    auto text_beg = mCode;
    mValue.clear();
    for (; *mCode != 0; ++mCode) {
      if (*mCode == '<') {
        if (*(mCode + 1) == '!' && *(mCode + 2) == '[') {
          mValue.append(text_beg, mCode);
          mValue.append(mCdataSequence());
          text_beg = mCode--;
        } else {
          break;
        }
      }
      if (*mCode == '&') {
        mValue.append(text_beg, mCode);
        mValue.append(mEscapeSequence());
        text_beg = mCode--;
      }
    }
    mType = EntityType::TEXT;
    mValue.append(text_beg, mCode);
  }

  void mNextDeclaration()
  {
    if (mInitialized) {
      throw ParserError("Invalid declaration or using processor "
                        "instruction, which aren't currently implemented.");
    }
    assert(*mCode++ == '<');
    assert(*mCode++ == '?');
    mExpect('x');
    mExpect('m');
    mExpect('l');
    mReadParameters();
    if (mParams.count("encoding")) {
      auto encoding = mParams["encoding"];
      if (encoding != "UTF-8") {
        throw ParserError("Invalid encoding:" + encoding);
      }
    }
    if (mParams.count("version")) {
      mVersion = mParams["version"];
    }
    mExpect('?');
    mExpect('>');
    mInitialized = true;
  }

  std::string mCdataSequence()
  {
    using namespace std;
    mEnsure('<');
    mEnsure('!');
    mEnsure('[');
    mExpect('C');
    mExpect('D');
    mExpect('A');
    mExpect('T');
    mExpect('A');
    mExpect('[');
    auto   cdata_beg = mCode;
    string result;
    for (; *mCode != 0; ++mCode) {
      if (*mCode == ']' && *(mCode + 1) == ']' && *(mCode + 2) == '>') {
        result.assign(cdata_beg, mCode);
        break;
      }
    }
    mEnsure(']');
    mEnsure(']');
    mEnsure('>');
    return result;
  }
  std::string mEscapeSequence()
  {
    using namespace std;
    mEnsure('&');
    auto escape_beg = mCode;
    for (; *mCode != 0; ++mCode) {
      if (*mCode == ';') {
        std::string escape(escape_beg, mCode);
        mEnsure(';');
        if (escape[0] == '#') {
          char32_t value = 0;
          if (escape[1] == 'x') {
            for (size_t i = 2; i < escape.size(); ++i) {
              value *= 0x10;
              if (isdigit(escape[i])) {
                value += escape[i] & 0xf;
              } else if (isupper(escape[i])) {
                value += (escape[i] - 'A') | 0xA;
              } else {
                value += (escape[i] - 'a') | 0xA;
              }
            }
          } else {
            for (size_t i = 1; i < escape.size(); ++i) {
              value *= 10;
              value += escape[i] & 0xf;
            }
          }
          if (value) {
            if (value < 0x7f) {
              return {char(value)};
            } else {
              string locale = std::setlocale(LC_ALL, nullptr);
              std::setlocale(LC_ALL, "en_US.utf8");
              char           buffer[MB_CUR_MAX + 1];
              std::mbstate_t state{};
              auto           end = c32rtomb(buffer, value, &state);
              buffer[end]        = 0;
              std::setlocale(LC_ALL, locale.c_str());
              return string(buffer);
            }
          }
        }
        static map<string, string> escapeMapping = {{"lt", "<"},
                                                    {"gt", ">"},
                                                    {"amp", "&"},
                                                    {"quot", "\""},
                                                    {"apos", "'"}};
        return escapeMapping[escape];
      }
    }
    throw ParserError("Invalid Escape Sequence");
  }

  void mReadParameters()
  {
    using namespace std;
  PARAM_NAME:
    mIgnoreBlanks();
    string pname     = "";
    auto   pname_beg = mCode;
    for (; *mCode != 0; ++mCode) {
      if (*mCode == '>' || *mCode == '/' || *mCode == '?') {
        return;
      }
      if (*mCode == '=' || strchr(BLANKS, *mCode)) {
        if (mCode == pname_beg) {
          throw ParserError(
            "Invalid Parameter. A name is expected before the '='");
        }
        pname.assign(pname_beg, mCode);
        goto PARAM_VALUE;
      }
    }
    throw ParserError("Expected close tag or parameter definition");
  PARAM_VALUE:
    mIgnoreBlanks();
    if (*mCode != '=') {
      mParams[pname] = pname;
      goto PARAM_NAME;
    }
    ++mCode;
    mIgnoreBlanks();
    if (!strchr("\"\'", *mCode)) {
      throw ParserError("Invalid Parameter '" + pname +
                        "'. The parameter value must be "
                        "surrounded by \' or \", we got: '" +
                        *mCode + "'");
    }
    char endToken   = *mCode++;
    auto pvalue_beg = mCode++;
    mParams[pname].clear();
    for (; *mCode != 0; ++mCode) {
      if (*mCode == '>') {
        throw ParserError("Expected a \' or \" before <");
      }
      if (*mCode == '&') {
        mParams[pname].append(pvalue_beg, mCode);
        mParams[pname].append(mEscapeSequence());
        pvalue_beg = mCode--;
      }
      if (*mCode == endToken) {
        mParams[pname].append(pvalue_beg, mCode);
        ++mCode;
        goto PARAM_NAME;
      }
    }
    throw ParserError("Unclosed parameter value");
  }

  void mExpect(char aExpected)
  {
    if (*mCode == aExpected) {
      ++mCode;
    } else {
      using namespace std;
      throw ParserError("Expected char '"s + aExpected + "', got '" + *mCode +
                        "'.");
    }
  }

  void mEnsure(char aExpected)
  {
    assert(*mCode == aExpected);
    ++mCode;
  }

  size_t mIgnoreBlanks()
  {
    auto initial = mCode;
    while (strchr(BLANKS, *mCode)) {
      ++mCode;
    }
    return mCode - initial;
  }
};
}
