<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:os/fs

core:os/fs — what is on the disk.

    import "core:os/fs"

    ^fs.Dir d = fs.open(".")
    defer fs.close(d)
    for:
        (cstring name, bool ok) = fs.next(d)
        if !ok: break
        println(name, "  dir=", fs.is_dir(name))

**The package, in four files.** This one asks questions of a path and walks
a directory, over `cstring` paths and with `bool` answers. `files.dado` reads
and writes whole files and changes the tree — remove, rename, copy, `mtime`,
temp directories, links — over `string8` paths, failing with `core:os`'s
codes. `walk.dado` walks and makes whole trees. `windows.dado` holds what the
Windows arms share.

Tier: hosted — Linux, macOS and Windows, each with an arm of its own. The
error codes are `core:os`'s. Thread-safety: `cwd` answers in this package's
own buffer, so two threads asking at once overwrite each other; everything
else is safe from any thread.

**Every path in this file is a `cstring`**, and that is the boundary at a signature.
A path exists in this package only to be handed to `<dirent.h>` or to a
`stat`, so the parameter says what the platform needs and the caller says how
it got one: a literal already is one, and bytes a program built become one
through `strings.clone_to_cstring` and a `cstring(…)` at the call. A name
coming *back* — from `next` or `cwd` — is a `cstring` for the mirror reason:
C hands over a pointer and threw the length away, and there is no conversion
into a `string` to hide the walk that finds it again.

Everything here is a question about a path, answered by the platform. There
is no path *algebra* — no join, no basename, no normalise — because those
return new bytes and there is nowhere to build them that outlives the block.
Use `format` at the point of use; it is one line and the bytes
live exactly as long as you need them.

**This package has no C of its own.** On POSIX, `<dirent.h>`, `<sys/stat.h>`,
`<stdlib.h>`, `<string.h>` and `<unistd.h>` are bound directly and the bodies
below are the whole implementation; what that costs, and what it buys, is
written beside `Stat`. On Windows, `<windows.h>` is, through `windows.dado`.

## Declarations

233 declarations, 183 public.

* `!ref []char8 read_file(string8 path)` — The whole file. Opened with every sharing mode, so a file another…
* `!void write_file(string8 path, string8 data)` — Written to a new file beside `path`, flushed to the disk, and moved over…
* `!void remove_file(string8 path)` — A read-only file refuses `DeleteFileW`, which POSIX's `unlink` never…
* `!void remove_dir(string8 path)`
* `!void remove_all(string8 path)` — A link — symbolic, or a junction — is removed as itself, never entered:
* `!void rename(string8 from, string8 to)` — `MoveFileExW` with `MOVEFILE_REPLACE_EXISTING`: an existing `to` is…
* `!void copy_file(string8 from, string8 to)` — `CopyFileW`, replacing `to`; it copies the attributes too.
* `!i64 mtime(string8 path)`
* `!ref []char8 make_temp_dir(string8 prefix)` — A new directory under `os.temp_dir()`, named `prefix` and sixteen hex…
* `!void make_symlink(string8 target, string8 path)` — `CreateSymbolicLinkW`, with…
* `const i32 O_RDONLY`
* `const i32 O_WRONLY`
* `const i32 O_CREAT`
* `const i32 O_EXCL`
* `const i32 O_TRUNC`
* `const i32 O_CLOEXEC`
* `i32 open(cstring path, i32 flags, ...)`
* `#c.long read(i32 fd, rawptr buf, #c.size_t count)`
* `#c.long write(i32 fd, const rawptr buf, #c.size_t count)`
* `#c.int close(i32 fd)`
* `#c.int fsync(i32 fd)`
* `#c.int unlink(cstring path)`
* `#c.int rmdir(cstring path)`
* `#c.int rename(cstring from, cstring to)`
* `^#c.char mkdtemp(^#c.char template)`
* `!ref []char8 read_file(string8 path)` — The whole file. A directory opens for reading on Linux and fails at the…
* `!void write_file(string8 path, string8 data)` — Written to a new file beside `path` (`O_CREAT | O_EXCL`, so it is never…
* `!void remove_file(string8 path)`
* `!void remove_dir(string8 path)`
* `!void remove_all(string8 path)` — `lstat` decides, so a symbolic link is `unlink`ed as itself and never…
* `!void rename(string8 from, string8 to)` — `rename(2)`: an existing `to` is replaced atomically; across…
* `!void copy_file(string8 from, string8 to)` — The bytes, into `to` created or truncated, with `from`'s permission…
* `!i64 mtime(string8 path)`
* `!ref []char8 make_temp_dir(string8 prefix)` — `mkdtemp`, which makes the directory `0700` and never over anything…
* `!void make_symlink(string8 target, string8 path)` — `symlink(2)`, which needs no privilege on POSIX.
* `bool hidden(string8 name)` — Whether a name is `.`, `..`, or begins with a dot. The three a listing…
* `type Dir: (rawptr search, bool pending, FsFindData found, [1024]char8 name)` — The search a directory listing is: `FindFirstFileW` answers the first…
* `^Dir open(cstring path)`
* `bool opened(^Dir dir)`
* `void close(^Dir dir)`
* `(cstring name, bool ok) next(^Dir dir)` — The name is converted into the record's own buffer, so it is good until…
* `bool is_dir(cstring path)` — Following links, as `stat` does on POSIX.
* `bool is_file(cstring path)`
* `bool exists(cstring path)`
* `i64 bytes(cstring path)`
* `i64 real_path(cstring path, []char8 out)` — `GetFinalPathNameByHandleW` over a handle that followed every link: the…
* `bool symlink(cstring target, cstring path)` — `CreateSymbolicLinkW`, through `make_symlink`, which says why it failed.
* `i64 read_link(cstring path, []char8 out)`
* `bool is_symlink(cstring path)`
* `bool executable(cstring path)` — **Windows has no execute bit**: whether a file runs is decided by its…
* `bool make_executable(cstring path)` — Nothing to set: with no execute bit, a file that exists is as runnable…
* `cstring cwd()`
* `bool set_cwd(cstring path)`
* `bool make_dir(cstring path)`
* `bool make_private_dir(cstring path)` — Owner and `SYSTEM` only, through a protected DACL (`windows.dado`), which…
* `type Stat`
* `#c.int stat(cstring path, ^Stat buf)`
* `#c.int lstat(cstring path, ^Stat buf)`
* `@macro #c.int S_ISDIR(u16 mode)`
* `@macro #c.int S_ISREG(u16 mode)`
* `@macro #c.int S_ISLNK(u16 mode)`
* `type Stat`
* `#c.int stat(cstring path, ^Stat buf)`
* `#c.int lstat(cstring path, ^Stat buf)`
* `@macro #c.int S_ISDIR(u32 mode)`
* `@macro #c.int S_ISREG(u32 mode)`
* `@macro #c.int S_ISLNK(u32 mode)`
* `#c.int mkdir(cstring path, #c.ushort mode)`
* `#c.int chmod(cstring path, #c.ushort mode)`
* `#c.int mkdir(cstring path, #c.uint mode)`
* `#c.int chmod(cstring path, #c.uint mode)`
* `type DIR`
* `^DIR opendir(cstring path)`
* `#c.int closedir(^DIR d)`
* `type Dirent`
* `^Dirent readdir(^DIR d)`
* `type Dirent`
* `^Dirent readdir(^DIR d)`
* `^#c.char realpath(cstring path, ^#c.char resolved)` — **`realpath(path, nil)`, not a caller-sized buffer.** POSIX.1-2008 makes…
* `void free(rawptr p)`
* `#c.size_t strlen(cstring s)`
* `@macro rawptr memcpy(rawptr dst, rawptr src, #c.size_t nbytes)`
* `^#c.char getcwd(^#c.char buf, #c.size_t nbytes)`
* `#c.int chdir(cstring path)`
* `#c.int symlink(cstring target, cstring path)`
* `#c.long readlink(cstring path, ^#c.char buf, #c.size_t nbytes)`
* `#c.int access(cstring path, i32 mode)` — **`access` and not a mode read**, because *"can this be executed"* is a…
* `const i32 X_OK`
* `type Dir: DIR` — The handle, under the name a program writes. A `type` is a view and creates…
* `^Dir open(cstring path)` — **A `nil` path is answered, not passed on.** Every function here that takes a…
* `bool opened(^Dir dir)`
* `void close(^Dir dir)`
* `(cstring name, bool ok) next(^Dir dir)` — The next entry's name, and whether there was one.
* `bool is_dir(cstring path)` — `stat` follows symlinks, which is what a program listing a tree almost always…
* `bool is_file(cstring path)`
* `bool exists(cstring path)`
* `i64 bytes(cstring path)` — Bytes, or -1 when the path cannot be read. `i64`, because a file may be…
* `i64 real_path(cstring path, []char8 out)` — Resolve `path` to its canonical absolute form against the disk — every `.`,…
* `bool symlink(cstring target, cstring path)` — Make `path` a symbolic link to `target`. `target` is stored as written and…
* `i64 read_link(cstring path, []char8 out)` — What `path` points at, written into `out`, as a count of bytes. `-1` when…
* `bool is_symlink(cstring path)` — Whether `path` is itself a symbolic link.
* `bool executable(cstring path)` — Whether the caller could execute `path`.
* `bool make_executable(cstring path)` — Add the execute bit to `path`, **for each class that can already read it**.
* `cstring cwd()` — Where the process is. The bytes are this package's own storage and the next…
* `bool set_cwd(cstring path)`
* `bool make_dir(cstring path)` — Create one directory. Not the parents — `mkdir -p` is a loop over a split…
* `bool make_private_dir(cstring path)` — `make_dir` with the mode `0700` (`0x1C0`): a directory only its owner can…
* `type Entry: (string8 path, bool is_dir)` — One path a walk found, and whether it is a directory.
* `type Listing: (ref []char8 bytes, ref []Entry entries)` — What a walk returns: one block of bytes holding every path, and one slice of…
* `Listing walk(string8 root, (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc)` — Every path under `root`, depth first, **a directory before its contents**.
* `void release(Listing l)` — Free what a walk allocated. A `Listing` that was never walked is safe to…
* `bool make_dirs(string8 path, (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc)` — `mkdir -p`: every directory on `path` that is not there yet.
* `type FsFindData`
* `type FsHandleInfo`
* `type FsSecurity: (#c.ulong nLength, rawptr lpSecurityDescriptor, #c.int bInheritHandle)`
* `type FsOverlapped`
* `const #c.uint CP_UTF8`
* `const #c.ulong GENERIC_READ`
* `const #c.ulong GENERIC_WRITE`
* `const #c.ulong FILE_READ_ATTRIBUTES`
* `const #c.ulong FILE_SHARE_READ`
* `const #c.ulong FILE_SHARE_WRITE`
* `const #c.ulong FILE_SHARE_DELETE`
* `const #c.ulong OPEN_EXISTING`
* `const #c.ulong CREATE_NEW`
* `const #c.ulong FILE_ATTRIBUTE_NORMAL`
* `const #c.ulong FILE_ATTRIBUTE_DIRECTORY`
* `const #c.ulong FILE_ATTRIBUTE_READONLY`
* `const #c.ulong FILE_ATTRIBUTE_REPARSE_POINT`
* `const #c.ulong FILE_FLAG_BACKUP_SEMANTICS`
* `const #c.ulong FILE_FLAG_OPEN_REPARSE_POINT`
* `const #c.ulong INVALID_FILE_ATTRIBUTES`
* `const rawptr INVALID_HANDLE_VALUE`
* `const #c.ulong MOVEFILE_REPLACE_EXISTING`
* `const #c.ulong MOVEFILE_WRITE_THROUGH`
* `const #c.ulong SYMBOLIC_LINK_FLAG_DIRECTORY`
* `const #c.ulong SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE`
* `const #c.ulong IO_REPARSE_TAG_SYMLINK`
* `const #c.ulong IO_REPARSE_TAG_MOUNT_POINT`
* `const #c.ulong VOLUME_NAME_DOS`
* `const #c.ulong HEAP_ZERO_MEMORY`
* `const #c.long ERROR_FILE_NOT_FOUND`
* `const #c.long ERROR_PATH_NOT_FOUND`
* `const #c.long ERROR_NO_MORE_FILES`
* `const #c.long ERROR_ACCESS_DENIED`
* `const #c.long ERROR_INVALID_PARAMETER`
* `const #c.long ERROR_ALREADY_EXISTS`
* `const #c.long ERROR_FILE_EXISTS`
* `#c.int MultiByteToWideChar(#c.uint code_page, #c.ulong flags, cstring from, #c.int from_count, ^u16 into, #c.int into_count)`
* `#c.int WideCharToMultiByte(#c.uint code_page, #c.ulong flags, ^const u16 from, #c.int from_count, ^#c.char into, #c.int into_count, cstring default_char, ^#c.int used_default)`
* `#c.ulong GetLastError()`
* `void SetLastError(#c.ulong code)`
* `rawptr FindFirstFileW(^const u16 pattern, ^FsFindData found)`
* `#c.int FindNextFileW(rawptr search, ^FsFindData found)`
* `#c.int FindClose(rawptr search)`
* `#c.ulong GetFileAttributesW(^const u16 path)`
* `#c.int SetFileAttributesW(^const u16 path, #c.ulong attributes)`
* `rawptr CreateFileW(^const u16 path, #c.ulong access, #c.ulong share, ^FsSecurity security, #c.ulong disposition, #c.ulong flags, rawptr template_file)`
* `#c.int GetFileInformationByHandle(rawptr file, ^FsHandleInfo info)`
* `#c.int CloseHandle(rawptr handle)`
* `#c.int ReadFile(rawptr file, rawptr into, #c.ulong count, ^#c.ulong got, ^FsOverlapped overlapped)`
* `#c.int WriteFile(rawptr file, const rawptr from, #c.ulong count, ^#c.ulong wrote, ^FsOverlapped overlapped)`
* `#c.int FlushFileBuffers(rawptr file)`
* `#c.int DeviceIoControl(rawptr device, #c.ulong code, rawptr in_buffer, #c.ulong in_size, rawptr out_buffer, #c.ulong out_size, ^#c.ulong returned, ^FsOverlapped overlapped)`
* `#c.ulong GetFullPathNameW(^const u16 path, #c.ulong size, ^u16 into, ^^u16 file_part)`
* `#c.ulong GetFinalPathNameByHandleW(rawptr file, ^u16 into, #c.ulong size, #c.ulong flags)`
* `#c.ulong GetCurrentDirectoryW(#c.ulong size, ^u16 into)`
* `#c.int SetCurrentDirectoryW(^const u16 path)`
* `#c.int CreateDirectoryW(^const u16 path, ^FsSecurity security)`
* `#c.int RemoveDirectoryW(^const u16 path)`
* `#c.int DeleteFileW(^const u16 path)`
* `#c.int MoveFileExW(^const u16 from, ^const u16 to, #c.ulong flags)`
* `#c.int CopyFileW(^const u16 from, ^const u16 to, #c.int fail_if_exists)`
* `#c.uchar CreateSymbolicLinkW(^const u16 link, ^const u16 target, #c.ulong flags)`
* `rawptr GetProcessHeap()`
* `rawptr HeapAlloc(rawptr heap, #c.ulong flags, #c.size_t bytes)`
* `#c.int HeapFree(rawptr heap, #c.ulong flags, rawptr block)`
* `rawptr LocalFree(rawptr block)`
* `#c.ullong GetTickCount64()`
* `#c.ulong GetCurrentProcessId()`
* `const #c.int SDDL_REVISION_1`
* `#c.int ConvertStringSecurityDescriptorToSecurityDescriptorW(^const u16 sddl, #c.ulong revision, ^rawptr descriptor, ^#c.ulong size)`
* `#c.size_t wcslen(^const u16 s)`
