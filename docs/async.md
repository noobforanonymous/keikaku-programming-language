# Asynchronous Programming in Keikaku

Keikaku supports asynchronous programming using `async` protocols, `await`, `promise` objects, and built-in concurrency primitives.

## `async` Protocols

Use `async protocol` or `async sequence` to define functions that can execute without blocking the main event loop.

```keikaku
async protocol fetch_user(id):
    yield "User " + text(id)

async protocol fetch_posts(user_id):
    sleep(100)  # Simulate delay
    yield "Post 1 for " + text(user_id)
```

## `await` Keyword

Use `await` to pause execution until a promise resolves or a generator yields.

```keikaku
async protocol main():
    user := await fetch_user(1)
    posts := await fetch_posts(user)
    declare(user, posts)

await main()
```

## Promise API (`resolve`)

Promises represent values that may be available in the future.

```keikaku
p := resolve("success")
result := await p
# result is "success"
```

## `defer` Execution

Schedule a function call for later execution using `defer`.

```keikaku
defer(500, declare, "Executed after 500ms")
```

## `sleep`

Pause execution for a specified duration in milliseconds.

```keikaku
sleep(1000)
declare("One second elapsed.")
```
