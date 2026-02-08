# Error Handling in Keikaku

Keikaku emphasizes structured control flow, treating errors as anticipated deviations in the plan. The language uses `attempt`/`recover` blocks for precise exception management.

## `attempt` Block

Use `attempt` to wrap code that might fail. If an exception occurs, execution jumps to the `recover` block.

```keikaku
attempt:
    result := divide(10, 0)
    declare(result)
recover error:
    declare("Deviation handled: ", error)
```

## `disrupt`

You can manually trigger exceptions within a generator context using `disrupt(gen, error)`.

```keikaku
sequence worker():
    cycle while true:
        attempt:
            yield "processing"
        recover task_error:
            yield "Error detected: " + text(task_error)

gen := worker()
proceed(gen)  # processing
disrupt(gen, "System overload")  # Error detected: System overload
```

## `anomaly` Block

For code that intentionally deviates from standard execution paths or for prototyping, use `anomaly`.

```keikaku
anomaly:
    declare("This block executes regardless of standard checks")
```
