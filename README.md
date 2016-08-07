<!--
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
-->

<!--
Copyright (c) 2016 David Jackson
-->

# Cytoplasm: static site generator

Cytoplasm (or `cyto`) is a static site generator written in C. It has built-in
support for templating, Markdown-esque markup, and blog posting.

## Building and Installing

Cytoplasm depends on the [Ctache](https://github.com/dwjackson/ctache)
templating library, a C99 compiler, and several POSIX-standard functions.

If you downloaded an archive of Cytopasm that does not have a configure script,
you will need to have the GNU Autotools installed to create it. To create the
configure script, run:

```sh
autoreconf -iv
```

To build and install Cytoplasm:

```sh
./configure
make
make install
```

Note that you may need to be `root` in order to install.

## Usage

To create a new Cytoplasm site:

```sh
cyto init [project_name]
```

To generate the static site files:

```sh
cyto generate
```

To clean up (i.e. remove) the generated site files:

```sh
cyto clean
```

Note: Most `cyto` commands are usable just by typing their first letter (this
works in cases where a specific command is the only one that begins with that
letter).

A more complete reference can be found in the `cytoplasm(1)` and `cyto(1)` man 
pages.

## cymkd

Cymkd is a Markdown-like dialect that aspires to conform to the
[CommonMark](http://commonmark.org/) specification. It is included in Cytoplasm
in the form of a shared library `libcymkd.so` -- see `cymkd(3)` for usage
information -- and as a standalone executable, `cymkd(1)`.

To use the `cymkd(1)` executable:

```sh
cymkd [file_name]
```

## License

Cytoplasm is licensed under the
[Mozilla Public License Version 2.0](https://www.mozilla.org/en-US/MPL/2.0/).
There is an [FAQ](https://www.mozilla.org/en-US/MPL/2.0/FAQ/) if you're not
familiar with the terms. The version of the MPLv2 used by this project is
compatible with the GPL.
