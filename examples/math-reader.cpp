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
  if (p.Type() != EntityType::TAG || p.Value() != "math") {
    throw runtime_error("Invalid formula");
  }
  stack<char>   ops;
  stack<double> vls;
  int           count = 0;
  while (p.Next() && p.Value() != "math") {
    switch (p.Type()) {
      case EntityType::COMMENT:
        break;
      case EntityType::TAG:
        if (p.Value() == "value") {
          ops.push('v');
          vls.push(0);
        } else if (p.Value() == "add") {
          ops.push('a');
          vls.push(0);
        } else if (p.Value() == "mul") {
          ops.push('m');
          vls.push(1);
        } else {
          throw runtime_error("Invalid tag");
        }
        break;
      case EntityType::TAG_ENDING: {
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
      case EntityType::TEXT:
        if (ops.top() == 'v') {
          vls.top() = stod(p.Value());
        } else {
          throw runtime_error("Values should be encolsed by <value></value>");
        }
        break;
    }
  }
  if (p.Type() != EntityType::TAG_ENDING || p.Value() != "math") {
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
