#include <helgoboss-learn/string-util.h>

using std::string;
using std::function;

namespace helgoboss::util {
  string toString(int maxSize, const function<void(char*, int)>& fillBuffer) {
    // TODO Can this be implemented in a better way?
    string s;
    s.resize((size_t) maxSize);
    fillBuffer(&s[0], maxSize);
    s.resize(s.find('\0'));
    return s;
  }
}