<!-- dadoc 1.0.0-rc.2 -->
<!-- commit 7e1d5137cae1 (dirty) -->
# Diagnostic codes

Every code `dadoc` reports. Each is stable within a major version; its page says what it means and the rule behind it. `dado explain <CODE>` prints the same page, and `dado search <words>` finds a code by what it says.

| Code | Class | Title |
|---|---|---|
| [ERR0001](ERR0001.md) | error | construct is not implemented at this milestone |
| [ERR0101](ERR0101.md) | error | unexpected character in source |
| [ERR0102](ERR0102.md) | error | tab used in indentation |
| [ERR0103](ERR0103.md) | error | dedent does not match any open indentation level |
| [ERR0104](ERR0104.md) | error | bracket left open at end of file |
| [ERR0105](ERR0105.md) | error | closing bracket matches no open bracket |
| [ERR0106](ERR0106.md) | error | string literal is never closed |
| [ERR0107](ERR0107.md) | error | unknown escape sequence in a string literal |
| [ERR0108](ERR0108.md) | error | malformed numeric literal |
| [ERR0109](ERR0109.md) | error | block comment is never closed |
| [ERR0110](ERR0110.md) | error | character literal is never closed |
| [ERR0111](ERR0111.md) | error | a character literal holding other than one character |
| [ERR0112](ERR0112.md) | error | a `"""` literal whose text does not line up under its closing delimiter |
| [ERR0201](ERR0201.md) | error | unexpected token |
| [ERR0202](ERR0202.md) | error | expected an indented block after ':' |
| [ERR0203](ERR0203.md) | error | two visibility modifiers in the one slot a declaration has for them |
| [ERR0204](ERR0204.md) | error | a program declaring a name in the compiler's `#` namespace |
| [ERR0205](ERR0205.md) | error | an `else when` arm after the `else` that already closed the chain |
| [ERR0206](ERR0206.md) | error | a variadic parameter written where one cannot be, or reached by a route its calling convention has no answer for |
| [ERR0207](ERR0207.md) | error | a slot of a destructure written without parentheses that ends without a name |
| [ERR0208](ERR0208.md) | error | a `using` written where its subject cannot be scoped — no suite under the block form, or no call in front of the postfix one |
| [ERR0209](ERR0209.md) | error | a keyword written where a variable, parameter or member name goes |
| [ERR0301](ERR0301.md) | error | program has no 'main' function |
| [ERR0302](ERR0302.md) | error | the import graph has a cycle |
| [ERR0303](ERR0303.md) | error | an import path names no package, or a JavaScript path no file |
| [ERR0304](ERR0304.md) | error | one directory whose files declare two package names |
| [ERR0305](ERR0305.md) | error | two imports bound under one name |
| [ERR0306](ERR0306.md) | error | an entry point whose parameters C would not accept |
| [ERR0307](ERR0307.md) | error | `@test` written somewhere other than the first line of the file |
| [ERR0308](ERR0308.md) | error | a `@test` file with no `i32 test()`, or one C could not run |
| [ERR0309](ERR0309.md) | error | a bare name that more than one unqualified import, or one and the package or a built-in, supplies |
| [ERR0310](ERR0310.md) | error | a declaration whose name an `import .` of its file already makes bare |
| [ERR0311](ERR0311.md) | error | `imports:` or `project:` written in a package fetched into an `imports` location |
| [ERR0401](ERR0401.md) | error | unknown type name |
| [ERR0402](ERR0402.md) | error | a type of that name is already declared |
| [ERR0403](ERR0403.md) | error | a view names the slots of something that is not in scope |
| [ERR0404](ERR0404.md) | error | one view binds the same name to two slots |
| [ERR0405](ERR0405.md) | error | a call names something that is not a declared function |
| [ERR0406](ERR0406.md) | error | a function of that name is already declared |
| [ERR0407](ERR0407.md) | error | two embedded shapes contribute one unqualified name |
| [ERR0408](ERR0408.md) | error | a name that is not a value in scope |
| [ERR0409](ERR0409.md) | error | one scope declares the same name twice |
| [ERR0410](ERR0410.md) | error | a name that the active view does not bind to a slot |
| [ERR0411](ERR0411.md) | error | a name that starts with the prefix emitted names use |
| [LNT0412](LNT0412.md) | lint | a name is declared and never read |
| [ERR0413](ERR0413.md) | error | a name another package declared but does not export |
| [ERR0414](ERR0414.md) | error | a name one of the global builtins already owns |
| [ERR0415](ERR0415.md) | error | a name that is not a member of that enum |
| [ERR0416](ERR0416.md) | error | a name that is not a member of the union it is spelled against |
| [ERR0417](ERR0417.md) | error | a `#` name the compiler does not define |
| [ERR0418](ERR0418.md) | error | an `@` name that is not one of the ten attributes, or one of them written where none may go |
| [WRN0419](WRN0419.md) | warning | a package-level declaration that writes no visibility word, so it is public by default rather than by decision |
| [ERR0501](ERR0501.md) | error | type declaration resolves only to itself |
| [ERR0502](ERR0502.md) | error | array length is not a compile-time integer |
| [ERR0503](ERR0503.md) | error | a view names a different number of slots than the tuple has |
| [ERR0504](ERR0504.md) | error | a value of one shape where another is required |
| [LNT0505](LNT0505.md) | lint | a number that does not fit the integer type it is written for, and wraps |
| [ERR0506](ERR0506.md) | error | a shape with no slots has no value to hold |
| [ERR0507](ERR0507.md) | error | a shape whose recursion no pointer bounds has no C name |
| [ERR0508](ERR0508.md) | error | an enum value that arrived by counting collides with a written one |
| [ERR0509](ERR0509.md) | error | an enum backed by something that is not an integer |
| [LNT0510](LNT0510.md) | lint | a code point too wide for the character type it is written against, which keeps its low bits |
| [ERR0511](ERR0511.md) | error | `shapeof` written where there is nothing to compute it from |
| [ERR0512](ERR0512.md) | error | `auto` written where no value supplies a type |
| [ERR0513](ERR0513.md) | error | a union member with no name, or with a name another member already took |
| [ERR0514](ERR0514.md) | error | a union declaring no members, which no value could satisfy |
| [ERR0515](ERR0515.md) | error | `distinct` on a union, which is already nominal by its tag set |
| [ERR0516](ERR0516.md) | error | a projection mixing `xyzw` and `rgba`, which name the same axes twice |
| [ERR0517](ERR0517.md) | error | axes used on a shape whose view has taken one of the eight letters for a slot |
| [ERR0518](ERR0518.md) | error | an inferred type whose slots several visible views name differently, so the source does not say which vocabulary applies |
| [ERR0519](ERR0519.md) | error | a position a shape does not have |
| [ERR0520](ERR0520.md) | error | a slot of a contract that is not a function reference, so nothing could be called through it |
| [ERR0521](ERR0521.md) | error | a window that is not a run of the shape's positions |
| [ERR0522](ERR0522.md) | error | the address of a whole element of a shape stored as one run per slot |
| [ERR0523](ERR0523.md) | error | a `!` or `E!` in front of a type written anywhere but a function's return in its signature or a failable function type |
| [ERR0524](ERR0524.md) | error | slot names written inline under a type constructor, which no name receives and no later read can find |
| [ERR0525](ERR0525.md) | error | a slot name or a `view` written over a shape carrying a layout mark, which presents runs rather than slots |
| [ERR0526](ERR0526.md) | error | a count of slots no shape can have |
| [WRN0527](WRN0527.md) | warning | a folded float→integer conversion whose value the destination does not hold |
| [ERR0528](ERR0528.md) | error | a `#channel` written somewhere other than a package-level declaration or a named member of a `type` — a local, a function's signature, an anonymous tuple, a `union` or a `foreign` block |
| [ERR0529](ERR0529.md) | error | a channel parameter that is a view — `[]T`, `^T`, a string, a `cstring` — or a map or a channel, anywhere inside its type |
| [ERR0530](ERR0530.md) | error | a `ref` in the message of a `#channel(latest)` |
| [ERR0531](ERR0531.md) | error | a copy of a value holding a channel — an assignment, a by-value parameter or return, a binding or a walk by value |
| [ERR0532](ERR0532.md) | error | `#channel(N)` whose `N` is not a compile-time integer, is less than 1, or asks for more storage than a channel may hold |
| [ERR0533](ERR0533.md) | error | a method with no receiver, one more visible than its type, or one written in a declaration that holds no methods |
| [ERR0534](ERR0534.md) | error | a method named like a slot of the declaration that holds it |
| [ERR0535](ERR0535.md) | error | a trait extending a type or itself, a `using` naming a type, or a child trait replacing a parent's default |
| [ERR0536](ERR0536.md) | error | a declaration taking a trait on without a method the trait prescribes, or with a different signature or receiver |
| [ERR0537](ERR0537.md) | error | a value converted to a trait value whose type is no declaration, or a declaration that does not take the trait on |
| [ERR0538](ERR0538.md) | error | a generic `type` named without its arguments or with the wrong number of them, or declared embedding its binder or holding a channel no instantiation could own alone |
| [ERR0539](ERR0539.md) | error | a number no value of its type represents |
| [ERR0601](ERR0601.md) | error | an argument list fills a different number of slots than the parameter list has |
| [ERR0602](ERR0602.md) | error | a named argument names no parameter |
| [ERR0603](ERR0603.md) | error | two arguments fill the same slot |
| [ERR0604](ERR0604.md) | error | one call mixes named and positional arguments |
| [ERR0605](ERR0605.md) | error | a '..' splat cannot be expanded here |
| [ERR0606](ERR0606.md) | error | a value with no place where a place is needed — an assignment, an address, a `^self` receiver or a trait value |
| [ERR0607](ERR0607.md) | error | an operator its operands do not support |
| [ERR0608](ERR0608.md) | error | an expression evaluated for an effect it cannot have |
| [ERR0609](ERR0609.md) | error | a matrix product whose dimensions do not meet |
| [ERR0610](ERR0610.md) | error | `expect` written with no message, which is a claim that does not read as one |
| [ERR0611](ERR0611.md) | error | a call to a failable function that nothing handles |
| [ERR0612](ERR0612.md) | error | a map literal writing one constant key twice — a typo every time, and the one map error catchable before the program runs |
| [LNT0613](LNT0613.md) | lint | a parameter so big that passing it by value copies hundreds of bytes on every call |
| [ERR0614](ERR0614.md) | error | `#send`, `#peek`, `#drain` or `#release` over something that is not a channel, or written with named, spliced or missing arguments |
| [ERR0615](ERR0615.md) | error | a write to a by-value `self`, a slot of it, or its address — a write the caller would never see |
| [ERR0616](ERR0616.md) | error | a call through a trait value to a method whose signature mentions `Self` beyond its receiver, or that is generic |
| [ERR0701](ERR0701.md) | error | return value does not match the function's return type |
| [ERR0702](ERR0702.md) | error | a condition that is not a bool |
| [ERR0703](ERR0703.md) | error | `break` or `continue` outside a loop |
| [ERR0704](ERR0704.md) | error | a path through a function does not return its value |
| [ERR0705](ERR0705.md) | error | a `switch` with no `else` arm over a value that cannot promise exhaustiveness, or over a closed failure whose arms leave a member out |
| [ERR0706](ERR0706.md) | error | a `case` arm whose value an earlier arm already matched |
| [ERR0707](ERR0707.md) | error | a `defer` whose statement would leave the scope it is unwinding — by `return`, `break`, `continue`, `fallthrough`, `fail`, a `try` that passes a failure on, or a second `defer`, at any depth inside it |
| [ERR0708](ERR0708.md) | error | a `switch` over a `#` compiler definition, which compares at run time a value the compiler already knew |
| [ERR0709](ERR0709.md) | error | a `&` loop binding with no storage to address — a range, a string, an index, a map key, or a subject that is not a place |
| [LNT0710](LNT0710.md) | lint | a map whose key and value are one type, walked with one binding — the binding is the key, and the same loop used to bind the value |
| [ERR0711](ERR0711.md) | error | `fallthrough` somewhere it cannot stand — not the last statement of a `case` body, or not in a `switch` at all |
| [ERR0712](ERR0712.md) | error | `fallthrough` in the last `case` arm or in an `else`, where there is no next arm to fall into |
| [ERR0713](ERR0713.md) | error | `fallthrough` in a union `switch`, whose next arm binds a member the value does not hold |
| [ERR0714](ERR0714.md) | error | `fail`, a `try` that passes a failure on or fails with a code, a `defer!`, or a `defer fail`, in a function not marked failable |
| [ERR0715](ERR0715.md) | error | `fail` with a code that is zero |
| [ERR0716](ERR0716.md) | error | a `try` over something that is neither a failable call nor a map read, one that would throw its value away, or one whose `else` is malformed, binds over a constant, translates a code, or lets a declared binding be read on the failing path; or a `defer!` over anything but one failable call answering no value |
| [WRN0717](WRN0717.md) | warning | two named failure codes with one value, both reaching one function's caller |
| [ERR0718](ERR0718.md) | error | a `case a, b:` list in a `switch` over a union, whose arm binds one member's type |
| [ERR0719](ERR0719.md) | error | a queue `#send` written without `try … else`, or a `try` over a `latest` channel's `#send`, which cannot fail |
| [ERR0720](ERR0720.md) | error | a `for … in` over a channel itself rather than over `#peek` or `#drain` of it |
| [ERR0721](ERR0721.md) | error | `#peek` or `#drain` somewhere other than the head of a `for` — stored, passed, returned, indexed, or `#peek` as a statement — or `#release` written as a value |
| [ERR0722](ERR0722.md) | error | a `#slots` walk over a value with no declared slots to name, or a `break` or `continue` aimed at one |
| [ERR0723](ERR0723.md) | error | an `E!T` whose `E` is not an enum, is a C enum, or is backed by something other than `i32`, or one on a `foreign` function |
| [ERR0724](ERR0724.md) | error | a `fail` in a function marked `E!T` whose code is not known to be a member of `E` |
| [ERR0725](ERR0725.md) | error | a failure of another enum, or a plain failure, passed on by a function marked `E!T` |
| [ERR0730](ERR0730.md) | error | a `break` written in a `switch` arm, where C would leave the `switch` and Dado would leave the enclosing loop |
| [LNT0731](LNT0731.md) | lint | a `switch` over an `enum` whose arms leave a declared member unnamed |
| [ERR0801](ERR0801.md) | error | a key whose `==` is not stable and total, so an entry could be inserted and never found again |
| [ERR0802](ERR0802.md) | error | a `$K` bound to a type whose `==` is not stable and total, so the map it keys has no key |
| [ERR0803](ERR0803.md) | error | a view, a walk or a handle used after the block under it may have moved or been taken back — `#append`, `#resize`, `#reserve`, `#delete`, an insert, rebinding the handle, handing a `^` to it to a call, `#send`ing the `ref` to a channel, draining the channel a `#peek` lent it from, or ending the region it was minted in (a scratch frame rolled back, an arena's mark ended, an arena destroyed) — on some path through the same function |
| [ERR0804](ERR0804.md) | error | an assignment to `#default`, which is immutable within a scope — a nested suite shadows it with `using`, and nothing rebinds it |
| [ERR0805](ERR0805.md) | error | a spelling the language has retired, written where its replacement belongs — the message names the repair |
| [ERR0807](ERR0807.md) | error | a lifetime rung written on `resize`, which grows a block where it already lives and so takes no allocator at all |
| [ERR0808](ERR0808.md) | error | a `#stack` rung where there is no compound literal to emit — as a value, handed to a callee, on a verb that sizes its block at run time or allocates through the allocator's `proc`, or on a count only known at run time |
| [ERR0810](ERR0810.md) | error | a `using` subject that is not an allocator — the two words `(proc, data)` or an implementer of the `Allocator` trait — so there is nothing for the suite to allocate through |
| [ERR0813](ERR0813.md) | error | a `resize`, an `#append`, a `#reserve` or a `#truncate` on a by-value `ref` parameter — the callee changes its own copy of the handle and the caller is left holding the old one |
| [ERR0814](ERR0814.md) | error | `#cap`, `#append` or `#reserve` written over something that has no capacity distinct from its length — a fixed array, a borrow, a single-element reference or a map |
| [ERR0815](ERR0815.md) | error | a `#no_bounds` written where there is no bound to waive — a one-slot shape's sole position is `0`, so its check is what the subscript means rather than a guard around it, and a map read is total |
| [ERR0816](ERR0816.md) | error | a constant alignment that is not a power of two — the mask `(p + (n - 1)) & ~(n - 1)` is the alignment only for one, and a computed alignment is the programmer's word because the allocator interface carries one at run time |
| [ERR0817](ERR0817.md) | error | an atomic verb over something that is not a `^i32`, `^u32`, `^i64`, `^u64` or `^rawptr` — a width outside that set becomes a `libatomic` call, which is a link error on a freestanding target rather than a slow program |
| [ERR0818](ERR0818.md) | error | `#RELEASE` or `#ACQ_REL` as a compare-exchange's failure ordering — there is no store for it to release, and both C compilers reject it under the warnings this compiler builds with |
| [ERR0819](ERR0819.md) | error | a key in a place position on a by-value `{K: V}` parameter — an insert may reallocate the table and free the one the caller is still holding, and the caller has no way to be told |
| [ERR0820](ERR0820.md) | error | a map written while a loop is walking it — an insert, a `#delete`, rebinding the handle or handing `&m` to a call — other than the update of the entry being visited |
| [ERR0821](ERR0821.md) | error | `#stack` storage that outlives the block that minted it — returned, stored through a `^` parameter or into a package-level variable, or assigned to a binding in an enclosing block |
| [ERR0822](ERR0822.md) | error | a function whose body names `#caller`, taken as a value or exported, where no Dado call site is there to hand it the caller's allocator |
| [ERR0823](ERR0823.md) | error | a `#peek`, `#drain`, discard or `#release` of a channel inside a `for` already walking that channel |
| [ERR0824](ERR0824.md) | error | a place read after its `ref` moved into a channel by `#send`, on a path where the `#send` succeeded |
| [LNT0825](LNT0825.md) | lint | one package-level channel walked from two thread roots |
| [ERR0826](ERR0826.md) | error | a pointer, window or trait value of storage on this frame that outlives it — returned, stored through a parameter or into a package-level variable, appended into a handle one of those reaches, or read after its block ends |
| [ERR0827](ERR0827.md) | error | one call handed both a view into a block and the handle to it, where the callee may move the block and then read or write through the view |
| [ERR0828](ERR0828.md) | error | a `#truncate` to a count the compiler can see is negative, which is no length a run can have |
| [ERR0901](ERR0901.md) | error | an initializer that is not a compile-time constant |
| [ERR0902](ERR0902.md) | error | a division or remainder by a zero the compiler can see |
| [ERR0903](ERR0903.md) | error | a `when` condition over a constant more than one package declares, which import order would otherwise resolve |
| [ERR0904](ERR0904.md) | error | a template instantiated more times than the cap allows |
| [ERR0905](ERR0905.md) | error | two `@const` declarations of one name in one package |
| [ERR0906](ERR0906.md) | error | a `when` condition that is not a compile-time constant |
| [ERR0907](ERR0907.md) | error | `size` in a `when` condition, which asks the C compiler's number before a line of C exists |
| [ERR0908](ERR0908.md) | error | a `when` branching on `#OS` under a supplied triple whose operating system this compiler does not enumerate |
| [ERR0909](ERR0909.md) | error | a `$T` bound to a shape C has no name for, so the instantiation it asks for could not be emitted |
| [ERR0910](ERR0910.md) | error | an instantiation binding a shape built out of the one that asked for it, which is a chain with no end |
| [ERR0911](ERR0911.md) | error | a generic binding in a `when` condition, which selection runs too early to know |
| [ERR0912](ERR0912.md) | error | more type values evaluated in one compilation than the cap allows |
| [ERR0913](ERR0913.md) | error | a value where a type is wanted, or a type where a value is |
| [ERR0914](ERR0914.md) | error | a `type` parameter written somewhere a type value cannot be supplied or held |
| [ERR0915](ERR0915.md) | error | a compile-time value of a union two of whose members a folded constant cannot tell apart — the conversion's tag is lost in the fold, and two members of one sort leave nothing to recover it from |
| [ERR0916](ERR0916.md) | error | a `#LEN` whose operand's count lives in the value rather than in its type — a window's length is a run-time fact and `#len` is what asks for it |
| [ERR0917](ERR0917.md) | error | a `#lower` name in a `when` condition, which is the half of the `#` sigil that promises nothing at compile time |
| [ERR0918](ERR0918.md) | error | a `$` arity binder written where no signature can bind it — a `type` body, a package-level declaration or a local, none of which has a call to infer a count from |
| [ERR0919](ERR0919.md) | error | a call binding a template outside the domain its header `where` states — the predicate is asked of what this call bound, so the call is what is underlined |
| [ERR0920](ERR0920.md) | error | a `where` clause that is not a comptime predicate over the signature's bindings — it names a value, a word the compiler does not own, or nothing the signature binds at all |
| [ERR0921](ERR0921.md) | error | a `contract` written where a contract value would have to be stored or chosen while the program runs — it is compile-time known and erased, a `const` is where one lives, and `type` over the same shape is the run-time form |
| [ERR0922](ERR0922.md) | error | a template passing a value of a binding it holds as a shape to a template that asks that binding's declaration — one instantiation serves every declaration of the shape, so there is no declaration to hand over |
| [WRN0923](WRN0923.md) | warning | a package the program reaches that was written against POSIX with no `when #OS` arm for a non-POSIX target — `#WINDOWS` — so its C calls what that platform's C library does not promise |
| [ERR0930](ERR0930.md) | error | a `#static_assert` whose condition folds to `false` in this build |
| [ERR1001](ERR1001.md) | error | a transparent foreign type whose layout has no C spelling |
| [ERR1002](ERR1002.md) | error | a foreign type declared `@incomplete` held by value rather than behind a pointer |
| [ERR1003](ERR1003.md) | error | one C type declared twice by foreign blocks |
| [ERR1004](ERR1004.md) | error | a render the freestanding header set has nowhere to send |
| [ERR1005](ERR1005.md) | error | `export` written on a declaration that has no C symbol to promise |
| [ERR1006](ERR1006.md) | error | an `export` or `foreign` signature naming a shape C has no way to spell |
| [ERR1007](ERR1007.md) | error | two declarations promising one unmangled C symbol, or an `export` taking a name C, POSIX or this compiler already reserves |
| [ERR1009](ERR1009.md) | error | C's `...` in a native Dado signature, or a Dado pack in a `foreign` one |
| [ERR1010](ERR1010.md) | error | a `define` clause whose text is not a macro definition — `NAME` or `NAME=replacement` |
| [ERR1011](ERR1011.md) | error | one C macro given two different replacements by the `define` clauses of one program |
| [ERR1014](ERR1014.md) | error | a foreign slot or type whose C name is not one a C header could have given it |
| [ERR1016](ERR1016.md) | error | a `foreign` signature's `...` written where C has no ellipsis — before the end of the parameter list, or with no fixed parameter in front of it |
| [ERR1017](ERR1017.md) | error | `quit` on a target whose OS is `#NONE` or `#WEB`, which has no process to end and nobody to read the status |
| [ERR1018](ERR1018.md) | error | two slots of one transparent foreign type the emitted C would have to reach by one member name |
| [ERR1019](ERR1019.md) | error | a `#c.char` given to a Dado operator or to `print`, neither of which has an answer while its signedness is C's to choose — `#c.schar` and `#c.uchar` say which is meant and both convert |
| [ERR1021](ERR1021.md) | error | a value written on a `foreign enum` member, which C supplies, or a backing that is not an integer type |
| [ERR1022](ERR1022.md) | error | a `[_]T` written somewhere C has no flexible array member, or an operation on one whose answer would be the length nobody stated |
| [ERR1023](ERR1023.md) | error | a bitfield width written where C has none or with a base C cannot lay out in bits, a value that does not fit one, or an operation C refuses on one |
| [ERR1024](ERR1024.md) | error | a C union declared some other way, or an operation on one whose answer would be the discriminant C does not carry |
| [ERR1025](ERR1025.md) | error | an address or a window that would reach one type through its two C spellings — `&` on a member list a transparent `foreign` type wrote with a name, or a pointer, window or reference to a restatement standing where one to the shape it restates is wanted, or the reverse |
| [ERR1026](ERR1026.md) | error | an `@align` written on something with no storage to place, or given a number that is not a compile-time power of two |
| [ERR1027](ERR1027.md) | error | an `@symbol` written on something that emits no function symbol, or given a word that is not a registered symbol attribute or is written twice |
| [ERR1028](ERR1028.md) | error | a `@c_body` written where the compiler emits no C for the declaration under it, or one whose C reaches for a Dado local's emitted name |
| [ERR1029](ERR1029.md) | error | a `foreign` type whose C name is the one this compiler's mangling gives a shape the program holds, or a `foreign` function whose C name the function mangling can produce — two different things the emitted C would have to reach by one name |
| [ERR1030](ERR1030.md) | error | a `!` on a `@macro` foreign function, or a failable function type in a `foreign` signature |
| [ERR1031](ERR1031.md) | error | a `flags` argument that would reach past the package that wrote it — a warning flag no `#pragma GCC diagnostic` spells, or a codegen flag written away from the root package |
| [ERR1032](ERR1032.md) | error | a `foreign` block naming a header outside C's freestanding set, reached live on a target whose `#OS` is `#NONE` or `#WEB` |
| [ERR1033](ERR1033.md) | error | a `threads.spawn` reached by a `#WEB` build that was not built with `--web-threads` |
| [ERR1034](ERR1034.md) | error | a page template (`--html=`) whose `dado:` markers are missing, unknown, repeated or placed where their output cannot work |
| [ERR1101](ERR1101.md) | error | a `-D` naming no `@const` in the compilation |
| [ERR1102](ERR1102.md) | error | a `-D` value whose literal kind is not the one the declaration takes |
| [ERR1103](ERR1103.md) | error | a `-D` that does not name one `@const` — a bare name more than one package declares, or one declaration named twice by its two spellings |
| [ERR1201](ERR1201.md) | error | an argument whose type does not cross into the parameter it is passed to — a `#script_run` argument into a DadoScript parameter, or a script's argument into a Dado function's |
| [ERR1202](ERR1202.md) | error | a `#script_new` or `#script_reload` naming no DadoScript class, or a `#script_run` given something other than an instance's handle |
| [ERR1204](ERR1204.md) | error | a method called through a handle that the class lacks or given the wrong number of arguments, a `#script_new` giving `init` the wrong number, or a `#script_run` of a class with no `ready` or giving `ready` the wrong number |
| [ERR1205](ERR1205.md) | error | a `#script_*` verb in a build that can run no script — a `#NONE` target, or no DadoScript runtime in the compilation |
| [ERR1206](ERR1206.md) | error | a script's call of a Dado function whose signature cannot cross |
| [WRN1207](WRN1207.md) | warning | a script's argument narrowed into a Dado parameter, under `--strict`, where it is not a constant that fits |
| [ERR1208](ERR1208.md) | error | a Dado declaration named in a script whose type does not cross, a construction of one that is no shape, or a write to a slot of one a script holds as a tuple |
| [ERR1209](ERR1209.md) | error | a `#script_new`, `#script_stop`, `#script_pause`, `#script_resume`, `#script_is_running`, `#script_is_paused` or `#script_reload` given something other than a VM, or a `#script_vm` written with `using` |
| [ERR1210](ERR1210.md) | error | a retired script verb: `#script_load`, or `#script_run` given an entry name |
| [ERR1211](ERR1211.md) | error | a `using out:` or `using err:` after a call that runs no script |
| [ERR1212](ERR1212.md) | error | a `using` head whose sinks are misnamed, given twice, or followed by a positional subject |
| [ERR1213](ERR1213.md) | error | a `receive` given other than a package-level channel and a `void` function over its message's fields |
| [ERR1214](ERR1214.md) | error | a `receive` of a channel whose message carries a `ref` or a field that does not cross into a script |
| [ERR1215](ERR1215.md) | error | a `#script_stop` with no timeout, or a timeout that is not an integer number of milliseconds |
| [ERR1216](ERR1216.md) | error | a `#script_error()` outside the `else` of a `try` over a call into a script that can fail |
| [ERR1217](ERR1217.md) | error | an `on_error:` that is not `script.QUIT` or `script.CONTINUE` |
| [ERR1218](ERR1218.md) | error | a member of a script's instance named through its handle that the class does not have, or used as no field or method is |
| [ERR1219](ERR1219.md) | error | a script's field or answer whose DadoScript type has no Dado side, or an instance crossing beside a container |
| [ERR1220](ERR1220.md) | error | a Dado function a script passes a function to that keeps its bare function-reference parameter past the call |
| [ERR1223](ERR1223.md) | error | a `#script_reload` in a release build that does not enable reload |
| [ERR1224](ERR1224.md) | error | a reload's new code for a class the build does not carry, calling a Dado function the build does not bind or binds with another signature, dropping or changing a method the build's Dado runs or a field it reaches through a handle, or changing a kept field's type |
| [ERR1225](ERR1225.md) | error | a DadoScript feature a web build cannot run — `#script_reload`, `Script.load`, a call from Dado of an `async` method, or `#script_pause` single-threaded |
| [ERR1226](ERR1226.md) | error | a Dado call of a script's package function that names no published function, names two, or gives it what a function by its package does not take |
| [ERR1227](ERR1227.md) | error | an `#assert_raises` over a function that, or a function it reaches, does not raise into a script |
| [ERR1228](ERR1228.md) | error | an `#assert_raises` naming no function of its package, or written above something that is not one |
| [ERR1301](ERR1301.md) | error | a Dado construct written in DadoScript — memory management, a `#` spelling, an explicit width, the C boundary, or a word only Dado has |
| [ERR1302](ERR1302.md) | error | a DadoScript construct written in Dado code — a lambda, `async`, `await`, `assert`, `super`, a script's header, or a `foreign` block over JavaScript |
| [ERR1303](ERR1303.md) | error | a `.dados` file with no header whose basename is not an identifier, so it names no class, or an imported directory of scripts alone whose name is no identifier |
| [ERR1304](ERR1304.md) | error | an `init` that returns a value, a `ready` that answers something other than an `int` or nothing, or a `reloaded` that takes parameters or answers a value |
| [ERR1305](ERR1305.md) | error | a method no DadoScript array, map or string has — another language's spelling, or one DadoScript does not admit — or a method called on a scalar |
| [ERR1310](ERR1310.md) | error | a class rule broken — `extends` naming a Dado type or closing a cycle, an override of another shape, `super` outside a subclass or `super(…)` outside `init`, or a class-typed slot with no value |
| [ERR1320](ERR1320.md) | error | a JavaScript module bound under no identifier or imported unqualified, a module importing a package manager's name, or a `foreign "x.js":` declaration a script cannot call |
| [ERR1330](ERR1330.md) | error | `await` outside an `async` method or over a value that is not `async`, an `async` call left alone on a line, an `async` slot with no value, or an `async` `init` or generic method |
| [ERR1340](ERR1340.md) | error | an `export` in DadoScript that publishes a type with no declaration spelling, a class that is not exported, an inner class or generic method, a name twice, or a method reading `self` of a class that is not exported |
| [ERR1341](ERR1341.md) | error | a script of a JavaScript library package that calls a Dado function or loads a script by its path |
| [ERR1350](ERR1350.md) | error | `slots(v)` outside a `for` header, over a value that is no instance, or where a member or function of that name could mean it; or `T.names()` where an inner class named `names` could mean it |
