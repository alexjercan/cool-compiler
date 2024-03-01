# Cool Compiler

Compiler for the [Classroom Object-Oriented
Language](https://theory.stanford.edu/~aiken/software/cool/cool-manual.pdf).

This repo can be used to write a compiler in any specific language. The project
uses Docker for running all the implementations.

## Quickstart

To run the compiler for a specific project use

```console
./coolc <project> <file.cl> -- [--lex | --syn | --sem]
```

To run the checker for a specific implementation use

```console
./checker.sh <project> [--lex | --syn | --sem]
```

## Resources

- [Manual](https://theory.stanford.edu/~aiken/software/cool/cool-manual.pdf)
- [HTML Manual](https://dijkstra.eecs.umich.edu/eecs483/crm/One%20Page.html)
- [Downloads](https://web.eecs.umich.edu/~weimerw/2015-4610/cool.html)
