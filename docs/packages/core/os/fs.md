<!-- dadoc 1.0.0-rc.8 -->
<!-- commit 6c56ce20508a (dirty) -->
# core:os/fs

core:os/fs — what is on the disk.

    import "core:os"
    import "core:os/fs"

    try fs.Listing l = fs.read_dir(".") else why:
        println(".: ", os.message(why))
        return 1
    defer l.destroy()
    for e in l.entries:
        println(e.path, "  dir=", e.is_dir)

**The package, in five files.** This one asks the questions of a path that
have no failure but *no* — `is_dir`, `is_file`, `exists`, `is_link`,
`is_executable`, `hidden` — and holds the system's directory stream and
`struct stat` reader the rest is written over. `files.dado` is whole files,
the tree changed, `info`, `read_dir`, `glob`, `chmod` and the working
directory. `handle.dado` is an open `File`: read, write, seek, truncate,
sync and lock. `walk.dado` walks and makes whole trees. `windows.dado` holds
what the Windows arms share.

**Every failable function fails `os.Error`** (`os.Error.NOT_FOUND`, …),
whichever system it runs on, and `os.message(why)` is the sentence. **Paths
are `string8`**, UTF-8 on every system: POSIX passes the bytes on with a
`\0` added for the call, and Windows converts them to UTF-16
(`windows.dado`).

**Tier W H.** Hosted means Linux, macOS, Windows and the BSDs, each with an
arm of its own. `#WEB` builds every function: a page has no file system, and
node's is not imported yet, so each question answers false (nothing is
there) and each failable function fails with `os.Error.UNSUPPORTED`. `#NONE`
has no file system and is refused at build time.

**What raises into a script.** `hidden`, `FileInfo`'s methods, the
`Listing`'s `destroy` and the enums are plain Dado and promised. Every other
function asks the system through a `foreign` call, and is not; `walk`,
`make_dirs`, `read_dir` and `glob` are Dado, but each reaches the system too.

Joining, splitting and cleaning paths is `core:path`'s, which asks the disk
nothing; this package is the half that does.

**What is read from `struct stat`, and how.** POSIX fixes which members
`struct stat` has and nothing about their order, their widths or the padding
between them, and every C library lays it out its own way — glibc
differently on each architecture, musl, Darwin and each BSD differently
again. So no layout is restated here: a few lines of C beside this file
(`posix_stat` below) read the members by name, on whatever system the
program is built for, into five 64-bit numbers Dado owns. `struct dirent`'s
name is read the same way.

Thread-safety: every function is safe from any thread; two threads changing
one tree at once get whatever order the system gives them.

## Declarations

366 declarations, 55 public.

* `enum i32 Kind` — What `FileInfo.kind` says a path is.
* `type FileInfo` — What `info` and `link_info` answer about a path, read at once.
* `bool FileInfo.is_file(self)` — Whether this is a regular file.
* `bool FileInfo.is_dir(self)` — Whether this is a directory.
* `bool FileInfo.is_link(self)` — Whether this is a symbolic link (only `link_info` answers one).
* `os.Error!ref []char8 read_file(string8 path)` — The whole file at `path` as text, owned by the caller. Fails with `IS_DIR`
* `os.Error!ref []u8 read_bytes(string8 path)` — The whole file at `path` as bytes, owned by the caller: `read_file` for a…
* `os.Error!void write_file(string8 path, string8 data)` — Replace the file at `path` with the text `data`, atomically: a reader sees…
* `os.Error!void write_bytes(string8 path, []u8 data)` — Replace the file at `path` with the bytes `data`, atomically, as…
* `os.Error!Listing read_dir(string8 dir)` — Every entry of the directory `dir`, in the system's order, `.` and `..`
* `os.Error!Listing glob(string8 pattern)` — Every path that matches `pattern`, in byte order. Each component may hold…
* `os.Error!void remove_file(string8 path)` — Remove the file (or the link) at `path`. Fails with `IS_DIR` for a…
* `os.Error!void remove_dir(string8 path)` — Remove the directory at `path`, which must be empty (`NOT_EMPTY`
* `os.Error!void remove_all(string8 path)` — Remove `path` and, for a directory, everything in it. A link is removed…
* `os.Error!void rename(string8 from, string8 to)` — Move `from` to `to`, replacing what is at `to`. Across filesystems it…
* `os.Error!void copy_file(string8 from, string8 to)` — Copy the file `from` to `to`, replacing `to`, with `from`'s…
* `os.Error!i64 mtime(string8 path)` — When the file at `path` (links followed) was last written, in…
* `os.Error!ref []char8 make_temp_dir(string8 prefix)` — Make a new directory under `os.temp_dir()` that only its owner can…
* `os.Error!void make_symlink(string8 target, string8 path)` — Make `path` a symbolic link to `target`, stored as written (a relative…
* `os.Error!FileInfo info(string8 path)` — What is at `path`, links followed: its kind, permissions, size and last…
* `os.Error!FileInfo link_info(string8 path)` — What is at `path` itself, a link not followed: a link answers…
* `os.Error!void chmod(string8 path, u32 mode)` — Set the permission bits of `path` to `mode` (`0x1ED` is `0755`). On…
* `os.Error!ref []char8 current_dir()` — The working directory, owned by the caller, of any length.
* `os.Error!void change_dir(string8 path)` — Make `path` the working directory.
* `os.Error!void create_dir(string8 path)` — Make one directory at `path`, not its parents (`make_dirs` makes those).
* `os.Error!ref []char8 canonical(string8 path)`
* `os.Error!ref []char8 link_target(string8 path)`
* `bool hidden(string8 name)` — Whether a name is `.`, `..`, or begins with a dot: the three a listing…
* `bool is_dir(string8 path)` — Whether `path` is a directory, following links; false when nothing is…
* `bool is_file(string8 path)` — Whether `path` is a regular file (anything but a directory), following…
* `bool exists(string8 path)` — Whether something is at `path`, following links (a dangling link is not…
* `bool is_link(string8 path)` — Whether `path` is itself a symbolic link (not followed): on Windows a…
* `bool is_executable(string8 path)` — Whether the caller could run the file at `path`.
* `os.Error!void make_executable(string8 path)` — Make the file at `path` runnable by whoever can read it. Fails with the…
* `os.Error!void create_private_dir(string8 path)` — Make one directory only its owner can open. Fails with `EXISTS` when…
* `enum i32 Open` — What `open_file` opens the file for, or-ed together. One of `Read` and…
* `type File` — An open file. `fd` is the POSIX descriptor and `handle` the Windows one;…
* `os.Error!i64 File.read(self, []char8 into)` — Read at most `#len(into)` bytes of text at the current position,…
* `os.Error!i64 File.read_bytes(self, []u8 into)` — `read`, into bytes.
* `os.Error!i64 File.write(self, string8 data)` — Write all of `data` at the current position (at the end, for `Append`),…
* `os.Error!i64 File.write_bytes(self, []u8 data)` — `write`, of bytes.
* `os.Error!i64 File.seek(self, i64 offset, io.Whence from = io.Whence.Start)` — Move the position to `offset` counted `from` the start, the current…
* `os.Error!void File.truncate(self, i64 size)` — Cut or extend the file to `size` bytes; the position does not move.
* `os.Error!void File.sync(self)` — Push what was written to the disk before answering.
* `os.Error!FileInfo File.info(self)` — What the open file is: its kind, permissions, size and last write.
* `os.Error!void File.lock(self, bool exclusive)` — Take an exclusive lock (`exclusive`) or a shared one on the whole file,…
* `os.Error!bool File.try_lock(self, bool exclusive)` — `lock` without the wait: true when the lock was taken, false when…
* `os.Error!void File.unlock(self)` — Give back the lock `lock` or `try_lock` took.
* `void File.close(^self)` — Close the file, which also gives back its lock; a closed `File` is…
* `os.Error!File open_file(string8 path, Open flags)` — Open the file at `path` as `flags` says, at position 0. Fails with…
* `type Entry: (string8 path, bool is_dir)` — One path a listing found, and whether it is a directory (links followed).
* `type Listing` — What a listing answers: one block of bytes holding every path, and one…
* `void Listing.destroy(^self)` — Free the two blocks the listing holds, and leave it empty. Safe on an…
* `Listing walk(string8 root)` — Every path under `root`, depth first, **a directory before its contents**,…
* `os.Error!void make_dirs(string8 path)` — `mkdir -p`: every directory on `path` that is not there yet.
