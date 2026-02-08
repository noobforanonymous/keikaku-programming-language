# Language Tour

Keikaku is designed for expressive scripting and structured control flow.

## Basic Syntax

### Variables
- Use `designate` for explicit declaration.
- Use `:=` for quick assignment.

```keikaku
designate name = "Keikaku"
count := 42
processed := false
```

### Output
Use `declare` to print values.

```keikaku
declare("Hello, World!")
count := 10
declare("Count:", count)
```

### Conditionals (`foresee`)
Replace `if/else` with `foresee`/`alternate`/`otherwise`.

```keikaku
foresee count > 10:
    declare("High")
alternate count > 5:
    declare("Medium")
otherwise:
    declare("Low")
```

### Loops (`cycle`)
Supported loops: `while`, `through` (for-each), `from`/`to` (range).

```keikaku
# While loop
cycle while count > 0:
    count = count - 1

# For each loop
list := [1, 2, 3]
cycle through list as item:
    declare(item)

# Range loop
cycle from 1 to 5 as i:
    declare(i)
```

## Functions (`protocol`)

Define reusable logic using `protocol`.

```keikaku
protocol greet(name):
    declare("Hello,", name)
    yield "done"

result := greet("User")
```
Functions can `yield` values, behaving as generators if multiple `yield`s are present.

## Generators (`sequence`)

Explicitly define generators with `sequence`.

```keikaku
sequence counter(start):
    cycle while true:
        yield start
        start = start + 1

gen := counter(0)
declare(proceed(gen))  # 0
declare(proceed(gen))  # 1
```

## Data Types

- **Integers**: `42`, `100`
- **Floats**: `3.14`, `1.0`
- **Strings**: `"hello"`
- **Booleans**: `true`, `false`
- **Lists**: `[1, 2, 3]` (dynamic arrays)
- **Dictionaries**: `{key: val}` (key-value pairs)

## Built-in Functions

- `measure(x)`: Length of item.
- `span(n)`: Create a range list `[0, ..., n-1]`.
- `text(x)`: Convert to string.
- `number(x)`: Convert to integer.
- `classify(x)`: Get type name.
