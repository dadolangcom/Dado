//@ procs/failure 30
//@ ### C's undefined behaviour on the web and on `#NONE`
//@
//@ Dado's own arithmetic has none of the behaviours C leaves undefined: a
//@ `+` wraps, a shift past the width is defined, a zero divisor is Dado's
//@ own check and a `float` narrows to an integer by saturating. C's
//@ undefined-behaviour sanitizer therefore finds C's — a package's `.c`
//@ sources and inline `foreign` C — and natively `dado test`, `--debug`
//@ and `--sanitize` on a build or a run compile it with its runtime. The
//@ web and `#NONE` have no runtime to link, so there a sanitized build
//@ (`dado test --target=web` and `--debug` by default, as natively;
//@ `--sanitize` otherwise) is UBSan alone: signed overflow, shifts,
//@ division by zero and float-to-integer overflow call handlers the
//@ emitted C defines (`dadoc --ubsan-handlers`), and every other check is
//@ a bare trap. On the web the check is reported through the console —
//@ `ubsan: ub.c:2:37: signed integer overflow: 2147483647 + 1 cannot be
//@ represented in type 'int'`, then, where the build keeps the full call
//@ stack (`dado test`, `--debug`, `--full-trace`), the Dado calls it is
//@ under — and the module traps, status 134 from the loader. A
//@ `#NONE` target has nowhere to write: the handler calls
//@ `undefined_behavior` if the program's link defines one, `void
//@ undefined_behavior(const char *check, const char *file, unsigned line,
//@ unsigned column)`, `check` being the check's `-fsanitize=` name
//@ (`signed-integer-overflow`, `unsigned-integer-overflow`,
//@ `integer-divide-by-zero`, `shift-exponent`, `shift-base`,
//@ `float-cast-overflow`), and then traps. `--sanitize` and
//@ `--sanitize=address` there say the build is UBSan alone, and
//@ `--sanitize=thread` is an error. Sanitized or not, a `#NONE` object
//@ asks nothing of the link beyond the hook and the memcpy, memset,
//@ memmove and memcmp every freestanding compiler may call: its compile
//@ turns off the compiler's own default UBSan and stack protector.
