# pug++
(c) 2022-, Mura.

This is a minimal implementation of pug-html translator.

It is written in C++20. So it is free from EcmaScript and nodejs;
however, modern C++ compiler is required.

Currently, it is just for me.

## Usage

Include the header 'pug.hpp' to use it.

```
#include "pug.hpp"
```

Translate a Pug string to HTML.

```
std::string const       pug{ "..." };
std::string const       html{ xxx::pug::pug_string(pug) };
```

Translate a Pug file to HTML.

```
std::filesystem::path const     path{ "..." };
std::string const               html{ pug_file(path) };
```