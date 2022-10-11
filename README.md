# pug++
(c) 2022-, Mura.

This is a minimal implementation of pug-html translator.

It is written in C++20. So it is free from EcmaScript and nodejs;
however, modern C++ compiler is required.

Currently, it is just for me.
Most of limitations are following:
 - It supports only tabs as indent.
 - It supports only single line element.
 - It supports only the 'extends' as forward reference.
 - It supports only the following order in an element: tag#id.class.class(attr,attr)
 - It supports only either the '#id' style or the '(id='..')' style in an element.
 - It supports only either the '.class' style or the '(class='..')' style in an element.
 - It does not support default of the 'extends'.
 - It does not support inline 'style'.
 - It does not support 'mixin'.
 - It does not support most of EcmaScript-like features.

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