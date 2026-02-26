# Espresso 0.1 (alpha)

Espresso is a compiled, statically-typed programming language that transpiles 
to C++. It supports generics, traits, operator overloading, and string 
interpolation.

> ⚠️ Work in progress. The lexer and parser are functional. The semantic 
> analyser and code generator are not yet complete.

## Example

let x: Int = 42;
let name: String = "Espresso";
writeln($"Hello from {name}!");

func add(a: Int, b: Int) -> Int {
    return a + b;
}

## Building

Requires Python 3 and CMake.

On Unix/Linux/macOS:
​```bash
python -m venv venv
source venv/bin/activate
python setup.py
​```

On Windows:
​```bash
python -m venv venv
venv\Scripts\activate
python setup.py
​```

## Project Status
- Lexer: done
- Parser: done  
- Semantic analyser: in progress
- Code generator: not started

## Author
ascii0x41