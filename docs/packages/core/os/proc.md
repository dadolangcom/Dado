<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:os/proc

core:os/proc — running another program, and reading what it says.

    import "core:os/proc"
    import "core:io"

    ^io.FILE p = proc.read("git rev-parse --short HEAD")
    (cstring line, bool ok) = io.read_line(p)
    (bool exited, i32 status) = proc.close(p)

The one thing this package is careful about: **`close` is where the exit
status is**, not `read`. A pipe that opened tells you the shell started, and
nothing else — a command that does not exist still opens, prints to stderr,
and closes with 127. Reading to the end and then closing is the whole
protocol, and it is the same one C has.

`command` is handed to `/bin/sh`, so it is a shell line and not an argv:
pipes, redirection and globs all work, and so does everything an attacker
would like to put in a filename.

**`exec.dado` beside this file is the argv form, and it is the one to reach
for unless the shell is the point.** This paragraph used to end *"there is no
`exec` form yet, because that needs an array of strings crossing into C and
`foreign` has no spelling for one"*, which was measured false: `rawptr`
crosses, at a stated cost written on the `execvp` declaration there.

What these three functions cost, measured on one value — `a b ; touch FILE ;`
— handed to `read` the way a caller assembling a command would: the space
split one argument into two, the `;` **ran `touch`**, and its output arrived
in the caller's own pipe. The same value with a `'` in it instead killed the
shell line with *Unterminated quoted string* and came back as `(true, 2)`,
which no caller can tell from a program that ran and exited 2. Nothing in
`collections/` escapes a shell argument and there is no helper here that
would; build the command yourself and know what is in it.

**`command` is a `cstring` everywhere below.** It exists only to reach
`popen` and `system`, both of which read to a `\0`, so the parameter says so
and the caller says how it got one: a literal — which is what a shell line
usually is — already is one, and a command a program assembled goes through
`strings.clone_to_cstring` and a written `cstring(…)`, which is where the
copy that assembling it needs becomes visible.

## Declarations

273 declarations, 193 public.

* `const i32 DRAINED = 0` — What `drain` answers when it did not read anything, told apart. A caller that…
* `const i32 WAITED = 0 - 1`
* `const i32 BROKE = 0 - 2`
* `i32 drain(^io.FILE stream, []char8 into, i32 timeout_ms = 0)`
* `i32 fileno(^io.FILE stream)`
* `#c.long read(i32 fd, rawptr buf, #c.size_t count)`
* `type PollFd: (i32 fd, i16 events, i16 revents)`
* `i32 poll(^PollFd fds, u32 nfds, i32 timeout)`
* `type PollFd: (i32 fd, i16 events, i16 revents)`
* `i32 poll(^PollFd fds, u64 nfds, i32 timeout)`
* `i32 drain(^io.FILE stream, []char8 into, i32 timeout_ms = 0)` — Read whatever `stream` has written, waiting at most `timeout_ms`.
* `const i32 ERRORS_INHERIT = 0` — What `duplex` does with the child's standard error.
* `const i32 ERRORS_PIPE = 1`
* `const i32 ERRORS_MERGE = 2`
* `const i32 ERRORS_DISCARD = 3`
* `type Duplex` — A running child and the three pipes to it. Fields are readable; change them…
* `bool started(Duplex d)` — Whether `duplex` started a child. A `Duplex` that did not holds no…
* `bool ignoring_sigpipe()` — Whether the first `duplex` in this process set `SIGPIPE` to ignored — false…
* `(i64 sent, i32 stopped) send_all(^Duplex d, []char8 bytes, i32 timeout_ms = 0)` — `send` until all of `bytes` is gone, or nothing moved for `timeout_ms`.
* `(bool exited, i32 status) stop(^Duplex d, i32 grace_ms)` — The whole ending in one call: close every pipe, give the child `grace_ms` to…
* `Duplex duplex([]cstring argv, i32 errors = ERRORS_INHERIT)`
* `i32 send(^Duplex d, []char8 bytes, i32 timeout_ms = 0)`
* `i32 receive(^Duplex d, []char8 into, i32 timeout_ms = 0)`
* `i32 receive_errors(^Duplex d, []char8 into, i32 timeout_ms = 0)`
* `bool close_input(^Duplex d)`
* `bool terminate(^Duplex d)`
* `bool kill(^Duplex d)`
* `(bool done, bool exited, i32 status) finished(^Duplex d)`
* `(bool done, bool exited, i32 status) finish(^Duplex d, i32 timeout_ms)`
* `void release(^Duplex d)`
* `#c.long write(i32 fd, rawptr buf, #c.size_t count)`
* `const i32 F_GETFL`
* `const i32 F_SETFL`
* `const i32 F_SETFD`
* `const i32 F_DUPFD_CLOEXEC`
* `const i32 FD_CLOEXEC`
* `const i32 O_NONBLOCK`
* `const i32 O_WRONLY`
* `const i32 O_CLOEXEC`
* `i32 fcntl(i32 fd, i32 cmd, ...)`
* `i32 open(cstring path, i32 flags, ...)`
* `type posix_spawn_file_actions_t`
* `type posix_spawnattr_t`
* `const i32 POSIX_SPAWN_SETSIGDEF`
* `i32 posix_spawn_file_actions_init(^posix_spawn_file_actions_t fa)`
* `i32 posix_spawn_file_actions_destroy(^posix_spawn_file_actions_t fa)`
* `i32 posix_spawn_file_actions_adddup2(^posix_spawn_file_actions_t fa, i32 fd, i32 to)`
* `i32 posix_spawnattr_init(^posix_spawnattr_t attr)`
* `i32 posix_spawnattr_destroy(^posix_spawnattr_t attr)`
* `i32 posix_spawnattr_setflags(^posix_spawnattr_t attr, i16 flags)`
* `i32 posix_spawnattr_setsigdefault(^posix_spawnattr_t attr, ^SigSet set)`
* `i32 posix_spawnp(^i32 pid, cstring file, ^posix_spawn_file_actions_t fa, ^posix_spawnattr_t attr, ^const ^#c.char argv, ^const ^#c.char envp)`
* `^^^#c.char _NSGetEnviron()`
* `type @c("sigset_t") SigSet`
* `@macro i32 sigemptyset(^SigSet set)`
* `@macro i32 sigaddset(^SigSet set, i32 signum)`
* `const rawptr environ`
* `type @c("sigset_t") SigSet`
* `i32 sigemptyset(^SigSet set)`
* `i32 sigaddset(^SigSet set, i32 signum)`
* `const i32 errno`
* `const i32 EINTR`
* `const i32 EAGAIN`
* `const i32 WNOHANG`
* `const i32 SIGPIPE`
* `const i32 SIGTERM`
* `const i32 SIGKILL`
* `const SigHandler SIG_IGN`
* `const SigHandler SIG_DFL`
* `SigHandler signal(i32 signum, SigHandler handler)`
* `i32 kill(i32 pid, i32 signum)`
* `const i16 POLLOUT`
* `Duplex duplex([]cstring argv, i32 errors = ERRORS_INHERIT)` — Start `argv` with its stdin and stdout on pipes to this process, and its…
* `i32 send(^Duplex d, []char8 bytes, i32 timeout_ms = 0)` — Write as much of `bytes` as the pipe will take, waiting at most `timeout_ms`
* `i32 receive(^Duplex d, []char8 into, i32 timeout_ms = 0)` — Whatever the child has written to stdout, waiting at most `timeout_ms`:
* `i32 receive_errors(^Duplex d, []char8 into, i32 timeout_ms = 0)` — `receive` for the stderr pipe, which exists only under `ERRORS_PIPE`;…
* `bool close_input(^Duplex d)` — Close the child's stdin, so its next read sees end-of-file. True if this…
* `bool terminate(^Duplex d)` — Ask the child to stop (`SIGTERM`). False for a child already reaped — its…
* `bool kill(^Duplex d)` — Stop the child whether it agrees or not (`SIGKILL`). Same refusals.
* `(bool done, bool exited, i32 status) finished(^Duplex d)` — Whether the child has ended, without waiting. `done` false is "still…
* `(bool done, bool exited, i32 status) finish(^Duplex d, i32 timeout_ms)` — `finished`, waiting up to `timeout_ms` for it (negative waits indefinitely).
* `void release(^Duplex d)` — Close every descriptor the parent still holds, without waiting. The child…
* `const i32 ARG_CAP = 255` — **The argv this package will assemble, at most.** C wants a null after the…
* `type Child` — A running program and the pipe its standard output arrives on.
* `bool spawned(Child c)` — Whether a `spawn` started anything.
* `(bool exited, i32 status) exec([]cstring argv)`
* `Child spawn([]cstring argv)`
* `(bool exited, i32 status) wait(Child c)`
* `i32 execvp(cstring file, ^const ^#c.char argv)`
* `i32 fork()`
* `i32 pipe(^i32 fds)`
* `i32 dup2(i32 old, i32 new)`
* `i32 close(i32 fd)`
* `void _exit(i32 code)`
* `i32 waitpid(i32 pid, ^i32 status, i32 options)`
* `^io.FILE fdopen(i32 fd, cstring mode)`
* `(bool exited, i32 status) exec([]cstring argv)` — Run `argv` to completion with the caller's own streams, so its output goes…
* `Child spawn([]cstring argv)` — Start `argv` and read its standard output. The `read` twin.
* `(bool exited, i32 status) wait(Child c)` — Close the pipe and wait for the program. The `close` twin, and the same…
* `i32 system(cstring command)`
* `bool opened(^io.FILE stream)`
* `bool available()` — Whether there is a shell to run any of this with — C99's `system(NULL)`.
* `bool ok(bool exited, i32 status)` — Whether a `(exited, status)` pair means "it worked": ended by returning, and…
* `^io.FILE read(cstring command)` — Start `command` and read its standard output; nil when the pipe or the…
* `^io.FILE write(cstring command)` — Start `command` and write to its standard input.
* `(bool exited, i32 status) close(^io.FILE stream)` — Close the pipe and wait for the command. `(false, -1)` for a stream…
* `(bool exited, i32 status) run(cstring command)` — Run `command` to completion with the caller's own streams.
* `^io.FILE popen(cstring command, cstring mode)`
* `i32 pclose(^io.FILE stream)`
* `@macro i32 WIFEXITED(i32 status)`
* `@macro i32 WIFSIGNALED(i32 status)`
* `@macro i32 WEXITSTATUS(i32 status)`
* `@macro i32 WTERMSIG(i32 status)`
* `^io.FILE read(cstring command)` — Start `command` and read its standard output. `nil` when the pipe or the…
* `^io.FILE write(cstring command)` — Start `command` and write to its standard input.
* `(bool exited, i32 status) close(^io.FILE stream)` — Wait for the command and report how it ended.
* `(bool exited, i32 status) run(cstring command)` — Run `command` to completion with the caller's own streams — so its output…
* `type SECURITY_ATTRIBUTES: (#c.ulong nLength, rawptr lpSecurityDescriptor, #c.int bInheritHandle)`
* `type @incomplete @c("struct _PROC_THREAD_ATTRIBUTE_LIST") ProcThreadAttributeList`
* `type STARTUPINFOW`
* `type STARTUPINFOEXW: (STARTUPINFOW StartupInfo, ^ProcThreadAttributeList lpAttributeList)`
* `type PROCESS_INFORMATION: (rawptr hProcess, rawptr hThread, #c.ulong dwProcessId, #c.ulong dwThreadId)`
* `type HANDLE` — `OVERLAPPED`'s third member is an anonymous union of an offset pair…
* `type OVERLAPPED: (#c.ullong Internal, #c.ullong InternalHigh, rawptr Pointer, rawptr hEvent)`
* `const rawptr INVALID_HANDLE_VALUE`
* `const #c.ulong STD_INPUT_HANDLE`
* `const #c.ulong STD_OUTPUT_HANDLE`
* `const #c.ulong STD_ERROR_HANDLE`
* `const #c.ulong INFINITE`
* `const #c.ulong WAIT_OBJECT_0`
* `const #c.ulong CREATE_SUSPENDED`
* `const #c.ulong EXTENDED_STARTUPINFO_PRESENT`
* `const #c.ulong STARTF_USESTDHANDLES`
* `const #c.ulong DUPLICATE_SAME_ACCESS`
* `const #c.ulong STILL_ACTIVE`
* `const #c.ulong PIPE_ACCESS_OUTBOUND`
* `const #c.ulong FILE_FLAG_OVERLAPPED`
* `const #c.ulong FILE_FLAG_FIRST_PIPE_INSTANCE`
* `const #c.ulong PIPE_TYPE_BYTE`
* `const #c.ulong PIPE_READMODE_BYTE`
* `const #c.ulong PIPE_WAIT`
* `const #c.ulong PIPE_REJECT_REMOTE_CLIENTS`
* `const #c.ulong GENERIC_READ`
* `const #c.ulong GENERIC_WRITE`
* `const #c.ulong FILE_SHARE_READ`
* `const #c.ulong FILE_SHARE_WRITE`
* `const #c.ulong OPEN_EXISTING`
* `const #c.uint CP_UTF8`
* `rawptr GetStdHandle(#c.ulong which)`
* `rawptr GetCurrentProcess()`
* `#c.ulong GetCurrentProcessId()`
* `#c.ulong GetLastError()`
* `#c.int CloseHandle(rawptr handle)`
* `#c.int DuplicateHandle(rawptr from_process, rawptr from, rawptr to_process, ^rawptr to, #c.ulong access, #c.int inherit, #c.ulong options)`
* `#c.int CreatePipe(^rawptr read_end, ^rawptr write_end, ^SECURITY_ATTRIBUTES attributes, #c.ulong size)`
* `rawptr CreateNamedPipeW(^const u16 name, #c.ulong open_mode, #c.ulong pipe_mode, #c.ulong instances, #c.ulong out_size, #c.ulong in_size, #c.ulong timeout, ^SECURITY_ATTRIBUTES attributes)`
* `rawptr CreateFileW(^const u16 name, #c.ulong access, #c.ulong share, ^SECURITY_ATTRIBUTES attributes, #c.ulong disposition, #c.ulong flags, rawptr template)`
* `rawptr CreateEventW(^SECURITY_ATTRIBUTES attributes, #c.int manual, #c.int initial, ^const u16 name)`
* `#c.int InitializeProcThreadAttributeList(^ProcThreadAttributeList list, #c.ulong count, #c.ulong flags, ^#c.size_t size)`
* `#c.int UpdateProcThreadAttribute(^ProcThreadAttributeList list, #c.ulong flags, #c.ullong attribute, rawptr value, #c.size_t size, rawptr previous, ^#c.size_t returned)`
* `void DeleteProcThreadAttributeList(^ProcThreadAttributeList list)`
* `#c.int CreateProcessW(^const u16 application, ^u16 command_line, ^SECURITY_ATTRIBUTES process, ^SECURITY_ATTRIBUTES thread, #c.int inherit, #c.ulong flags, rawptr environment, ^const u16 directory, ^STARTUPINFOW startup, ^PROCESS_INFORMATION info)`
* `#c.ulong ResumeThread(rawptr thread)`
* `rawptr CreateJobObjectW(^SECURITY_ATTRIBUTES attributes, ^const u16 name)`
* `#c.int AssignProcessToJobObject(rawptr job, rawptr process)`
* `#c.int TerminateJobObject(rawptr job, #c.uint code)`
* `#c.int TerminateProcess(rawptr process, #c.uint code)`
* `#c.ulong WaitForSingleObject(rawptr handle, #c.ulong milliseconds)`
* `#c.ulong WaitForMultipleObjects(#c.ulong count, ^const HANDLE handles, #c.int all, #c.ulong milliseconds)`
* `#c.int GetExitCodeProcess(rawptr process, ^#c.ulong code)`
* `#c.int PeekNamedPipe(rawptr pipe, rawptr buffer, #c.ulong size, ^#c.ulong read, ^#c.ulong available, ^#c.ulong left)`
* `#c.int ReadFile(rawptr file, rawptr buffer, #c.ulong count, ^#c.ulong read, ^OVERLAPPED overlapped)`
* `#c.int WriteFile(rawptr file, const rawptr buffer, #c.ulong count, ^#c.ulong written, ^OVERLAPPED overlapped)`
* `#c.int CancelIoEx(rawptr file, ^OVERLAPPED overlapped)`
* `#c.int GetOverlappedResult(rawptr file, ^OVERLAPPED overlapped, ^#c.ulong transferred, #c.int wait)`
* `#c.int MultiByteToWideChar(#c.uint code_page, #c.ulong flags, cstring from, #c.int from_count, ^u16 into, #c.int into_count)`
* `#c.ulong GetEnvironmentVariableW(^const u16 name, ^u16 into, #c.ulong size)`
* `void Sleep(#c.ulong milliseconds)`
* `const #c.int PROC_THREAD_ATTRIBUTE_HANDLE_LIST`
* `const #c.long ERROR_FILE_NOT_FOUND`
* `const #c.long ERROR_PATH_NOT_FOUND`
* `const #c.long ERROR_BROKEN_PIPE`
* `const #c.long ERROR_NO_DATA`
* `const #c.long ERROR_IO_PENDING`
* `const #c.long ERROR_INVALID_PARAMETER`
* `const #c.long ERROR_TOO_MANY_OPEN_FILES`
* `const #c.long WAIT_TIMEOUT`
* `^io.FILE _fdopen(i32 fd, cstring mode)`
* `i32 _fileno(^io.FILE stream)`
* `#c.intptr_t _get_osfhandle(i32 fd)`
* `i32 _open_osfhandle(#c.intptr_t handle, i32 flags)`
* `i32 _close(i32 fd)`
* `ref []char8 command_line_utf8([]cstring argv)` — The UTF-8 command line `argv` quotes to: the program, then each…
