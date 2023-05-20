# ResourceCompiler

Compiles files into byte buffers so that they can be easily accessed in executables.

## Testing

The Test directory is setup to work with [googletest](https://github.com/google/googletest).

## Building

Building with CMake and Make.

```sh
mkdir build
cd build
cmake .. -DResourceCompiler_BUILD_TEST=ON -DResourceCompiler_AUTO_RUN_TEST=ON
make
```

### Optional defines

| Option                              | Description                                          | Default |
| :---------------------------------- | :--------------------------------------------------- | :-----: |
| ResourceCompiler_BUILD_TEST         | Build the unit test program.                         |   ON    |
| ResourceCompiler_AUTO_RUN_TEST      | Automatically run the test program.                  |   OFF   |
| ResourceCompiler_USE_STATIC_RUNTIME | Build with the MultiThreaded(Debug) runtime library. |   ON    |
