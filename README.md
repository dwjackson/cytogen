<!--
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
-->

<!--
Copyright (c) 2016-2020 David Jackson
-->

# Cytogen: static site generator

Cytogen (or `cyto`) is a static site generator written in C. It has built-in
support for templating, Markdown-esque markup, and blog posting. A "static-site
generator" is a program you can use to take a bunch of files (e.g. HTML and
Markdown) and generate a static website. The advantage here is that you can
imitate dynamic sites through templating and transformation without having any
extra code that has to run on the server. This means for example that your site
loads quickly and has less attack surface than a more dynamic website would.

## Building and Installing

Cytogen depends on the [Ctache](https://github.com/dwjackson/ctache)
templating library, a C99 compiler, and several POSIX-standard functions.

If you downloaded an archive of Cytopasm that does not have a configure script,
you will need to have the GNU Autotools installed to create it. To create the
configure script, run:

```sh
autoreconf -iv
```

To build and install Cytogen:

```sh
./configure
make
make install
```

Note that you may need to be `root` in order to install.

## Usage

To create a new Cytogen site:

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

To serve the site on a local webserver for debugging purposes (this is *not* a
"real" web-server, so don't try to use it as such!):

```sh
cd _site
cyto serve
```

Note: Most `cyto` commands are usable just by typing their first letter (this
works in cases where a specific command is the only one that begins with that
letter).

A more complete reference can be found in the `cytogen(1)` and `cyto(1)` man 
pages.

## cymkd

Cymkd is a Markdown-like dialect that aspires to conform to the
[CommonMark](http://commonmark.org/) specification. It is included in Cytogen
in the form of a shared library `libcymkd.so` -- see `cymkd(3)` for usage
information -- and as a standalone executable, `cymkd(1)`.

To use the `cymkd(1)` executable:

```sh
cymkd [file_name]
```

Invoked without an argument, `cymkd` can be used as a "filter" in that you can
pipe text to it and it will print output to `stdout`. For example:

```sh
cat some_file.md | cymkd > output.html
```

## License

Cytogen is licensed under the
[Mozilla Public License Version 2.0](https://www.mozilla.org/en-US/MPL/2.0/).
There is an [FAQ](https://www.mozilla.org/en-US/MPL/2.0/FAQ/) if you're not
familiar with the terms. The version of the MPLv2 used by this project is
compatible with the GPL.
