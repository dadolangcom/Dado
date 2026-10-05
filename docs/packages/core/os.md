<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
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

170 declarations, 155 public.

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
* `#c.ulong GetLastError()`
* `const #c.long ERROR_FILE_NOT_FOUND`
* `const #c.long ERROR_PATH_NOT_FOUND`
* `const #c.long ERROR_INVALID_DRIVE`
* `const #c.long ERROR_BAD_NETPATH`
* `const #c.long ERROR_BAD_PATHNAME`
* `const #c.long ERROR_ENVVAR_NOT_FOUND`
* `const #c.long ERROR_ACCESS_DENIED`
* `const #c.long ERROR_ALREADY_EXISTS`
* `const #c.long ERROR_FILE_EXISTS`
* `const #c.long ERROR_DIRECTORY`
* `const #c.long ERROR_DIR_NOT_EMPTY`
* `const #c.long ERROR_INVALID_NAME`
* `const #c.long ERROR_INVALID_PARAMETER`
* `const #c.long ERROR_INVALID_HANDLE`
* `const #c.long ERROR_CRC`
* `const #c.long ERROR_READ_FAULT`
* `const #c.long ERROR_WRITE_FAULT`
* `const #c.long ERROR_DISK_FULL`
* `const #c.long ERROR_HANDLE_DISK_FULL`
* `const #c.long ERROR_SHARING_VIOLATION`
* `const #c.long ERROR_LOCK_VIOLATION`
* `const #c.long ERROR_NOT_SAME_DEVICE`
* `const #c.long ERROR_FILENAME_EXCED_RANGE`
* `const #c.long ERROR_CANT_RESOLVE_FILENAME`
* `const #c.long ERROR_WRITE_PROTECT`
* `const #c.long ERROR_NOT_ENOUGH_MEMORY`
* `const #c.long ERROR_OUTOFMEMORY`
* `const #c.long ERROR_TOO_MANY_OPEN_FILES`
* `const #c.long ERROR_PRIVILEGE_NOT_HELD`
* `const #c.long ERROR_NOT_SUPPORTED`
* `const #c.long ERROR_CALL_NOT_IMPLEMENTED`
* `const #c.long ERROR_OPERATION_ABORTED`
* `i32 from_system(i64 raw)` — A Win32 error number (what `GetLastError` answers) as a code above.
* `const i32 errno`
* `const i32 ENOENT`
* `const i32 EEXIST`
* `const i32 EACCES`
* `const i32 EPERM`
* `const i32 ENOTDIR`
* `const i32 EISDIR`
* `const i32 ENOTEMPTY`
* `const i32 EINVAL`
* `const i32 EBADF`
* `const i32 EIO`
* `const i32 ENOSPC`
* `const i32 EDQUOT`
* `const i32 EBUSY`
* `const i32 ETXTBSY`
* `const i32 EXDEV`
* `const i32 ENAMETOOLONG`
* `const i32 ELOOP`
* `const i32 EROFS`
* `const i32 ENOMEM`
* `const i32 EMFILE`
* `const i32 ENFILE`
* `const i32 ENOSYS`
* `const i32 ENOTSUP`
* `const i32 EOPNOTSUPP`
* `const i32 EINTR`
* `i32 from_system(i64 raw)` — An `errno` value as a code above.
* `type @incomplete @c("struct HINSTANCE__") ExeModule`
* `#c.ulong GetModuleFileNameW(^ExeModule module, ^u16 into, #c.ulong size)`
* `!string8 executable_path()` — The image's path. `GetModuleFileNameW` answers the count it wrote, and a…
* `#c.int _NSGetExecutablePath(^#c.char into, ^u32 size)`
* `^#c.char realpath(cstring path, ^#c.char resolved)`
* `void free(rawptr p)`
* `#c.size_t strlen(cstring s)`
* `!string8 executable_path()` — `_NSGetExecutablePath` answers -1 and the size it needs when the buffer…
* `#c.long readlink(cstring path, ^#c.char into, #c.size_t size)`
* `!string8 executable_path()` — `readlink` does not terminate what it writes and answers how much it…
* `i32 abs(i32 n)`
* `i32 rand()`
* `void srand(u32 seed)`
* `void exit(i32 status)`
* `cstring getenv(cstring name)`
* `const i32 RAND_MAX`
* `const i32 EXIT_SUCCESS`
* `const i32 EXIT_FAILURE`
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
* `const #c.uint CP_UTF8`
* `const #c.ulong STD_INPUT_HANDLE`
* `const #c.ulong STD_OUTPUT_HANDLE`
* `const #c.ulong STD_ERROR_HANDLE`
* `const #c.long ERROR_ENVVAR_NOT_FOUND`
* `#c.int MultiByteToWideChar(#c.uint code_page, #c.ulong flags, cstring from, #c.int from_count, ^u16 into, #c.int into_count)`
* `#c.int WideCharToMultiByte(#c.uint code_page, #c.ulong flags, ^const u16 from, #c.int from_count, ^#c.char into, #c.int into_count, cstring default_char, ^#c.int used_default)`
* `^u16 GetCommandLineW()`
* `rawptr LocalFree(rawptr block)`
* `#c.ulong GetEnvironmentVariableW(^const u16 name, ^u16 into, #c.ulong size)`
* `#c.int SetEnvironmentVariableW(^const u16 name, ^const u16 value)`
* `#c.ulong GetTempPathW(#c.ulong size, ^u16 into)`
* `#c.ulong GetCurrentProcessId()`
* `rawptr GetStdHandle(#c.ulong which)`
* `#c.int GetConsoleMode(rawptr console, ^#c.ulong mode)`
* `#c.ulong GetLastError()`
* `void SetLastError(#c.ulong code)`
* `^^u16 CommandLineToArgvW(^const u16 line, ^#c.int count)`
* `#c.int _wputenv_s(^const u16 name, ^const u16 value)`
* `#c.size_t wcslen(^const u16 s)`
* `[]string8 args(i32 argc, ^rawptr argv)` — The command line Windows kept, split by `CommandLineToArgvW` — the…
* `(ref []char8 value, bool found) get_env(string8 name)` — `GetEnvironmentVariableW` answers 0 both for a variable that is not set…
* `!void set_env(string8 name, string8 value)`
* `!void unset_env(string8 name)` — A null value deletes the variable. Deleting one that is not set is…
* `i64 pid()`
* `!ref []char8 home_dir()` — `USERPROFILE`, which Windows sets for every logged-on user.
* `ref []char8 temp_dir()` — `GetTempPathW`, which reads `TMP`, `TEMP` and `USERPROFILE` in that…
* `bool is_terminal(i32 stream)` — A console answers `GetConsoleMode`; a file, a pipe or `NUL` does not.
* `cstring getenv(cstring name)`
* `#c.int setenv(cstring name, cstring value, #c.int overwrite)`
* `#c.int unsetenv(cstring name)`
* `#c.int getpid()`
* `#c.int isatty(#c.int fd)`
* `[]string8 args(i32 argc, ^rawptr argv)` — `argv` as the process received it. The bytes are the process's for its…
* `(ref []char8 value, bool found) get_env(string8 name)`
* `!void set_env(string8 name, string8 value)`
* `!void unset_env(string8 name)`
* `i64 pid()`
* `!ref []char8 home_dir()` — `HOME`, which POSIX requires the login to set.
* `ref []char8 temp_dir()` — `TMPDIR` when it is set and not empty, otherwise `/tmp`. A trailing `/`
* `bool is_terminal(i32 stream)` — `stream` is a descriptor: 0, 1 and 2 are the standard input, output and…
