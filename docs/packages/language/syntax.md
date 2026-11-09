<!-- dadoc 1.0.0-rc.9 -->
<!-- self-hosted (tools/dadoc) -->
# language:syntax

Items and declarations: types, signatures, attributes, channels, and a
script's classes.

## Declarations

738 declarations, 451 public.

* `u32 package_decl(^Parser p)`
* `u32 import_decl(^Parser p)`
* `u32 visibility(^Parser p)`
* `string8 vis_spelling(u32 v)`
* `u32 take_visibility_word(^Parser p, u32 word)`
* `u32 visibility_word(^Parser p)`
* `u32 item(^Parser p)`
* `u32 static_assert_item(^Parser p)`
* `u32 assert_raises_item(^Parser p)`
* `u32 take_pending_item(^Parser p)`
* `bool at_freestanding_mark(^Parser p)`
* `bool at_build_const(^Parser p)`
* `bool at_test_marker(^Parser p)`
* `u32 test_marker(^Parser p)`
* `bool at_union_head(^Parser p)`
* `bool at_trait_head(^Parser p)`
* `bool at_cunion_head(^Parser p)`
* `bool at_contract_head(^Parser p)`
* `bool at_class_head(^Parser p, i64 skip)`
* `bool at_script_header(^Parser p)`
* `bool at_exported_script_header(^Parser p)`
* `bool at_qualifier(^Parser p)` — Whether the token ahead is a type qualifier in front of a type.
* `bool starts_type(u8 k)`
* `u32 trait_decl(^Parser p, u32 vis)`
* `u32 trait_line(^Parser p)`
* `u32 contract_decl(^Parser p, u32 vis)`
* `u32 union_decl(^Parser p, u32 vis, u32 dist)`
* `u32 cunion_decl(^Parser p, u32 vis, bool fgn)`
* `bool at_member_comma(^Parser p)`
* `void member_comma(^Parser p)`
* `string8 failure_stored()`
* `u32 fn_or_var_decl(^Parser p, u32 vis)`
* `u32 const_decl(^Parser p, u32 vis, u32 sigil)`
* `u32 enum_decl(^Parser p, u32 vis, u32 dist)`
* `u32 type_decl(^Parser p, u32 vis, u32 dist)`
* `u32 binder_list(^Parser p)` — `($T, $U)` after a type's or trait's name: pushes N_BINDERs.
* `u32 decl_tail(^Parser p, u32 kind, u32 name)` — `extends A, B` / `using C` (kind 1 type, 2 trait, 3 class): pushes the…
* `u32 type_body_with_methods(^Parser p)` — The body after `type Name:`; methods are pushed after it is answered.
* `void method_block(^Parser p, bool keep)` — Methods indented under a head line that wrote the shape; pushed when…
* `u32 type_body(^Parser p, bool fgn)`
* `bool tuple_return_ahead(^Parser p)`
* `void misplaced_method_block(^Parser p)`
* `bool at_plain_method_line(^Parser p)`
* `u32 member_list_parenthesized(^Parser p, bool fgn)`
* `u32 member_list_inline(^Parser p, bool fgn)`
* `u32 member_list(^Parser p, bool fgn, bool methods)` — An indented member list; with `methods`, the methods it holds are pushed…
* `u32 member_line_promised(^Parser p, bool fgn, bool methods)`
* `bool listed_promise_refused(^Parser p)`
* `u32 member_line(^Parser p, bool fgn)` — A slot (N_MEMBER) or a method (N_FN).
* `bool nested_declaration_refused(^Parser p)`
* `void method_misplaced(^Parser p, u32 m)`
* `u32 member(^Parser p, bool fgn)`
* `u32 member_after(^Parser p, bool fgn, u32 align, u32 ty)`
* `bool at_anonymous_cunion(^Parser p)`
* `u32 anonymous_cunion(^Parser p, bool fgn)`
* `u32 bit_width(^Parser p, bool fgn)`
* `u32 fn_decl_after(^Parser p, u32 vis, u32 lo, u32 ret, u32 fl, u32 name, u32 symbol)`
* `u32 signature(^Parser p, i64 base, bool method)` — `(receiver?, params...) where?`: the receiver and constraint go in the…
* `u32 where_clause(^Parser p)`
* `u32 receiver(^Parser p)`
* `u32 method_after(^Parser p, u32 vis, u32 lo, u32 ret, u32 fl, u32 name, bool in_trait)` — A method's signature and body; a trait's may be bodiless (F_REQUIRED).
* `u32 param(^Parser p, bool defaults)`
* `u32 param_default(^Parser p, bool allowed)`
* `type Place` — A line and a column (in characters), from 1.
* `type Liner` — Walks a text forward to answer places; moving back starts it over.
* `Place Liner.place(^self, i64 at)`
* `void put(^ref []char8 out, string8 s)`
* `void put_num(^ref []char8 out, i64 n)`
* `void put_debug(^ref []char8 out, string8 s)` — `s` as Rust's `{:?}` quotes a `str`.
* `void dump_tokens(^ref []char8 out, string8 name, string8 src, ^Tokens t, i64 base = 0, bool followed = false)` — One file's `--emit=tokens` block, `name` in its header. `base` is where the…
* `void render_diags(^ref []char8 out, string8 name, string8 src, ^Tokens t, bool followed = false)` — The diagnostics in dadoc's human form: header, excerpt, caret, help.
* `void render_parse_diags(^ref []char8 out, string8 name, string8 src, ^Ast a, bool followed = false)`
* `void render_list(^ref []char8 out, string8 name, string8 src, []Diag diags, []char8 text, bool followed)`
* `u8 binary_power(u8 k)`
* `bool is_range(u8 k)`
* `bool is_assign(u8 k)`
* `u32 expr(^Parser p)`
* `u32 expr_bp(^Parser p, u8 min)`
* `u32 conditional_tail(^Parser p, u32 then)`
* `bool at_is(^Parser p)`
* `bool at_has(^Parser p)`
* `u32 prefix(^Parser p)`
* `u32 primary(^Parser p)`
* `bool names_a_slot(^Parser p, u32 ty)` — Whether a tuple type names a slot (a declaration that binds its slots).
* `u32 type_arg(^Parser p)`
* `u32 operand_marker(^Parser p)` — `#no_bounds` in front of an index or a stored value: its token, or NONE.
* `u32 postfix(^Parser p, u32 e)`
* `u32 arg_list(^Parser p)` — Pushes the arguments (E_ARG); BAD on failure.
* `u32 head_expr(^Parser p)`
* `u32 keyword_name(^Parser p, string8 what, u8 e1, u8 e2, u8 e3, u8 e4)` — A keyword written as a name, when the next token is one of `e1`..`e4`:
* `u32 keyword_use(^Parser p)`
* `void string_body(string8 w, ^ref []char8 out)` — dado_ast's string_body: a literal's text between its quotes, a…
* `void check_string(^Parser p, u32 t)` — dado_ast's literal_faults (ERR0112) and, for a multi-line literal,…
* `bool c_identifier(string8 s)`
* `bool c_type_name(string8 s)`
* `u32 c_name_attr(^Parser p, u32 kind, bool allowed)` — A slot's `@c("…")` (kind 0), a type's (1), an enumerator's (2), a function's (3).
* `u32 slot_c_name(^Parser p, bool fgn)`
* `u32 foreign_type_c_name(^Parser p)`
* `u32 align_attr(^Parser p)`
* `u32 foreign_symbol_attr(^Parser p)` — `@symbol(…)` in a `foreign` block: refused; BAD when written.
* `u32 symbol_attr(^Parser p)`
* `u32 thread_local_attr(^Parser p)`
* `bool at_c_body(^Parser p)`
* `u32 c_body_attr(^Parser p)`
* `bool at_channel_head(^Parser p)`
* `u32 channel_item(^Parser p, u32 vis)`
* `u32 channel_member(^Parser p)`
* `void channel_misplaced(^Parser p, string8 what)`
* `u32 foreign_block(^Parser p, u32 vis, u32 free)`
* `void item_hashes(^Ast a, string8 src, ^Tokens t)`
* `const [53]string8 WORDS = ("public", "private", "export", "switch", "case", "defer", "pass", "fallthrough", "quit", "union", "contract", "test", "when", "cunion", "where", "is", "has", "c", "incomplete", "align", "symbol", "macro", "freestanding", "thread_local", "c_body", "shapeof", "typeof", "ref", "trait", "extends", "using", "class", "assert", "self", "fail", "try", "latest", "out", "err", "volatile", "restrict", "_Atomic", "noinline", "no_sanitize_address", "define", "flags", "link", "#static_assert", "#assert_raises", "#channel", "#signal", "const", "file")`
* `const u32 W_PUBLIC = 0`
* `const u32 W_PRIVATE = 1`
* `const u32 W_EXPORT = 2`
* `const u32 W_SWITCH = 3`
* `const u32 W_CASE = 4`
* `const u32 W_DEFER = 5`
* `const u32 W_PASS = 6`
* `const u32 W_FALLTHROUGH = 7`
* `const u32 W_QUIT = 8`
* `const u32 W_UNION = 9`
* `const u32 W_CONTRACT = 10`
* `const u32 W_TEST = 11`
* `const u32 W_WHEN = 12`
* `const u32 W_CUNION = 13`
* `const u32 W_WHERE = 14`
* `const u32 W_IS = 15`
* `const u32 W_HAS = 16`
* `const u32 W_C = 17`
* `const u32 W_INCOMPLETE = 18`
* `const u32 W_ALIGN = 19`
* `const u32 W_SYMBOL = 20`
* `const u32 W_MACRO = 21`
* `const u32 W_FREESTANDING = 22`
* `const u32 W_THREAD_LOCAL = 23`
* `const u32 W_C_BODY = 24`
* `const u32 W_SHAPEOF = 25`
* `const u32 W_TYPEOF = 26`
* `const u32 W_REF = 27`
* `const u32 W_TRAIT = 28`
* `const u32 W_EXTENDS = 29`
* `const u32 W_USING = 30`
* `const u32 W_CLASS = 31`
* `const u32 W_ASSERT = 32`
* `const u32 W_SELF = 33`
* `const u32 W_FAIL = 34`
* `const u32 W_TRY = 35`
* `const u32 W_LATEST = 36`
* `const u32 W_OUT = 37`
* `const u32 W_ERR = 38`
* `const u32 W_VOLATILE = 39`
* `const u32 W_RESTRICT = 40`
* `const u32 W_ATOMIC = 41`
* `const u32 W_NOINLINE = 42`
* `const u32 W_NO_SANITIZE = 43`
* `const u32 W_DEFINE = 44`
* `const u32 W_FLAGS = 45`
* `const u32 W_LINK = 46`
* `const u32 H_STATIC_ASSERT = 47`
* `const u32 H_ASSERT_RAISES = 48`
* `const u32 H_CHANNEL = 49`
* `const u32 H_SIGNAL = 50`
* `const u32 W_WORDS = 53`
* `types.Interner names()` — The one string interner (`types.Interner`), holding the positional words…
* `void intern(^Tokens t, string8 src, ^types.Interner n)` — Fills `t.sym` with the id of each IDENT and HASH_NAME token.
* `type Diag` — A diagnostic. Its message is `text[msg..<mend]`, its help `text[help..<stop]`.
* `type Tokens` — One file's tokens (struct of arrays) and lex diagnostics.
* `void Tokens.free(^self)`
* `i64 Tokens.count(^self)`
* `string8 Tokens.message(^self, Diag d)`
* `string8 Tokens.help(^self, Diag d)`
* `Tokens lex(string8 src)` — Lexes `src`, answering its tokens and diagnostics. The caller frees them.
* `u32 token_end(string8 src, u8 t, u32 start)` — Where the token at `start` with tag `t` ends.
* `i64 token_at(^Tokens t, u32 at)` — The token at byte `at`: the first whose end is past it, so the one holding…
* `string8 describe(u8 t)` — How a tag is written in prose: "`(`".
* `i64 matching_close([]u8 t, i64 open)`
* `bool declares_a_parameter([]u8 t, i64 open)` — Whether the parameter list opened at `open` declares a first parameter:
* `(u64 value, bool ok) int_value(string8 text)` — An integer literal's value; `ok` is false past u64's range. A malformed…
* `u32 char_value(string8 text)` — A character literal's code point; 0 for one the lexer refused.
* `void string_value(string8 written, ^ref []char8 out)` — Appends a string literal's text to `out`: quotes off, a multi-line one's…
* `void unescape(string8 body, ^ref []char8 out)` — `body` with its escapes resolved, appended to `out`; a bad escape is kept.
* `(u64 value, bool ok) int_literal(string8 text)` — dado_parse's parse_int: the value of an integer literal read as Rust's…
* `bool at_misplaced_try(^Parser p)`
* `u32 misplaced_try(^Parser p)`
* `bool misplaced_try_since(^Parser p, i64 before)`
* `u32 tried_as_meant(^Parser p, u32 s, i64 before)`
* `bool recover_statement(^Parser p, i64 before)`
* `u32 type_or_misread_conversion(^Parser p)` — A type, or a refused `[]f64(d)`; NONE when neither reads.
* `void suggested_name(string8 named, ^ref []char8 out)` — A name for written type `named`, for the misread conversion's repair.
* `u32 script_header(^Parser p)`
* `u32 class_decl(^Parser p, u32 vis)`
* `void unparse_type(^Parser p, u32 t, ^ref []char8 out)` — dado_parse's unparse_type, an expression written as its source text.
* `const u32 BAD = 0xFFFFFFFE`
* `` const [12]string8 LAYOUT_WORDS = ("end of line", "an indented block", "the end of an indented block", "end of file", "an identifier", "a `#` compiler definition", "an integer literal", "a float literal", "a string literal", "a character literal", "", "") ``
* `string8 describe_tag(u8 t, ^ref []char8 out)` — How the grammar names a token kind in a message: "`(`", "an identifier".
* `Ast parse(string8 src, ^Tokens t, bool dados = false)` — Parses one file's tokens (interned: `intern`); `dados` is a `.dados` file.
* `bool test_header(string8 src)` — dado_span's test_header: whether the first line is `@test`, optionally a status.
* `string8 trim(string8 s)`
* `u32 block(^Parser p)`
* `u32 stmt(^Parser p)`
* `u32 if_stmt(^Parser p)`
* `bool at_postfix_suite_word(^Parser p)`
* `u32 using_head(^Parser p, bool suite)` — An E_USING: the allocator, then named sinks.
* `bool at_when_head(^Parser p)`
* `u32 when_item(^Parser p)`
* `u32 let_assign_or_expr(^Parser p, u32 c_body)` — A declaration, an assignment, or an expression statement.
* `const u8 NEWLINE = 0`
* `const u8 INDENT = 1`
* `const u8 DEDENT = 2`
* `const u8 EOF = 3`
* `const u8 IDENT = 4`
* `const u8 HASH_NAME = 5`
* `const u8 INT = 6`
* `const u8 FLOAT = 7`
* `const u8 STRING = 8`
* `const u8 CHAR = 9`
* `const u8 KW_PACKAGE = 10`
* `const u8 KW_IMPORT = 11`
* `const u8 KW_TYPE = 12`
* `const u8 KW_DISTINCT = 13`
* `const u8 KW_CONST = 14`
* `const u8 KW_ENUM = 15`
* `const u8 KW_FOREIGN = 16`
* `const u8 KW_VIEW = 17`
* `const u8 KW_FOR = 18`
* `const u8 KW_IN = 19`
* `const u8 KW_IF = 20`
* `const u8 KW_ELSE = 21`
* `const u8 KW_RETURN = 22`
* `const u8 KW_BREAK = 23`
* `const u8 KW_CONTINUE = 24`
* `const u8 KW_ASYNC = 25`
* `const u8 KW_AWAIT = 26`
* `const u8 KW_SUPER = 27`
* `const u8 LPAREN = 28`
* `const u8 RPAREN = 29`
* `const u8 LBRACKET = 30`
* `const u8 RBRACKET = 31`
* `const u8 LBRACE = 32`
* `const u8 RBRACE = 33`
* `const u8 COMMA = 34`
* `const u8 COLON = 35`
* `const u8 DOT = 36`
* `const u8 AT = 37`
* `const u8 QUESTION = 38`
* `const u8 DOTDOT = 39`
* `const u8 DOTDOT_LESS = 40`
* `const u8 DOTDOT_EQ = 41`
* `const u8 ASSIGN = 42`
* `const u8 PLUS_EQ = 43`
* `const u8 MINUS_EQ = 44`
* `const u8 STAR_EQ = 45`
* `const u8 SLASH_EQ = 46`
* `const u8 PERCENT_EQ = 47`
* `const u8 AMP_EQ = 48`
* `const u8 PIPE_EQ = 49`
* `const u8 TILDE_EQ = 50`
* `const u8 AMP_TILDE_EQ = 51`
* `const u8 SHL_EQ = 52`
* `const u8 SHR_EQ = 53`
* `const u8 PLUS = 54`
* `const u8 MINUS = 55`
* `const u8 STAR = 56`
* `const u8 SLASH = 57`
* `const u8 PERCENT = 58`
* `const u8 PLUS_PLUS = 59`
* `const u8 EQ_EQ = 60`
* `const u8 BANG_EQ = 61`
* `const u8 LT = 62`
* `const u8 LT_EQ = 63`
* `const u8 GT = 64`
* `const u8 GT_EQ = 65`
* `const u8 AMP_AMP = 66`
* `const u8 PIPE_PIPE = 67`
* `const u8 BANG = 68`
* `const u8 AMP = 69`
* `const u8 PIPE = 70`
* `const u8 TILDE = 71`
* `const u8 AMP_TILDE = 72`
* `const u8 SHL = 73`
* `const u8 SHR = 74`
* `const u8 CARET = 75`
* `const u8 DOLLAR = 76`
* `const u8 TAG_COUNT = 77`
* `const [77]string8 TAG_NAMES = ("NEWLINE", "INDENT", "DEDENT", "EOF", "IDENT", "HASH_NAME", "INT", "FLOAT", "STRING", "CHAR", "KW_PACKAGE", "KW_IMPORT", "KW_TYPE", "KW_DISTINCT", "KW_CONST", "KW_ENUM", "KW_FOREIGN", "KW_VIEW", "KW_FOR", "KW_IN", "KW_IF", "KW_ELSE", "KW_RETURN", "KW_BREAK", "KW_CONTINUE", "KW_ASYNC", "KW_AWAIT", "KW_SUPER", "LPAREN", "RPAREN", "LBRACKET", "RBRACKET", "LBRACE", "RBRACE", "COMMA", "COLON", "DOT", "AT", "QUESTION", "DOTDOT", "DOTDOT_LESS", "DOTDOT_EQ", "ASSIGN", "PLUS_EQ", "MINUS_EQ", "STAR_EQ", "SLASH_EQ", "PERCENT_EQ", "AMP_EQ", "PIPE_EQ", "TILDE_EQ", "AMP_TILDE_EQ", "SHL_EQ", "SHR_EQ", "PLUS", "MINUS", "STAR", "SLASH", "PERCENT", "PLUS_PLUS", "EQ_EQ", "BANG_EQ", "LT", "LT_EQ", "GT", "GT_EQ", "AMP_AMP", "PIPE_PIPE", "BANG", "AMP", "PIPE", "TILDE", "AMP_TILDE", "SHL", "SHR", "CARET", "DOLLAR")` — Dump names (`--emit=tokens`), by tag.
* `const [77]string8 SPELLINGS = ("", "", "", "", "", "", "", "", "", "", "package", "import", "type", "distinct", "const", "enum", "foreign", "view", "for", "in", "if", "else", "return", "break", "continue", "async", "await", "super", "(", ")", "[", "]", "{", "}", ",", ":", ".", "@", "?", "..", "..<", "..=", "=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "~=", "&~=", "<<=", ">>=", "+", "-", "*", "/", "%", "++", "==", "!=", "<", "<=", ">", ">=", "&&", "||", "!", "&", "|", "~", "&~", "<<", ">>", "^", "$")` — Fixed spellings, by tag; "" where the text varies or there is none.
* `string8 tag_name(u8 t)`
* `string8 spelling(u8 t)` — The fixed text of tag `t`, or "".
* `bool is_layout(u8 t)`
* `bool is_keyword(u8 t)`
* `bool has_text(u8 t)` — Whether a token of this tag carries text worth printing (names, literals).
* `bool opens(u8 t)`
* `bool closes(u8 t)`
* `u8 keyword(string8 w)` — The keyword spelled by `w`, or IDENT.
* `const u32 NONE = 0xFFFFFFFF`
* `const u8 S_LEAF = 0` — Shapes: what `a` and `b` hold.
* `const u8 S_A = 1`
* `const u8 S_AB = 2`
* `const u8 S_L = 3` — `b` is a list; `a` is flags.
* `const u8 S_B = 4`
* `const u8 N_FILE = 0` — Layouts. main is a token; lists are written [slots..., rest...].
* `const u8 N_TEST = 1`
* `const u8 N_PACKAGE = 2`
* `const u8 N_SCRIPT = 3`
* `const u8 N_IMPORT = 4`
* `const u8 N_FN = 5`
* `const u8 N_RECEIVER = 6`
* `const u8 N_PARAM = 7`
* `const u8 N_TYPE = 8`
* `const u8 N_TRAIT = 9`
* `const u8 N_CONST = 10`
* `const u8 N_VAR = 11`
* `const u8 N_ENUM = 12`
* `const u8 N_ENUM_MEMBER = 13`
* `const u8 N_UNION = 14`
* `const u8 N_FOREIGN = 15`
* `const u8 N_CLAUSE = 16`
* `const u8 N_F_FN = 17`
* `const u8 N_F_OPAQUE = 18`
* `const u8 N_F_CONST = 19`
* `const u8 N_F_VAR = 20`
* `const u8 N_F_ENUM = 21`
* `const u8 N_F_ENUM_MEMBER = 22`
* `const u8 N_WHEN = 23`
* `const u8 N_WHEN_ARM = 24`
* `const u8 N_WHEN_ELSE = 25`
* `const u8 N_STATIC_ASSERT = 26`
* `const u8 N_ASSERT_RAISES = 27`
* `const u8 N_CLASS = 28`
* `const u8 N_MEMBERS = 29`
* `const u8 N_MEMBER = 30`
* `const u8 N_BINDER = 31`
* `const u8 N_ALIGN = 32`
* `const u8 N_SYMBOL = 33`
* `const u8 N_C_NAME = 34`
* `const u8 N_C_BODY = 35`
* `const u8 N_WIDTH = 36`
* `const u8 N_WORD = 37`
* `const u8 N_ALIAS = 38`
* `const u8 N_WHERE = 39`
* `const u8 N_THREAD_LOCAL = 40`
* `const u8 T_NAME = 41` — types…
* `const u8 T_PATH = 42`
* `const u8 T_POINTER = 43`
* `const u8 T_REF = 44`
* `const u8 T_SLICE = 45`
* `const u8 T_ARRAY = 46`
* `const u8 T_MATRIX = 47`
* `const u8 T_SHAPEOF = 48`
* `const u8 T_MAP = 49`
* `const u8 T_TUPLE = 50`
* `const u8 T_FN = 51`
* `const u8 T_BOUND = 52`
* `const u8 T_BINDING = 53`
* `const u8 T_KIND = 54`
* `const u8 T_LAYOUT = 55`
* `const u8 T_QUAL = 56`
* `const u8 T_PACK = 57`
* `const u8 T_CHANNEL = 58`
* `const u8 E_INT = 59` — expressions…
* `const u8 E_FLOAT = 60`
* `const u8 E_STR = 61`
* `const u8 E_CHAR = 62`
* `const u8 E_NAME = 63`
* `const u8 E_TUPLE = 64`
* `const u8 E_ARRAY = 65`
* `const u8 E_MAP = 66`
* `const u8 E_ENTRY = 67`
* `const u8 E_IN = 68`
* `const u8 E_IS = 69`
* `const u8 E_HAS = 70`
* `const u8 E_CALL = 71`
* `const u8 E_ARG = 72`
* `const u8 E_FIELD = 73`
* `const u8 E_INDEX = 74`
* `const u8 E_SLICE = 75`
* `const u8 E_UNARY = 76`
* `const u8 E_DEREF = 77`
* `const u8 E_BINARY = 78`
* `const u8 E_SPLAT = 79`
* `const u8 E_RANGE = 80`
* `const u8 E_TYPE = 81`
* `const u8 E_NARROW = 82`
* `const u8 E_COND = 83`
* `const u8 E_LAMBDA = 84`
* `const u8 E_USING = 85`
* `const u8 S_BLOCK = 86` — statements…
* `const u8 S_RETURN = 87`
* `const u8 S_VIEW = 88`
* `const u8 S_FOR = 89`
* `const u8 S_FOR_IN = 90`
* `const u8 S_BIND = 91`
* `const u8 S_IF = 92`
* `const u8 S_SWITCH = 93`
* `const u8 S_CASE = 94`
* `const u8 S_DEFER = 95`
* `const u8 S_BREAK = 96`
* `const u8 S_CONTINUE = 97`
* `const u8 S_PASS = 98`
* `const u8 S_FALLTHROUGH = 99`
* `const u8 S_QUIT = 100`
* `const u8 S_FAIL = 101`
* `const u8 S_TRY = 102`
* `const u8 S_TRY_ELSE = 103`
* `const u8 S_LET = 104`
* `const u8 S_ASSIGN = 105`
* `const u8 S_EXPR = 106`
* `const u8 S_WHEN = 107`
* `const u8 S_WHEN_ARM = 108`
* `const u8 S_SUITE = 109`
* `const u8 S_AWAIT = 110`
* `const u8 S_ASSERT = 111`
* `const u8 N_PARAM_V = 112`
* `const u8 N_SIGNAL = 113`
* `const u8 N_TAGS = 114`
* `const [114]u8 SHAPES = (3, 0, 0, 3, 0, 3, 0, 2, 3, 3, 3, 3, 3, 1, 3, 3, 0, 3, 1, 1, 1, 3, 1, 3, 3, 3, 2, 3, 3, 3, 3, 0, 1, 3, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 2, 3, 1, 2, 3, 3, 0, 0, 0, 1, 1, 0, 3, 0, 0, 0, 0, 0, 3, 3, 3, 2, 2, 2, 3, 3, 1, 1, 2, 3, 1, 1, 2, 1, 2, 1, 2, 3, 3, 3, 3, 1, 3, 2, 3, 0, 3, 3, 3, 1, 0, 0, 0, 0, 1, 1, 2, 1, 3, 3, 1, 3, 2, 2, 1, 2, 2, 3)`
* `const u32 F_VIS = 7` — Flags (in `a` of declarations).
* `const u32 F_DISTINCT = 8`
* `const u32 F_CONTRACT = 16`
* `const u32 F_RAW = 32`
* `const u32 F_PAREN = 64`
* `const u32 F_EMBED = 128`
* `const u32 F_BUILD = 256`
* `const u32 F_FAILABLE = 512`
* `const u32 F_ASYNC = 1024`
* `const u32 F_REQUIRED = 2048`
* `const u32 F_MACRO = 4096`
* `const u32 F_CVAR = 8192`
* `const u32 F_FREE = 16384`
* `const u32 F_INCOMPLETE = 32768`
* `const u32 F_VARIADIC = 65536`
* `const u32 F_TYPEOF = 131072`
* `const u32 F_UNQUALIFIED = 262144`
* `const u32 VIS_UNMARKED = 0`
* `const u32 VIS_PUBLIC = 1`
* `const u32 VIS_PACKAGE = 2`
* `const u32 VIS_FILE = 3`
* `const u32 VIS_EXPORT = 4`
* `type Ast` — One file's tree.
* `void Ast.free(^self)`
* `i64 Ast.count(^self)`
* `string8 Ast.message(^self, Diag d)`
* `string8 Ast.help(^self, Diag d)`
* `[]u32 Ast.list(^self, u32 x)` — The entries of list `x` (an index into extra).
* `void Ast.kids(^self, u32 n, ^ref []u32 out)` — Node `n`'s children, absent ones skipped, appended to `out`.
* `void dump_tree(^ref []char8 out, string8 src, ^Tokens t, ^Ast a)` — Appends the dump of `a` to `out`.
* `u32 type_expr(^Parser p)`
* `u32 async_value(^Parser p, u32 mark, u32 ty)`
* `void async_misplaced(^Parser p, u32 mark)`
* `u32 type_expr_marked(^Parser p)` — A declaration head's type: its `!` mark goes to tm_lo/tm_hi, its `E!`
* `u32 type_core(^Parser p)`
