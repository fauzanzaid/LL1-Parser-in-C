# LL1-Parser-in-C
An implemention of an LL1 Parser in C

### Building
First, download the dependencies:
```bash
./download_dependencies.sh
```

Then, to build the static library, run the following commands from the terminal:
```bash
mkdir build ; cd build && cmake .. && make ; cd ..
```
This will build ```libParserLL1.a``` in ```./lib``` directory.

### Usage
See ```include/ParserLL1.h``` for information about functionality provided by this module
