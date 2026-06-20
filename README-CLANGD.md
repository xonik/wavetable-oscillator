# clangd Setup for Arduino/Teensy Projects

This guide explains how to configure clangd for proper IntelliSense support in Arduino/Teensy projects using PlatformIO.

## Quick Setup (This Repository)

The `.clangd` configuration file is already set up for this repository. If you encounter issues:

1. Regenerate the compile database:
   ```bash
   pio run -t compiledb
   ```

2. Restart clangd in VS Code:
   - Open Command Palette (Cmd+Shift+P)
   - Run: `clangd: restart`

## What This Provides

With proper clangd setup, you get:
- **Go to Definition** (Cmd+Click or F12) - Jump to function/class definitions
- **Hover Information** - See function signatures and documentation
- **Code Completion** - IntelliSense for Arduino/Teensy APIs
- **Find References** - See where symbols are used
- **Rename Refactoring** - Safely rename variables/functions across files

## Setting Up a New Repository

Follow these steps to configure clangd in a different Arduino/Teensy repository:

### 1. Generate Compile Database

In your project root:
```bash
pio run -t compiledb
```

This creates `compile_commands.json` with compilation flags for clangd.

### 2. Create `.clangd` Configuration

Create a `.clangd` file in the repository root with the necessary include paths and compiler flags. The paths below are for macOS with the current toolchain (GCC 11.3.1):

```yaml
CompileFlags:
  CompilationDatabase: .
  Add:
    - -I/Users/joakim/.platformio/packages/toolchain-gccarmnoneeabi-teensy/arm-none-eabi/include/c++/11.3.1
    - -I/Users/joakim/.platformio/packages/toolchain-gccarmnoneeabi-teensy/arm-none-eabi/include/c++/11.3.1/arm-none-eabi/thumb/v7e-m+dp/hard
    - -I/Users/joakim/.platformio/packages/toolchain-gccarmnoneeabi-teensy/arm-none-eabi/include/c++/11.3.1/backward
    - -I/Users/joakim/.platformio/packages/toolchain-gccarmnoneeabi-teensy/lib/gcc/arm-none-eabi/11.3.1/include
    - -I/Users/joakim/.platformio/packages/toolchain-gccarmnoneeabi-teensy/lib/gcc/arm-none-eabi/11.3.1/include-fixed
    - -I/Users/joakim/.platformio/packages/toolchain-gccarmnoneeabi-teensy/arm-none-eabi/include
    - --target=arm-none-eabi
    - -Wno-everything
    - -Wno-error
    - -isystem
    - /Users/joakim/.platformio/packages/framework-arduinoteensy/cores/teensy4
  Remove:
    - -nostdlib
    - -Wall
Diagnostics:
  Suppress:
    - "*"
  UnusedIncludes: None
  ClangTidy:
    Remove:
      - "*"
  MissingIncludes: None
Index:
  Background: Build
```

### 3. Create/Update VS Code Settings

In `.vscode/settings.json`:
```json
{
    "C_Cpp.intelliSenseEngine": "default",
    "C_Cpp.default.configurationProvider": "platformio.platformio-ide",
    "editor.formatOnSave": true,
    "C_Cpp.clang_format_style": "{ BasedOnStyle: LLVM, BreakBeforeBraces: Attach }"
}
```

### 4. Restart clangd

In VS Code:
- Command Palette (Cmd+Shift+P) 
- Run: `clangd: restart`

## Troubleshooting

### IntelliSense not working?
1. Check that clangd extension is installed: `clangd` by LLVM
2. Restart VS Code completely (not just the editor)
3. Regenerate compile database: `pio run -t compiledb`
4. Restart clangd: Command Palette → `clangd: restart`

### Incorrect include paths?
1. Check your GCC version: `ls ~/.platformio/packages/toolchain-gccarmnoneeabi-teensy/lib/gcc/arm-none-eabi/`
2. Update paths in `.clangd` if version differs
3. Run: `pio run -t compiledb` again

### "Unknown include" errors?
1. Ensure PlatformIO environment is built: `pio run`
2. Check that `compile_commands.json` exists in project root
3. Verify the compilation database has entries for your files

## Making It Portable Across Machines

The hardcoded paths in `.clangd` may need updating on different machines. Consider using a build script to auto-generate the configuration, or manually adjust paths when setting up on a new machine.

## References

- [clangd Documentation](https://clangd.llvm.org/)
- [PlatformIO Compilation Database](https://docs.platformio.org/en/latest/integration/ide/vscode.html#intellisense)
- [VS Code C++ IntelliSense](https://code.visualstudio.com/docs/languages/cpp)
