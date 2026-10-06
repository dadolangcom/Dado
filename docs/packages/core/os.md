<!-- dadoc 1.0.0-rc.2 -->
<!-- commit 7e1d5137cae1 (dirty) -->
# core:os

std/os — the process, and the arguments it was started with.

    i32 main(i32 argc, ^rawptr argv):
        for i in 0..<argc:
            println(i, ": ", os.arg(argv, i))

The entry point's parameter list is fixed, because C's is: `main`
takes nothing, or `(i32 argc, ^rawptr argv)`. `^rawptr` is the honest Dado
spelling of `char **` — a pointer to pointers, to bytes whose end is a `\0`
rather than a type.

Reading `argv[i]` is pointer arithmetic, which Dado does not have. So it
happens in `args.c` beside this file, which the toolchain compiles and links
because it is in the package directory. (That was true once; the comment
on `arg` below says what replaced it.)

**The package, in three files.** This one holds the `cstring` surface the
tree has used since before `string8`: `arg`, `all`, `env`, `has_env`, the
seeded generator and `quit_with`. `system.dado` holds the `string8` surface
written per system — `args`, `get_env`/`set_env`/`unset_env`, `pid`,
`home_dir`, `temp_dir`, `is_terminal` — and `errors.dado` the error code space
every failable function in `os` and `os/fs` fails with, and `message`.

Tier: hosted — Linux, macOS and Windows, each with its own arm where the
systems differ. Thread-safety: what changes the environment belongs before
threads start (`system.dado` says why); everything else is safe from any
thread.

## Declarations

170 declarations, 56 public.

* `const i32 NOT_FOUND = 1` — Nothing is at the path, or a directory on the way to it is missing.
* `const i32 EXISTS = 2` — Something is already at the path, and the operation does not replace it.
* `const i32 PERMISSION = 3` — The system refused: the caller may not read, write, enter or change this.
* `const i32 NOT_DIR = 4` — A component of the path that has to be a directory is not one.
* `const i32 IS_DIR = 5` — The path is a directory and the operation wanted a file.
* `const i32 NOT_EMPTY = 6` — The directory has entries, and the operation needs it empty.
* `const i32 INVALID = 7` — The argument is not one the operation can take: an empty name, a name with a…
* `const i32 IO = 8` — The device failed to read or write.
* `const i32 NO_SPACE = 9` — The disk, or the user's quota on it, is full.
* `const i32 BUSY = 10` — Something else holds the file: it is in use, locked, or being executed.
* `const i32 CROSS_DEVICE = 11` — A rename between two filesystems, which moves nothing.
* `const i32 TOO_LONG = 12` — The path, or one of its components, is longer than the system allows.
* `const i32 LOOP = 13` — Symbolic links loop, or are nested deeper than the system will follow.
* `const i32 READ_ONLY = 14` — The filesystem is mounted read-only.
* `const i32 NO_MEMORY = 15` — The system, or this process, ran out of memory.
* `const i32 TOO_MANY_OPEN = 16` — The process, or the whole system, has too many files open.
* `const i32 PRIVILEGE = 17` — The operation needs a privilege the process does not hold. On Windows,…
* `const i32 UNSUPPORTED = 18` — The system or the filesystem does not do this at all.
* `const i32 INTERRUPTED = 19` — A signal interrupted the call before it finished.
* `const i32 UNKNOWN = 99` — The system failed with a reason this table does not name.
* `string8 message(i32 code)` — A sentence for `code`, for a person: what went wrong and, where there is…
* `i32 last_error()` — The calling thread's most recent system failure, as a code above. Read it…
* `i32 from_system(i64 raw)` — A Win32 error number (what `GetLastError` answers) as a code above.
* `i32 from_system(i64 raw)` — An `errno` value as a code above.
* `!string8 executable_path()` — The image's path. `GetModuleFileNameW` answers the count it wrote, and a…
* `!string8 executable_path()` — `_NSGetExecutablePath` answers -1 and the size it needs when the buffer…
* `!string8 executable_path()` — `readlink` does not terminate what it writes and answers how much it…
* `cstring arg(^rawptr argv, i32 index)` — The argument at `index`, exactly as C handed it over.
* `[]rawptr all(^rawptr argv, i32 argc)` — The whole argument vector, as a slice, windowed once.
* `bool has_arg(i32 argc, i32 index)` — Whether `argv` carries an argument at `index` — i.e. whether `index` is…
* `cstring env(cstring name)` — The value of an environment variable, or `nil` when it is unset.
* `bool has_env(cstring name)` — Whether a variable is set. `getenv(name) != nil`, which is C's own question…
* `i32 magnitude(i32 n)`
* `void seed(i32 value)` — C's generator, seeded explicitly. Deterministic for a given seed, which is…
* `i32 random()`
* `i32 random_below(i32 bound)` — A value in `0..<bound`, by the modulo everyone writes and nobody should…
* `i32 random_max()`
* `void quit_with(i32 status)`
* `i32 success()`
* `i32 failure()`
* `[]string8 args(i32 argc, ^rawptr argv)` — The command line Windows kept, split by `CommandLineToArgvW` — the…
* `(ref []char8 value, bool found) get_env(string8 name)` — `GetEnvironmentVariableW` answers 0 both for a variable that is not set…
* `!void set_env(string8 name, string8 value)`
* `!void unset_env(string8 name)` — A null value deletes the variable. Deleting one that is not set is…
* `i64 pid()`
* `!ref []char8 home_dir()` — `USERPROFILE`, which Windows sets for every logged-on user.
* `ref []char8 temp_dir()` — `GetTempPathW`, which reads `TMP`, `TEMP` and `USERPROFILE` in that…
* `bool is_terminal(i32 stream)` — A console answers `GetConsoleMode`; a file, a pipe or `NUL` does not.
* `[]string8 args(i32 argc, ^rawptr argv)` — `argv` as the process received it. The bytes are the process's for its…
* `(ref []char8 value, bool found) get_env(string8 name)`
* `!void set_env(string8 name, string8 value)`
* `!void unset_env(string8 name)`
* `i64 pid()`
* `!ref []char8 home_dir()` — `HOME`, which POSIX requires the login to set.
* `ref []char8 temp_dir()` — `TMPDIR` when it is set and not empty, otherwise `/tmp`. A trailing `/`
* `bool is_terminal(i32 stream)` — `stream` is a descriptor: 0, 1 and 2 are the standard input, output and…
