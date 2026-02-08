# Keikaku Programming Language

Keikaku is a dynamic, interpreted programming language designed for expressive control flow and structured state management. It features first-class support for lazy evaluation, generators with bidirectional communication, and asynchronous programming primitives.

## Key Features

- **Advanced Generators**: Full sequence protocol support including `yield`, `delegate` (yield from), and bidirectional state passing via `transmit`/`receive`.
- **Asynchronous Concurrency**: Built-in `async`/`await` syntax for non-blocking operations and promise management.
- **Structured Control Flow**: Expressive constructs like `foresee` (conditionals) and `cycle` (loops) with rich iteration capabilities.
- **Lazy Evaluation**: Generator expressions for efficient, on-demand data processing.
- **Robust Error Handling**: `attempt`/`recover` blocks and exception injection into generators via `disrupt`.

## specific syntax 

Keikaku uses a clean, keyword-rich syntax designed for readability and intent.

```keikaku
# Asynchronous Protocol
async protocol fetch_data(id):
    sleep(100)
    yield "data_" + text(id)

# Generator with Delegation
sequence aggregator():
    # Delegate to another iterator
    delegate fetch_data(1)
    delegate fetch_data(2)

# Execution
cycle through aggregator() as item:
    declare("Received:", item)
```

## Installation

### Automated Install Scripts

**Arch Linux** (using `makepkg`)
```bash
cd packaging/arch
makepkg -si
```

**Debian / Ubuntu / Kali**
```bash
./packaging/debian/install.sh
```

**Windows** (Cross-compile from Linux to .exe)
```bash
# Requires mingw-w64-gcc
./packaging/windows/build_exe.sh
# Output: packaging/windows/build-win/keikaku.exe
```

### Build from Source (Universal)

If the scripts above don't cover your OS, you can build manually using CMake:

```bash
mkdir build && cd build
cmake ..
make
sudo make install
```

## Usage

### Interactive REPL
Launch the interpreter without arguments to enter the Read-Eval-Print Loop:
```bash
keikaku
```

### File Execution
Run a Keikaku source file:
```bash
keikaku script.kei
```

## Documentation

- [Language Tour](docs/language_tour.md): An overview of syntax and basic concepts.
- [Generators & Iteration](docs/generators.md): In-depth guide to sequences, delegation, and lazy evaluation.
- [Asynchronous Programming](docs/async.md): Using async/await, promises, and concurrency patterns.
- [Error Handling](docs/error_handling.md): Managing exceptions and control flow disruptions.

## Website
**[https://keikaku.rothackers.com](https://keikaku.rothackers.com)**

## Author
Architected by **Regaan** // Founder of [Rot Hackers](https://rothackers.com)

## License
MIT License
