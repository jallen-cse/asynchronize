# Asynchronize

Friendly synchronization tools for modern C++.

[![License: MIT](https://img.shields.io/github/license/jallen-cse/error?color=blue&style=shield)](./LICENSE)
[![CircleCI](https://circleci.com/gh/jallen-cse/asynchronize.svg?style=shield)](https://circleci.com/gh/jallen-cse/asynchronize)

**Asynchronize** is an attempt to provide basic thread synchronization classes that have a feel similar to Python's `threading.Event`. I always 
found myself creating trios of `bool`, `std::mutex`, and `std::condition_variable` to achieve this same pattern with far more lines of code.

## Usage
:rotating_light: **Read [Todo](#todo) before using!** :rotating_light: 

**Asynchronize** is currently a header-only library (this will change in the future w/ static & dynamic lib builds). Simply paste the
header into your source tree and get back to work!

*The library depends on your system's thread implementation (like `pthreads` on Linux) so you may need to add link options to your build.*

## Docs (Incomplete)
Documentation is contained inline in the source file but is also available through Doxygen. Open `/doc/html/annotated.html` in a browser to view the generated docs.

## Todo
**Asynchronize** is a work in progress with a totally unsolidified API. I need to do a functionality / sanity check over the entire project. Things 
will be broken! Likewise, it is all untested, so there may be some unexpected behaviors lurking.
