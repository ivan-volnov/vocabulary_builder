# vocabulary_builder

Vocabulary Builder tool for Anki

## Build requirements

- CMake >= 3.24
- Clang >= 15
- TODO: Check this

```bash
pkg install cmake llvm15
```

## Compilation

Clone the repo including all submodules

```bash
git clone --recurse-submodules -j6 git@github.com:ivan-volnov/vocabulary_builder.git
```

Update submodules if they changed after clone

```bash
git submodule update --init --recursive
```

Build

```bash
mkdir build; cd build/
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER:STRING=clang++15 -DCMAKE_C_COMPILER:STRING=clang15
cmake --build . -j4
cmake --install .
```

## Build options

To build tests add this to cmake call

```bash
-DST_TEST=ON
```

Enable SQL debugging

```bash
-DST_DEBUG_SQL=ON
```
