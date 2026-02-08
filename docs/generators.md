# Generators & Iteration in Keikaku

Generators are a core feature of Keikaku, providing powerful lazy sequences and bidirectional communication.

## Creating Generators (`sequence`)

Use the `sequence` keyword to define a generator function. Execution pauses at each `yield`.

```keikaku
sequence numbers(max):
    cycle from 1 to max as i:
        yield i

sum := 0
cycle through numbers(10) as n:
    sum = sum + n
declare(sum)
```

## Delegation (`delegate`)

Delegate allows yielding all values from another iterable (generator or list) seamlessly.

```keikaku
sequence subtask():
    yield "a"
    yield "b"

sequence maintask():
    yield "start"
    delegate subtask()
    yield "end"

# Produces: start, a, b, end
```

## Bidirectional Communication (`transmit`/`receive`)

Generators can receive values from the caller using `transmit`.

```keikaku
sequence echo():
    cycle while true:
        msg := receive()
        yield "You said: " + text(msg)

gen := echo()
proceed(gen)  # Start generator
response := transmit(gen, "Hello")
declare(response)  # "You said: Hello"
```

## Generator Expressions (`( ... for ... )`)

Lazy sequences can be created inline using parentheses `()`.

```keikaku
squares := (x * x for x through [1, 2, 3, 4, 5])
cycle through squares as s:
    declare(s)

evens := (x for x through [1, 2, 3, 4, 5, 6] where x % 2 == 0)
cycle through evens as e:
    declare(e)
```

## Exception Handling (`disrupt`)

You can inject exceptions into a running generator using `disrupt`.

```keikaku
sequence robust_task():
    cycle while true:
        attempt:
            yield "working"
        recover error:
            yield "Recovered from: " + text(error)

gen := robust_task()
proceed(gen)
disrupt(gen, "Network Failure")
# Generator catches exception and continues
```
