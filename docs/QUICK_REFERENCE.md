# Keikaku Quick Reference Card

```
╔══════════════════════════════════════════════════════════════════════════════╗
║                        K E I K A K U   v1.0.0                                ║
║                  Quick Reference - "All according to plan"                   ║
╚══════════════════════════════════════════════════════════════════════════════╝

┌─────────────────────────────────────────────────────────────────────────────┐
│ VARIABLES                                                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│   designate x = 10          # Full declaration                              │
│   x := 10                   # Shorthand (preferred)                         │
│   x = 20                    # Reassignment                                  │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│ DATA TYPES                                                                   │
├─────────────────────────────────────────────────────────────────────────────┤
│   42                        # Integer                                       │
│   3.14                      # Float                                         │
│   "hello"                   # String                                        │
│   true / false              # Boolean                                       │
│   [1, 2, 3]                 # List                                          │
│   list[0]                   # Index access                                  │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│ CONDITIONALS                                                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│   foresee condition:        # if                                            │
│       do_something()                                                        │
│   alternate condition2:     # else if                                       │
│       do_other()                                                            │
│   otherwise:                # else                                          │
│       do_default()                                                          │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│ LOOPS                                                                        │
├─────────────────────────────────────────────────────────────────────────────┤
│   cycle while condition:    # while loop                                    │
│       do_something()                                                        │
│                                                                             │
│   cycle through items as x: # for-each loop                                 │
│       declare(x)                                                            │
│                                                                             │
│   cycle from 1 to 10 as i:  # range loop (1 to 9)                           │
│       declare(i)                                                            │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│ FUNCTIONS (PROTOCOLS)                                                        │
├─────────────────────────────────────────────────────────────────────────────┤
│   protocol name(param1, param2 := default):                                 │
│       do_something()                                                        │
│       yield result          # return value                                  │
│                                                                             │
│   result := name(arg1, arg2)  # function call                               │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│ OPERATORS                                                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│   +  -  *  /               # Arithmetic                                     │
│   //                       # Integer division                               │
│   %                        # Modulo                                         │
│   **                       # Power                                          │
│   ==  !=  <  <=  >  >=     # Comparison                                     │
│   and  or  not             # Logical                                        │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│ SPECIAL CONSTRUCTS                                                           │
├─────────────────────────────────────────────────────────────────────────────┤
│   scheme:                   # Deferred execution block                      │
│       code_here()                                                           │
│   execute                                                                   │
│                                                                             │
│   preview expression        # Evaluate without side effects                 │
│                                                                             │
│   override var = value      # Force assignment (bypass scope)               │
│                                                                             │
│   absolute condition        # Strong assertion                              │
│                                                                             │
│   anomaly:                  # Mark experimental code                        │
│       risky_code()                                                          │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│ GENERATORS (SEQUENCES)                                                       │
├─────────────────────────────────────────────────────────────────────────────┤
│   sequence name(args):      # Define generator                              │
│       yield value           # Yield value and pause                         │
│       delegate other()      # Yield all from other                          │
│                                                                             │
│   gen := name(args)                                                         │
│   val := proceed(gen)       # Get next value                                │
│   val := transmit(gen, x)   # Send value to generator                       │
│   val := receive()          # Receive sent value (inside gen)               │
│   disrupt(gen, "error")     # Throw error into generator                    │
│                                                                             │
│   (x*2 for x through list)  # Generator expression                          │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│ ASYNC / AWAIT                                                                │
├─────────────────────────────────────────────────────────────────────────────┤
│   async protocol name():    # Define async function                         │
│       await promise         # Pause until resolved                          │
│                                                                             │
│   p := resolve(value)       # Create resolved promise                       │
│   sleep(ms)                 # Pause execution                               │
│   defer(ms, func, args)     # Schedule execution                            │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│ ERROR HANDLING                                                               │
├─────────────────────────────────────────────────────────────────────────────┤
│   attempt:                  # Try block                                     │
│       risky_code()                                                          │
│   recover error:            # Catch block                                   │
│       handle(error)                                                         │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│ BUILT-IN FUNCTIONS                                                           │
├─────────────────────────────────────────────────────────────────────────────┤
│   declare(...)             # Print output                                   │
│   inquire("prompt")        # Read input                                     │
│   measure(x)               # Length of string/list                          │
│   span(n) / span(a, b)     # Create range list                              │
│   text(x)                  # Convert to string                              │
│   number(x)                # Convert to integer                             │
│   decimal(x)               # Convert to float                               │
│   boolean(x)               # Convert to boolean                             │
│   classify(x)              # Get type name                                  │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│ USAGE                                                                        │
├─────────────────────────────────────────────────────────────────────────────┤
│   keikaku                   # Start REPL                                    │
│   keikaku file.kei          # Run a script                                  │
│   keikaku --help            # Show help                                     │
│   keikaku --version         # Show version                                  │
└─────────────────────────────────────────────────────────────────────────────┘

                    "Everything proceeds according to keikaku."
```
