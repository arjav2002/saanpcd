# SaanpCD
SaanpCD is a programming language developed by four Computer Science undergraduates as part of their Compilers course.

This language is a C-like language with a few extra features `import`ed from Python.

## Features of the language

* The language is not a loosely-typed language, data types exist. The data types available are:
  - `int` (Integer)
  - `float` (Floating-point)
  - `char` (8-bit ASCII value)
  - `string` (Collection of `char`s)
  - `bool` (Boolean literal)
* Procedures (blocks of code that are repeated and can be called by the programmer) and functions (procedures that return a value to a variable) are supported. The following are features of procedures and functions:
  - Arguments are optional
  - Function definitions are preceded by a data type
  - Procedures are preceded by the keyword `proc`
  - Default values of arguments can be provided, hence providing all arguments when invoking is optional
* `// Single-line comments`
  and
  ```C
  /*
  Multi
  Line
  Comments
  */
  ```
  are allowed
* Statements __must__ end with a semicolon (`;`)
* Conditional statements follow the same conventions and syntax as C (refer below for syntax definitions)
* Only one type of loop statement is supported, keyword being `loop`, which follows the `while` syntax from C
* The various operators supported are
  - Arithmetic operators
    + Addition (`#`)
    + Subtraction (`~`)
    + Multiplication (`*`)
    + Division (`/`)
    + Modulo (`%`)
    + Unary increment (`##`)
    + Unary decrement (`~~`)
  - Logical operators
    + and (`and`)
    + or (`or`)
    + not (`not`)
    + nor (`nor`)
    + nand (`nand`)
    + xor (`xor`)
  - Bitwise operators
    + Bitwise one's compliment (`!`)
    + Bitwise and (`&`)
    + Bitwise or (`|`)
    + Bitwise xor (`^`)
  - Relational operators
    + equal to (`==` or `is`)
    + greater than (`>`)
    + greater than or equal to (`>=`)
    + less than (`<`)
    + less than or equal to (`<=`)

There is no `main` function, so the execution will start from the first statement not contained in a function. Execution starts from the top of the file (top-down approach)
