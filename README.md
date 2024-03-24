# Cool Compiler

Compiler for the [Classroom Object-Oriented
Language](https://theory.stanford.edu/~aiken/software/cool/cool-manual.pdf).

## Requirements

- [FASM](https://flatassembler.net/)

## Quickstart

To compile a single file use

```console
make
./build/main <file.cl>
```

this will create a file `main.asm` with the assembly code and a file `main`
with the executable.

To run the compiler for a specific stage use

```console
make
./build/main [--lex | --syn | --sem | --tac | --asm] [--o outfile] <file.cl> ...
```

To run the checker for a specific implementation use

```console
./checker.sh [--lex | --syn | --sem | --tac | --asm]
```

## Resources

- [Manual](https://theory.stanford.edu/~aiken/software/cool/cool-manual.pdf)
- [HTML Manual](https://dijkstra.eecs.umich.edu/eecs483/crm/One%20Page.html)
- [Downloads](https://web.eecs.umich.edu/~weimerw/2015-4610/cool.html)
