
#ifndef _LOG_YAML_H
#define _LOG_YAML_H

// TODO logf -- like printf
// TODO more templates

// TODO fewer includes
#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <exception>
#include <iostream>
#include <iterator>
#include <list>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

#include <boost/type_traits.hpp>

namespace Log {

  using namespace std;

  class Log
  {
  private:
    bool use_stderr;
    vector<const string> lines;
    string stderr_prefix;
    ostringstream ostr;
    stack<set<const string> > used_keys;
    stack<const unsigned> next_anon_key_to_try;

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


  public:
    Log(const string& top_key,
        bool use_stderr=true,
        const string& stderr_prefix=string("(LOG) "))
      : use_stderr (use_stderr),
        stderr_prefix (stderr_prefix)
    {
      level = 0;
      used_keys.push(set<const string>());
      next_anon_key_to_try.push(0);
      headstr = open(top_key);
    }

    inline string head()
    {
      return headstr;
    }

    inline string comma_sep(const vector<const string> st)
    {
      ostringstream o;
      for(vector<const string>::const_iterator i = st.begin();
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
    inline vector<const string> to_strings(const vector<char*>& vt)
    {
      vector<const string> r;
      for(vector<const char* const>::const_iterator i = vt.begin();
          i != vt.end(); i++) {
        ostringstream o;
        o << (*i);
        r.push_back
          (quoted(o.str()));
      }
      return r;
    }

    // quote strings
    inline vector<const string> to_strings(const vector<string>& vt)
    {
      vector<const string> r;
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
    inline vector<const string> to_strings(const vector<T>& vt)
    {
      vector<const string> r;
      for(typename vector<T>::const_iterator i = vt.begin();
          i != vt.end(); i++) {
        ostringstream o;
        o << (*i);
        r.push_back(o.str());
      }
      return r;
    }

    template <typename V>
    inline string log_specialize(const string keystr, const V& t, const boost::false_type&, const boost::true_type&)
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
    inline string log_specialize(const string& keystr, T d, const boost::true_type&, const boost::false_type&)
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

    // How to make a type trait:
    // https://stackoverflow.com/a/12045843/914859
    template <typename Container>
    struct is_container : boost::false_type { };

    template <typename T, typename S> struct is_container<std::set<T,S> > : boost::true_type { };
    template <typename T, typename S> struct is_container<std::vector<T,S> > : boost::true_type { };
    template <typename T, typename S> struct is_container<std::list<T,S> > : boost::true_type { };

    template<typename T>
    inline string log(const string& keystr, const T t)
    {
        typedef boost::is_arithmetic<T> truth_type;
        typedef is_container<T> container_truth_type;
        truth_type x;
        container_truth_type y;
        return log_specialize(keystr, t, x, y);
    }

    template<typename T>
    inline string log(const T t)
    {
      typedef boost::is_arithmetic<T> truth_type;
      typedef is_container<T> container_truth_type;
      truth_type x;
      container_truth_type y;
      return log_specialize(string(""), t, x, y);
    }

    // string-like
    template<typename T>
    inline string log_specialize(const string& keystr, const T& str, const boost::false_type&, const boost::false_type&)
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
      used_keys.push(set<const string>());
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
      ostr.clear();
      for(vector<const string>::const_iterator i = lines.begin();
          i != lines.end();
          i++)
        ostr << (*i);
      return ostr.str();
    }
  };
}

#endif // _LOG_YAML_H
