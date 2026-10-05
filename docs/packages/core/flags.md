<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:flags

core:flags — command lines: flags, positionals, subcommands, and the help
text they imply, declared once and read back typed.

    import "core:flags"
    import "core:fmt"

    i32 run([]string8 args):                 // the words after the program's name
        flags.Parser p = flags.parser("greet", "Say hello to someone.")
        flags.BoolFlag loud = p.boolean("-l --loud", "shout")
        flags.IntFlag times = p.integer("-n --times", "N", "how many times", 1)
        flags.TextFlag who = p.positional("NAME", "who to greet", true)
        try string8 _ = p.parse(args) else:
            fmt.println(p.error())             // the sentence, repair included
            return 2
        if p.wants_help():
            fmt.println(p.help())
            return 0
        for i in 0..<p.number(times):
            fmt.println("hello, ", p.value(who), "!" if p.on(loud) else ".")
        return 0

**Tier W H. Script class S.** Pure Dado, and it never reads the process's
arguments itself: `parse` takes a `[]string8` the caller supplies, so the
same declarations parse a test's words, a script's, or `main`'s, on every
target alike.

**The shape, and why.** A `Parser` holds a table of declarations. Each
declaring method answers a small typed handle (`BoolFlag`, `TextFlag`,
`IntFlag`, `ListFlag`, each a `distinct` index into the table), and after
`parse` the reading methods take the handle back: `on`, `value`,
`number`, `values`. The program writes no loop and the handle's type
decides what may be read from it, so reading a switch as a number is a
compile error (ERR0504) rather than a zero. A handle is an index and not a
`^T` into the table, so declaring more flags (which grows the table) never
leaves an earlier handle pointing at moved memory. Calls are all
positional with trailing defaults, as the language asks (a call never
mixes positional and named arguments), so a declaration reads
`names, form, summary[, default][, scope]`.

**Spellings are written as they are typed.** `names` is the flag's
spellings separated by spaces: `-v --verbose`, `-o`, `--cc`, or an old
spelling kept beside a new one, `--base --html-base`. A short spelling is
a dash and one byte, a long one two dashes and a word.

**The form is written as the help shows it**, and decides how a value is
given:

* `PROG` — `--cc PROG`, `--cc=PROG`; short: `-o PATH`, `-o=PATH`,
  `-oPATH`;
* `=LIST` — only `--manifest=LIST` (a bare `--manifest` is refused, and the
  next word is never taken as its value);
* `[=PATH]` — `--html` alone, or `--html=PATH`; alone it gives the empty
  value, or a `choice`'s first choice, and `given` tells it from absent;
* for a `choice`, the inner placeholder lists the accepted values:
  `=web|js`, `[=address|thread]`, `off|default`.

A value is never empty: `--cc ""` and `--cc=` are refused. Short boolean
flags cluster: `-vh` is `-v -h`, and a short flag that takes a value ends
a cluster, so `-vlm` is `-v -l m`. A plain `-` is a positional (the
conventional name for a standard stream).

**No automatic `--no-name`.** Where a program has an opposite, it declares
it as a flag of its own (`--no-werror`, `--no-assert`): most such flags
have no positive twin to negate, and an automatic negation would add
spellings nobody chose (`--no-verbose`, `--no-release`) to every program's
command line, each one more near miss for the spelling suggestions.

**Subcommands.** `command` declares a word; once any is declared, the
first positional word must be one of them, and `parse` answers it. One
level only: a word after the command that picks a mode of it (`doc
reference`) is a positional the program reads. Every flag and positional
declaration takes a `scope`: `""`, the default, is every command and the
words before the command; otherwise it is the command names it belongs to,
separated by spaces, so flags shared by several commands are declared once
(`"build run check emit test"`). A flag written where its scope does not
reach is refused with the commands it belongs to.

**Positionals.** `positional` takes one word, optional unless `required`;
`rest` collects every further positional word; `tail` collects exactly
the words after `--` (a `run` command's program arguments), and while a
command has a `tail`, a word after `--` goes nowhere else. Without one,
`--` only ends the flags: what follows is positional however it is
spelled. A positional word with nowhere to go is refused.

**Help.** Every parser declares `-h --help` itself. `parse` stops at it
and answers success, `wants_help` says so, and `help` writes the text for
the command chosen (or for the program): a usage line, the summary, the
commands, the positionals and the flags, each flag with its form, its
summary, its default and whether it repeats, in declaration order and
aligned, wrapped to 80 columns where the words allow. Plain text: no
colour. A declaration whose summary is `""` is accepted and left out of
the help, which is how an old spelling stays working without being
advertised.

**Values are windows.** A text value, a positional and a list's items are
windows into the `args` passed to `parse` (`-DNAME=1`'s value is the word
from its third byte), so `args` must outlive the reads. Nothing is
copied.

**Error codes.** `parse` is failable. `message(code)` answers a sentence
for the code; `error()` answers the full sentence for the failure just
returned, which names the word, the flag and the repair:

* `UNKNOWN_FLAG` — no such flag here; the nearest spelling (edit distance)
  is suggested, or the commands it belongs to are named.
* `MISSING_VALUE` — a flag that takes a value has none (or an empty one);
  its form is shown.
* `BAD_VALUE` — the value is not what the flag accepts: not a whole
  number, not one of its choices, or a value given to a switch.
* `UNKNOWN_COMMAND` — the first word is not a command; the nearest is
  suggested.
* `EXTRA_POSITIONAL` — a positional word with no place to go.
* `MISSING_POSITIONAL` — a required positional is absent.
* `BAD_SPEC` — the declarations themselves are wrong: a spelling that is
  neither `-c` nor `--word`, one spelling twice where both reach, a scope
  naming no command, a value form missing, a default that is not a choice,
  a number whose value is optional.
  This is the program's mistake, not the user's, reported at `parse`.

**Allocation.** Ambient: the table, the lists and the failure sentence are
on `#default`, and `help` answers a `ref []char8` the caller owns. Under a
`using` arena nothing needs freeing; outside one, `destroy` frees what the
parser holds and the caller `#delete`s the help text. Lengths and indices
are `i64`.

**Thread-safety.** No package state. A `Parser` is a plain value that one
thread at a time may declare into, parse with and read; separate parsers
are independent.

## Declarations

98 declarations, 35 public.

* `const i32 UNKNOWN_FLAG = 1` — `parse`'s code: a flag no declaration in reach spells.
* `const i32 MISSING_VALUE = 2` — `parse`'s code: a flag that takes a value was given none, or an empty one.
* `const i32 BAD_VALUE = 3` — `parse`'s code: a flag's value is not one it accepts.
* `const i32 UNKNOWN_COMMAND = 4` — `parse`'s code: the first word is not a declared command.
* `const i32 EXTRA_POSITIONAL = 5` — `parse`'s code: a positional word has no positional left to fill.
* `const i32 MISSING_POSITIONAL = 6` — `parse`'s code: a required positional was not given.
* `const i32 BAD_SPEC = 7` — `parse`'s code: the declarations are malformed or contradict each other.
* `string8 message(i32 code)` — A sentence for each of this package's codes, and one for any other.
* `distinct type BoolFlag: i64` — A switch: on when given.
* `distinct type TextFlag: i64` — One text: a flag's value, a `choice`, or a positional.
* `distinct type IntFlag: i64` — A whole number.
* `distinct type ListFlag: i64` — A list: a repeatable flag's values, or the words a `rest` or `tail`
* `type Parser` — A program's command line: what it declares, and what the last `parse`
* `void command(^self, string8 name, string8 summary)` — A command word, with the one line the help shows beside it (`""`
* `BoolFlag boolean(^self, string8 names, string8 summary, string8 scope = "")` — A switch, off unless given. A value written to it (`--release=yes`)
* `TextFlag text(^self, string8 names, string8 form, string8 summary, string8 default = "", string8 scope = "")` — A flag with one text value; the last one given wins. `default` is what…
* `IntFlag integer(^self, string8 names, string8 form, string8 summary, i64 default = 0, string8 scope = "")` — A flag whose value must be a whole number (decimal, an optional sign).
* `ListFlag list(^self, string8 names, string8 form, string8 summary, string8 scope = "")` — A repeatable flag: every value given, in order (`-D A=1 -D B=2`).
* `TextFlag choice(^self, string8 names, string8 form, string8 summary, string8 default = "", string8 scope = "")` — A flag whose value must be one of the choices its form lists between…
* `TextFlag positional(^self, string8 name, string8 summary, bool required = false, string8 scope = "")` — One positional word, filled in declaration order. A `required` one…
* `ListFlag rest(^self, string8 name, string8 summary, string8 scope = "")` — Every positional word left over once the `positional`s are filled.
* `ListFlag tail(^self, string8 name, string8 summary, string8 scope = "")` — The words after `--`, exactly as written, flags or not.
* `!string8 parse(^self, []string8 args)` — Read `args` (the words after the program's name) against the…
* `bool wants_help(self)` — Whether the last `parse` stopped at `-h` or `--help`.
* `string8 error(self)` — The full sentence for the last failure of `parse`; empty after a…
* `string8 command_name(self)` — The command word the last `parse` chose, or `""`.
* `bool on(self, BoolFlag f)` — Whether the switch was given.
* `string8 value(self, TextFlag f)` — The text given, or the default when it was not.
* `bool given(self, TextFlag f)` — Whether the text was given at all, which tells a form-`[=PATH]` flag…
* `i64 number(self, IntFlag f)` — The number given, or the default when it was not.
* `[]string8 values(self, ListFlag f)` — Every value given, in order; empty when none was.
* `ref []char8 help(self)` — The help for the command the last `parse` chose, or for the program…
* `ref []char8 help_for(self, string8 command)` — The help for `command`, or for the program when `command` is `""`
* `void destroy(^self)` — Free what the parser holds. Not needed under a `using` arena. The…
* `Parser parser(string8 program, string8 summary)` — A parser for `program`, which the help and the error sentences name, with…
