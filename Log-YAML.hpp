
#ifndef _LOG_YAML_H
#define _LOG_YAML_H

// Check for evil defines
#ifdef s
#error "A before this one did '#define s'. Try including Log-YAML.hpp before other headers that may suck"
#else

// TODO hex feature? Would be cool to apply to numbers and vectors of numbers
// TODO YAML features
// [ .inf, -.Inf, .NAN ]
/* [ 1234, 0x4D2, 02333 ]   : [ Decimal int, Hexadecimal int, Octal int ]
    [ 1_230.15, 12.3015e+02 ]: [ Fixed float, Exponential float ]
    [ .inf, -.Inf, .NAN ]    : [ Infinity (float), Negative, Not a number ]
    { Y, true, Yes, ON  }    : Boolean true
    { n, FALSE, No, off }    : Boolean false
Escape codes:
 Numeric   : { "\x12": 8-bit, "\u1234": 16-bit, "\U00102030": 32-bit }
 Protective: { "\\": '\', "\"": '"', "\ ": ' ', "\<TAB>": TAB }
 C         : { "\0": NUL, "\a": BEL, "\b": BS, "\f": FF, "\n": LF, "\r": CR,
               "\t": TAB, "\v": VTAB }
 Additional: { "\e": ESC, "\_": NBSP, "\N": NEL, "\L": LS, "\P": PS }
*/
/* TODO
// define _GNU_SOURCE to access INFINITY and NAN which are gnu-extensions
// https://www.gnu.org/software/libc/manual/html_node/Infinity-and-NaN.html
#define _GNU_SOURCE 1

#include<math.h>
#include<stdio.h>

main() {
    printf("%f\n", INFINITY);
    printf("%f\n", -INFINITY);
    printf("%f\n", NAN);
}

...
inf
-inf
nan
*/

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <list>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

namespace Log {

// Adapted from C++ Reference to make gcc3 happy
template<class T, T v>
struct integral_constant {
    static const T value = v;
    typedef T value_type;
    typedef integral_constant type; // using injected-class-name
    operator value_type() const { return value; }
};

typedef integral_constant<bool, true> true_type;
typedef integral_constant<bool, false> false_type;

    // How to make a type trait:
    // https://stackoverflow.com/a/12045843/914859
    template <typename Container>
    struct is_container : false_type { };

    // Add STL containers you want to support here:
    template <typename T, typename S> struct is_container<std::set<T,S> > : true_type { };
    template <typename T, typename S> struct is_container<std::vector<T,S> > : true_type { };
    template <typename T, typename S> struct is_container<std::list<T,S> > : true_type { };

    template <typename Arithmetic>
    struct is_arithmetic : false_type { };

    // Add numeric types you want to support here
    template<> struct is_arithmetic<int> : true_type { };
    template<> struct is_arithmetic<unsigned> : true_type { };
    template<> struct is_arithmetic<long> : true_type { };
    template<> struct is_arithmetic<unsigned long> : true_type { };
    template<> struct is_arithmetic<short> : true_type { };
    template<> struct is_arithmetic<unsigned short> : true_type { };
    template<> struct is_arithmetic<float> : true_type { };
    template<> struct is_arithmetic<double> : true_type { };


  using namespace std;

  class Log
  {
  private:
    bool use_stderr;
    vector<string> lines;
    string stderr_prefix;
    stack<set<string> > used_keys;
    stack<unsigned> next_anon_key_to_try;

    inline string indent()
    {
      ostringstream o;
      for(unsigned i=0; i<level; i++)
        o << "  ";
      return o.str();
    }

    inline void debug_line(const string &line) {
      if(use_stderr)
        cerr << line;
    }

    unsigned level;

    inline string quoted(const string& str)
    {
      return string("\"") + str + string("\""); 
    }

    // escaping borrowed from https://github.com/kazuho/picojson/blob/master/picojson.h
    // See LICENSE.picojson
    typedef back_insert_iterator<string> Iter;
    void static copy(const std::string &s, Iter& oi) {
      std::copy(s.begin(), s.end(), oi);
    }

    struct serialize_str_char {
      Iter oi;
      void operator()(char c) {
        switch (c) {
#define MAP(val, sym)                           \
          case val:                             \
            copy(sym, oi);                      \
            break
          MAP('"', "\\\"");
          MAP('\\', "\\\\");
          MAP('/', "\\/");
          MAP('\b', "\\b");
          MAP('\f', "\\f");
          MAP('\n', "\\n");
          MAP('\r', "\\r");
          MAP('\t', "\\t");
#undef MAP
        default:
          if (static_cast<unsigned char>(c) < 0x20 || c == 0x7f) {
            char buf[7];
            snprintf(buf, sizeof(buf), "\\u%04x", c & 0xff);
            std::copy(buf, buf + 6, oi);
          } else {
            *oi++ = c;
          }
          break;
        }
      }
    };

    string serialize_str(const string &instr) {
      string outstr;
      Iter oi(outstr);
      serialize_str_char process_char = {oi};
      std::for_each(instr.begin(), instr.end(), process_char);
      return outstr;
    }

    inline string escaped(const string& str)
    {
      return serialize_str(str);
    }

    inline string istr(int i)
    {
      ostringstream o;
      o << i;
      return o.str();
    }

    inline string anon_key() {
      int anon = next_anon_key_to_try.top();
      while(used_keys.top().count
            (istr(anon)))
        anon++;
      next_anon_key_to_try.pop();
      next_anon_key_to_try.push(anon+1);
      return istr(anon);
    }

    inline string key(const string& keystr)
    {
      string tstr;
      if(keystr == "")
        tstr = anon_key();
      else
        tstr = keystr;
      while(used_keys.top().count(tstr))
        tstr += "'";
      used_keys.top().insert(tstr);

      return quoted(escaped(tstr)) + string(":");
    }

    string headstr;

    string top_key;


  public:
    Log(const string& top_key,
        bool use_stderr=true,
        const string& stderr_prefix=string("(LOG) "))
      : use_stderr (use_stderr),
        stderr_prefix (stderr_prefix),
        top_key(top_key)
    {
        clear();
    }

    inline void clear()
    {
      level = 0;
      used_keys = stack<set<string> >();
      used_keys.push(set<string>());
      next_anon_key_to_try = stack<unsigned>(); 
      next_anon_key_to_try.push(0);
      // lines.clear();
      lines = vector<string>();
      lines.push_back("---\n");
      debug_line("---\n");
      headstr = string("---\n") + open(top_key);
    }

    inline string header()
    {
      debug_line(headstr);
      return headstr;
    }

    inline string terminator()
    {
        string termstr = "...\n";
        debug_line(termstr);
        return termstr;
    }

    inline string comma_sep(const vector<string> st)
    {
      ostringstream o;
      for(vector<string>::const_iterator i = st.begin();
          i != st.end(); i++) {
        o << (*i);
        if(i+1 != st.end())
          o << ", ";
      }
      return o.str();
    }

    inline string bracket(const string& str)
    {
      return "[" + str + "]";
    }

    // quote strings
    // TODO use type traits to merge with vector<string> version
    inline vector<string> to_strings(const vector<char*>& vt)
    {
      vector<string> r;
      for(vector<char*>::const_iterator i = vt.begin();
          i != vt.end(); i++) {
        ostringstream o;
        o << (*i);
        r.push_back
          (quoted(o.str()));
      }
      return r;
    }

    // quote strings
    inline vector<string> to_strings(const vector<string>& vt)
    {
      vector<string> r;
      for(vector<string>::const_iterator i = vt.begin();
          i != vt.end(); i++) {
        ostringstream o;
        o << (*i);
        r.push_back
          (quoted(o.str()));
      }
      return r;
    }

    // double, int
    template <typename T>
    inline vector<string> to_strings(const vector<T>& vt)
    {
      vector<string> r;
      for(typename vector<T>::const_iterator i = vt.begin();
          i != vt.end(); i++) {
        ostringstream o;
        o << (*i);
        r.push_back(o.str());
      }
      return r;
    }

    template <typename V>
    inline string log_specialize(const string keystr, const V& t, const false_type&, const true_type&)
    {
      vector<typename std::iterator_traits<typename V::const_iterator>::value_type> v(t.begin(), t.end());
      ostringstream o;
      o << indent()
        << key(keystr)
        << " "
        << bracket(comma_sep(to_strings(v)))
        << endl;
      lines.push_back(o.str());
      debug_line(o.str());
      return o.str();
    }

    template<typename T>
    inline string log_specialize(const string& keystr, T d, const true_type&, const false_type&)
    {
      ostringstream o;
      o << indent()
        << key(keystr)
        << " "
        << d
        << endl;
      lines.push_back(o.str());
      debug_line(o.str());
      return o.str();
    }

    template<typename T>
    inline string log(const string& keystr, const T t)
    {
        typedef is_arithmetic<T> truth_type;
        typedef is_container<T> container_truth_type;
        truth_type x;
        container_truth_type y;
        return log_specialize(keystr, t, x, y);
    }

    template<typename T>
    inline string log(const T t)
    {
      typedef is_arithmetic<T> truth_type;
      typedef is_container<T> container_truth_type;
      truth_type x;
      container_truth_type y;
      return log_specialize(string(""), t, x, y);
    }

    // operator () as alias for Log::log
    template<typename T>
    inline string operator ()(const T t)
    {
        return log(t);
    }

    template<typename T>
    inline string operator ()(const string& keystr, const T t)
    {
        return log(keystr, t);
    }

    // string-like
    template<typename T>
    inline string log_specialize(const string& keystr, const T& str, const false_type&, const false_type&)
    {
      ostringstream o;
      o << indent()
        << key(keystr)
        << " "
        << quoted(escaped(str))
        << endl;
      lines.push_back(o.str());
      debug_line(o.str());
      return o.str();
    }


    inline string open(const string& str)
    {
      ostringstream o;
      o << indent()
        << key(str)
        << endl;
      lines.push_back(o.str());
      debug_line(o.str());
      level++;
      used_keys.push(set<string>());
      next_anon_key_to_try.push(0);
      return o.str();
    }

    inline string close()
    {
      if(level == 1) return string("");
      level--;
      used_keys.pop();
      next_anon_key_to_try.pop();
      return string("");
    }

    inline string str()
    {
      ostringstream ostr;
      for(vector<string>::const_iterator i = lines.begin();
          i != lines.end();
          i++)
        ostr << (*i);
      ostr << "...\n";
      return ostr.str();
    }

    inline string logf(const string& keystr, const char* format, ...)
    {
        va_list ap;
        va_start (ap, format);
        char* tmp = 0;
        vasprintf(&tmp, format, ap);
        va_end(ap);
        log(keystr, tmp);
        string rstr(tmp);

        free(tmp);
        return(rstr);
    }

  };
}

#endif // evil defines check
#endif // _LOG_YAML_H
