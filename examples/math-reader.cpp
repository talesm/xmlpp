#include <fstream>
#include <iostream>
#include <stack>
#include <string>
#include "Parser.hpp"
#include "config.h"

using namespace std;

string readfile(const char* file);
void eval(const string& buffer);

int
main()
{
  auto buffer = readfile(XMLPP_DIR "/examples/math.xml");
  cout << "Read buffer:" << endl;
  cout << buffer << endl << endl;
  cout << "Evals to: " << endl;
  eval(buffer);
  return 0;
}

void
eval(const string& buffer)
{
  using namespace xmlpp;
  Parser p(buffer.c_str());
  if (p.type() != entity_type::TAG || p.value() != "math") {
    throw runtime_error("Invalid formula");
  }
  stack<char>   ops;
  stack<double> vls;
  int           count = 0;
  while (p.next() && p.value() != "math") {
    switch (p.type()) {
      case entity_type::COMMENT:
        break;
      case entity_type::TAG:
        if (p.value() == "value") {
          ops.push('v');
          vls.push(0);
        } else if (p.value() == "add") {
          ops.push('a');
          vls.push(0);
        } else if (p.value() == "mul") {
          ops.push('m');
          vls.push(1);
        } else {
          throw runtime_error("Invalid tag");
        }
        break;
      case entity_type::TAG_ENDING: {
        ops.pop();
        double curValue = vls.top();
        vls.pop();
        if (ops.empty()) {
          cout << "Expression #" << ++count << ": " << curValue << endl;
        } else {
          switch (ops.top()) {
            case 'v':
              throw runtime_error(
                "Can not have any child tag inside a <value/>");
              break;
            case 'a':
              vls.top() += curValue;
              break;
            case 'm':
              vls.top() *= curValue;
              break;
            default:
              throw runtime_error("Invalid command:"s + ops.top());
          }
        }
        break;
      }
      case entity_type::TEXT:
        if (ops.top() == 'v') {
          vls.top() = stod(p.value());
        } else {
          throw runtime_error("Values should be encolsed by <value></value>");
        }
        break;
    }
  }
  if (p.type() != entity_type::TAG_ENDING || p.value() != "math") {
    throw runtime_error("Invalid formula");
  }
}

inline string
readfile(const char* file)
{
  string   buffer;
  ifstream input(file);
  string   line;
  while (getline(input, line)) {
    buffer += line + '\n';
  }
  return buffer;
}
