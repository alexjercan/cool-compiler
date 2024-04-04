# Cool Compiler

Compiler for the [Classroom Object-Oriented
Language](https://theory.stanford.edu/~aiken/software/cool/cool-manual.pdf).

> [!NOTE]
> This is project contains some extra features compared to the standard COOL
> language. This is because I want to be able to do interop with assembly code.

The compiler is written in C and it generates assembly code for x86-64. More
over, the compiler will run the `fasm` assembler to generate an object file and
then link it with `ld`.

The compiler can be stopped at different stages of the compilation process by
using the `--lex`, `--syn`, `--sem`, `--map`, `--tac` and `--asm` flags.

The compiler accepts multiple files as positional arguments. It will parse each
file individually and then merge the resulting ASTs into a single one. This
allows for the definition of classes in different files and follows the same
semantics as the original COOL compiler. Then it will run the semantic analysis
and generate the assembly code.

The standard library of the COOL language is split into modules which can be
loaded at the compile phase by using the `--module` flag. The available modules
are:
- `allocator` - the default memory allocator implemented in the original COOL
  compiler.
- `data` - which is included by default in all programs and  contains data
  structures like `List`, `Array`, etc.
- `mallocator` - a memory allocator implemented using the `malloc` and `free`
  functions. (needs to be linked with the `libc` library)
- `net` - networking client and server implementations.
- `prelude` - the prelude of the language, which should be included in all
  programs.
- `random` - a random number generator.
- `raylib` - which contains bindings to the raylib library. (needs to be linked
  with the `raylib`, `lm` and `libc` libraries, it also only works with
  `mallocator` as the memory allocator)
- `threading` - a threading library that uses pthreads. (needs to be linked with
  the `pthread` library)

## Requirements

- [FASM](https://flatassembler.net/)

## Quickstart

To compile a single file use

```console
make
./coolc <file.cl>
```

this will create a file `main` with the executable.

To run the compiler for a specific stage use

```console
make
./coolc [--lex | --syn | --sem | --map | --tac | --asm] [--o outfile] <file.cl> ...
```

To run the checker for a specific implementation use

```console
./checker.sh [--lex | --syn | --sem | --tac | --asm]
```

To compile the examples with the `coolc` compiler use

```console
make examples
```

this will generate all the example executables in the `build` folder.

To create a distributable version of the compiler use

```console
make dist
```

this will create `coolc.tar.gz` with the compiler and the standard library.

## Resources

- [Manual](https://theory.stanford.edu/~aiken/software/cool/cool-manual.pdf)
- [HTML Manual](https://dijkstra.eecs.umich.edu/eecs483/crm/One%20Page.html)
- [Downloads](https://web.eecs.umich.edu/~weimerw/2015-4610/cool.html)
- [Raylib](https://www.raylib.com/)
