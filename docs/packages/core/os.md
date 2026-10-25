<!-- dadoc 1.0.0-rc.8 -->
<!-- commit 6c56ce20508a (dirty) -->
# core:os

core:os — the process: its arguments, its environment, who and where it
is, and how it ends.

    i32 main(i32 argc, ^rawptr argv):
        for a in os.args(argc, argv):
            println(a)

The entry point's parameter list is fixed, because C's is: `main`
takes nothing, or `(i32 argc, ^rawptr argv)`. `^rawptr` is the honest Dado
spelling of `char **` — a pointer to pointers, to bytes whose end is a `\0`
rather than a type. `args` turns the pair into `[]string8` once.

**The package, in five files.** This one holds the seeded generator and
the exit statuses. `system.dado` holds the `string8` surface written per
system — `args`, `get_env`/`set_env`/`unset_env`, `pid`, `home_dir`,
`temp_dir`, `is_terminal`. `host.dado` holds the questions about the
machine and the user — `hostname`, `user_name`, `config_dir`, `cache_dir`,
`environment`, `cpu_count`, `random_bytes` — and Ctrl-C.
`executable.dado` finds the running program on the disk, and `errors.dado`
is `Error`, the code space every failable function in `os`, `os/fs`,
`os/proc` and `io` fails with, and `message`.

**Tier W H; N in part.** Hosted means Linux, macOS, Windows and the BSDs,
each with an arm of its own where the systems differ. `#WEB` builds every
function: a page or node hands a module no arguments (`args` is empty), no
environment this package can reach yet (`get_env` fails `NOT_FOUND`), no
terminal and no process id, so those answer empty and every function that
would have to ask a system fails with `UNSUPPORTED`, saying so; `quit_with`
hands the status to the host and `cpu_count` is the host's. **`#NONE` has no
process to describe**: it declares what is arithmetic — `Error` and
`message`, the generator, the exit statuses, `interrupted` — and nothing
that asks a system, so a call to one names the missing function. That is
deliberate, and why this package refuses `#NONE` by absence rather than
with a `#static_assert`: `core:io` fails with `Error`, and `core:fmt`,
which imports `core:io`, builds freestanding.

**What raises into a script.** The functions that are arithmetic over their
arguments — `message`, the generator and the exit statuses — carry
`#assert_raises`, checked at build time. Everything else asks the system
through a `foreign` call or reads `argv`, a `rawptr`, which is the C
boundary a script does not cross; each says so where it is written.

Thread-safety: what changes the environment belongs before threads start
(`system.dado` says why), and the generator's one state is not locked;
everything else is safe from any thread.

## Declarations

262 declarations, 35 public.

* `distinct enum i32 Error` — Why the system refused, in the numbers 1 to 99, the same on every system.
* `string8 message(Error why)` — A sentence for `why`, for a person: what went wrong and, where there is…
* `Error!void fail_with(Error why)` — Fail with `why`. **It always fails**: a caller writes `try os.fail_with(why)`
* `Error!void fail_last()` — Fail with the calling thread's most recent system failure, `last_error()`.
* `Error last_error()` — The calling thread's most recent system failure, as a code above. Read it…
* `Error from_system(i64 raw)` — A system's own error number as a code above: on Windows a Win32 error…
* `Error!string8 executable_path()` — Where the running program's executable is: an absolute path with every…
* `type EnvVar` — One variable of an `Environment`: views into the environment's own bytes,…
* `type Environment` — A copy of the whole environment: one block of bytes and one slice of views…
* `void Environment.destroy(self)` — Free the two blocks `environment` made. Safe on an empty environment.
* `Error!ref []char8 hostname()` — The name this machine goes by on its network, owned by the caller.
* `Error!ref []char8 user_name()` — The name of the user this process runs as, owned by the caller. Fails…
* `Error!ref []char8 config_dir()` — The directory this user's settings belong in, owned by the caller:
* `Error!ref []char8 cache_dir()` — The directory this user's caches belong in — what may be deleted and…
* `Environment environment()` — A copy of every environment variable, in the system's order. Empty on…
* `i32 cpu_count()` — How many processors this process may run on, at least 1.
* `Error!void random_bytes([]u8 into)` — Fill `into` with bytes from the system's cryptographic generator: the…
* `Error!void catch_interrupt()` — From now on, Ctrl-C does not end the program: it is recorded, and…
* `void release_interrupt()` — Put Ctrl-C back to ending the program. Nothing when it was not caught.
* `bool interrupted()` — Whether Ctrl-C arrived since `catch_interrupt`, or since this last answered…
* `void seed(i32 value)` — **A seeded generator, written in Dado** — not a good one and not a secure…
* `i32 random()` — The next value in `0..=random_max()`.
* `i32 random_max()` — The largest value `random` answers: 2147483647, on every system.
* `i32 random_below(i32 bound)` — A value in `0..<bound`, by the modulo everyone writes and nobody should…
* `void quit_with(i32 status)`
* `i32 success()` — The status that means success: 0, which is `EXIT_SUCCESS` on every C…
* `i32 failure()` — The status that means failure: 1, which is `EXIT_FAILURE` on every C library…
* `[]string8 args(i32 argc, ^rawptr argv)` — The process's arguments as UTF-8, the program first: made once, on the…
* `Error!ref []char8 get_env(string8 name)` — The value of the environment variable `name`, owned by the caller: a…
* `Error!void set_env(string8 name, string8 value)` — Set the environment variable `name` to `value`, replacing what was…
* `Error!void unset_env(string8 name)` — Remove the environment variable `name`. Removing one that is not set…
* `i64 pid()` — This process's id, as the system numbers it; 0 on `#WEB`, where there…
* `ref []char8 temp_dir()` — The directory for temporary files, owned by the caller, with no…
* `bool is_terminal(i32 stream)` — Whether standard stream `stream` — 0, 1 or 2, the standard input, output…
* `Error!ref []char8 home_dir()`
