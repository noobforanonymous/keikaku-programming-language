# Getting Started with Keikaku

## Installation

### Quick Install (Linux/macOS)
```bash
cd keikaku
./build.sh --install
```

### Manual Install
```bash
cd keikaku
mkdir build && cd build
cmake ..
make
sudo cp keikaku /usr/local/bin/
```

---

## Your First Keikaku Program

Create a file called `hello.kei`:

```keikaku
declare("Hello, World!")
```

Run it:
```bash
keikaku hello.kei
```

Output:
```
  Hello, World!
```

---

## Core Concepts

### 1. Variables & Types
Keikaku uses dynamic typing with explicit or implicit declaration.

```keikaku
name := "Guide"      # Implicit (preferred)
designate id = 123   # Explicit
list := [1, 2, 3]    # List
```

### 2. Control Flow
Structured decision making with `foresee` and `cycle`.

```keikaku
# Conditional
foresee x > 10:
    declare("High")
otherwise:
    declare("Low")

# Iteration
cycle from 1 to 5 as i:
    declare(i)
```

### 3. Functions (Protocols)
Define logic blocks that can yield values.

```keikaku
protocol add(a, b):
    yield a + b

result := add(10, 20)
```

### 4. Generators (Sequences)
Lazy evaluation for efficient data processing.

```keikaku
sequence count_up(max):
    cycle from 1 to max as i:
        yield i

# Use in loop
cycle through count_up(5) as n:
    declare(n)

# Generator expression
squares := (x * x for x through [1, 2, 3])
```

### 5. Async Programming
Non-blocking operations with valid syntax.

```keikaku
async protocol fetch_data():
    sleep(100)
    yield "data"

await fetch_data()
```

---

## Standard Library Basics

| Function | Description |
|----------|-------------|
| `declare(args...)` | Print output to console |
| `inquire(prompt)` | Read input from user |
| `measure(obj)` | Get length of list/string |
| `span(n)` | Generate range [0..n-1] |
| `text(val)` | Convert to string |
| `number(val)` | Convert to integer |

---

## Next Steps

- **[Language Tour](language_tour.md)**: Detailed specific syntax guide.
- **[Generators](generators.md)**: Master lazy streams and delegation.
- **[Async Guide](async.md)**: Learn about concurrency in Keikaku.

