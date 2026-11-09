//@ types/views 33
//@ ### Bytes and text are views of each other
//@
//@ A `string8`, a `[]char8` and a `[]u8` are the same two words — a
//@ pointer to 8-bit units and a count — and a conversion between any
//@ two of them is a **view**: the same bytes under the other type, no
//@ copy, nothing read. `string8(b)` of a `[]u8` or a `[]char8` is text;
//@ a window is reached through its name, since `[]char8(b)` written in
//@ place reads as a function type: `Chars(b)` for `type Chars: []char8`,
//@ `Bytes(s)` for `type Bytes: []u8`, either way. `const` elements are
//@ kept and never dropped, so a `[]const u8` becomes a `[]const char8` or
//@ a `string8`, and a `[]char8` from it is refused (`ERR0504`). A
//@ `ref []u8` converts through the window it lends and still owns its
//@ block, and the result is a view of that block like any other: read
//@ after the block moves or is freed, it is warned of (`WRN0803`), and
//@ of storage on the frame it does not outlive the frame (`ERR0826`).
//@
//@ ```dado
//@ package doc_sample
//@
//@ type Chars: []char8
//@
//@ public string8 as_text([]u8 b):
//@     return string8(b)
//@
//@ public []char8 as_chars([]u8 b):
//@     return Chars(b)
//@ ```
//@
//@ **A view conversion never checks that the bytes are UTF-8**: it is
//@ O(1) and never fails, so it cannot read them. A `string8` is UTF-8 by
//@ convention, and nothing relies on it for safety: `#len` counts bytes,
//@ `for` walks units, `==` and a map key compare bytes, and printing
//@ writes them as they are. `core:chars`' decoders answer U+FFFD for a
//@ malformed sequence and move on. Bytes from outside the program that
//@ must be text are asked first, with `chars.valid(s)`.
//@
//@ **A string literal becomes a `cstring` where one is wanted, and then it
//@ holds no NUL.** A `cstring` is a pointer and a terminator with no length
//@ beside it, so C reads it to its first NUL: `cstring s = "a\0b"` would
//@ hand C `"a"` and lose the rest, and it is refused (`ERR0540`). The same
//@ literal fits a `string8`, a `string16` or a `string32`, whose length
//@ counts the NUL as one more unit of text.
