# Debug Build

## Configure and build

```bash
cmake -B build-debug \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE=/Users/kaz/Dev/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_PREFIX_PATH=/Users/kaz/Dev/quration/build/vcpkg_installed/arm64-osx

cmake --build build-debug
```

## Run in VS Code debugger

1. Install the [CodeLLDB](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb) extension
2. Update args in `.vscode/launch.json` if needed
3. Set breakpoints, press **F5**

Current launch config: `build-debug/main/qret compile --pipeline qpe_pipeline.yaml`
