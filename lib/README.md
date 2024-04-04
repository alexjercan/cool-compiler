# COOL Modules

This directory contains the modules of the COOL language.

## depends.txt

In the `depends.txt` file you can find the dependencies of each module. The
syntax has the following rules:

- dependency

```txt
module1 : module2 | module3 | module4 | ...
```

This means that module1 depends on module2 or module3 or module4 or ...

- conflict

```txt
module1 ^ module2
```

This means that module1 and module2 are incompatible.

For example, in the current implementation, `allocator` and `mallocator` are
incompatible because they both define the same functions. Then, `raylib` needs
`prelude` to work, so the `depends.txt` file defines the dependency `raylib :
prelude`.

## module

To create a new module you need to create a new directory with the name of the
module. Inside the directory, you need to create a `flag.txt` file with the
flags needed to link with `ld`. For example, if you need to link with `libc`
you need to add the flag `-lc` in the `flag.txt` file. You also need to add
`-dynamic-linker` and `/lib64/ld-linux-x86-64.so.2` to the flags, when linking
with `libc` (or any other library that needs the dynamic linker). On the other
hand, if you only depend on `prelude` and `allocator` the binary will be
statically linked. But if you start to depend on `mallocator` or `raylib`, etc
you will need to link dynamically.

The `module` directory should contain the necessary `.cl` files that define the
behavior of the module. If you define `extern` functions in the module, you
also need the `.asm` files that implement the functions. The names of these
files do not matter, since the compiler will just concatenate all the files in
the module directory.

## tests

The tests for the modules should be placed in the `../tests/lib` directory. The
format is `%02d-%s.cl` where `%02d` is the number of the test and `%s` is the
name of the test. The tests should be written in COOL and should test the
functionality of the module. Then you also need the same file with `.ref`
extension for the expected output and `.flags` for the module list that the
compiler needs to include (e.g. `--module allocator --module prelude`).
