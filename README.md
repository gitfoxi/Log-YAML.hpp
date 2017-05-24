
Log-YAML.hpp
===============

Data logs optimized for parsing convenience.

Log-YAML.hpp helps you generate logs in a highly-restricted subset of YAML that
is easy to parse.

Example
---------

### C++

    #include "Log-YAML.hpp"
    
    Log::Log log("log");
    log.log(9);
    log.log(11e-3);
    log.log("doh");
    log.log("x", 3.0);
    log.log("x", 1);
    log.log("x", "doh");
    log.open("sub");
    log.log("x", 3.0);
    log.log("x", 1);
    log.log("x", "doh");
    log.close();
    log.close(); // oops. do the sane thing anyway
    log.log(7);
    vector<int> v; 
    v += 1,2,3,4,5,6,7,8,9; // (using boost::assign)
    log.log(v);
    cout << log.str();

### Output

    "log":
      "0": 9
      "1": 0.011
      "2": "doh"
      "x": 3.0
      "x'": 1
      "x''": "doh"
      "sub":
        "x": 3.0
        "x'": 1
        "x''": "doh"
      "3": 7
      "4": [1, 2, 3, 4, 5, 6, 7, 8, 9] 


### Or, log as you go

Above, the object is created then dumped all at once. Sometimes you want to send
your logs out, line by line, as they're created.

    Log.log("log");
    cerr << log.head();
    cerr << log.int(9);

### Auto output to stderr with tag

    Log.log("log", use_stderr=true, stderr_tag="(LOG) ");
    log.int(9);

### Stderr Output

    (LOG) "log":
    (LOG)   "0": 9
    
Install
--------

Just copy `Log-YAML.hpp` into your project and go.

XXX You do need boost - TODO include `boost/type_traits.hpp` here

Features
----------

* Header-only
* No possible run-time errors
* No possible parsing errors

(well if you can make an error, let me know ... it's a bug.)

Rules
------

* Only two scalar types: numeric and string
* Lists (vectors) of one type only - no lists of objects
* Strings are quoted
* Keys are strings
* Keys are unique
* Keys assigned autmatically or manually

Advice
-------

The goal is to avoid writing parsers.

Try not to invent a mini language inside of a string. Its tempting sometimes to log something like "TEST PASSED - TIMESTAMP 602161095". That defeats the purpose. Instead, make an object like:

    "TEST":
      "PASSFAIL": "PASS"
      "TIMESTAMP":  602161095

FAQ
---

### Why not just use YAML with yaml-cpp?

YAML optimizes for readability. But not many parsers can deal with the fact that it's fine to use a list or an object as a key value.

Besides, my users have complained it's hard to build and include the monster in their simple projects.

### So why not just use JSON with picojson or something?

I like picojson (and even borrowed a few lines of their code -- thanks picojson!) but you have to create a whole object before outputting it. With YAML, you can output lines as they're created and if you suddenly stop it's still valid.

For both YAML and JSON, restrictions on where you can put a key-value pair and where you can put a list complicate the casual programmer's life and cause her to give up and invent an unparsable mini-language instead.


License
--------

Copyright Michael Fox 2017

BSD-3
