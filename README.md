# Cytoplasm: static site generator

Cytoplasm (or `cyto`) is a static site generator written in C.

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

## License

Cytoplasm is licensed under the
[Mozilla Public License Version 2.0](https://www.mozilla.org/en-US/MPL/2.0/).
There is an [FAQ](https://www.mozilla.org/en-US/MPL/2.0/FAQ/) if you're not
familiar with the terms. The version of the MPLv2 used by this project is
compatible with the GPL.
