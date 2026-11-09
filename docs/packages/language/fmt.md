<!-- dadoc 1.0.0-rc.9 -->
<!-- self-hosted (tools/dadoc) -->
# language:fmt

`dado fmt` over language:syntax: unparse, format and align in one walk,
with the trivia beside it.

## Declarations

198 declarations, 126 public.

* `void suite_as(^F u, i64 policy, u32 he, u32 b)`
* `void block(^F u, u32 b)`
* `void stmt(^F u, u32 s)`
* `void expr(^F u, u32 e)`
* `void type_expr(^F u, u32 t)`
* `void channel_named(^F u, u32 t, u32 name)`
* `const u8 W_STMT = 0` — What a measured line renders (`within`), and a member list's members.
* `const u8 W_CONST = 1`
* `const u8 W_VAR = 2`
* `const u8 W_SASSERT = 3`
* `const u8 W_ARAISES = 4`
* `const u8 W_FFN = 5`
* `const u8 W_HEADER = 6`
* `const u8 L_LIST = 0`
* `const u8 L_UNION = 1`
* `const u8 L_ENUM = 2`
* `const u8 L_FENUM = 3`
* `const u8 L_TYPE = 4`
* `void format(string8 src, ^syntax.Tokens t, ^syntax.Ast a, ^const Style s, ^ref []char8 out)` — Formats `src` (parsed as `t` and `a`) under `s` into `out`.
* `void const_decl(^F u, u32 n)`
* `void c_body_attr(^F u, u32 n)`
* `void param(^F u, u32 p, bool default)`
* `void member(^F u, u32 m, bool own_line)`
* `const u32 NONE = 0xFFFFFFFF`
* `const u8 STOP_MEMBER = 0`
* `const u8 STOP_ENUM = 1`
* `const u8 STOP_COMMENT = 2`
* `type Mark`
* `type F`
* `i64 F.k(^self, i64 key)`
* `string8 F.text(^self, u32 tok)`
* `u32 F.tend(^self, u32 tok)`
* `[]u32 F.kids(^self, u32 n)`
* `u8 F.tag(^self, u32 n)`
* `void F.put(^self, string8 s)`
* `void F.ch(^self, char8 c)`
* `void F.tok(^self, u32 t)`
* `void F.sp_tok(^self, u32 t)`
* `void F.tok_sp(^self, u32 t)`
* `void F.num(^self, u32 v)`
* `bool F.ends(^self, string8 s)`
* `void F.pop(^self)`
* `void F.end_line(^self)`
* `void F.spaces(^self, i64 n)`
* `void F.comma(^self)`
* `void F.colon(^self)`
* `void F.decl_eq(^self)`
* `void F.infix(^self, string8 op)`
* `void F.infix_tok(^self, u32 t)`
* `void F.span(^self, u32 lo, u32 hi)`
* `void F.infix_word(^self, string8 w)`
* `i64 F.inside(^self, char8 b)`
* `void F.opens(^self, char8 b, bool empty)`
* `void F.closes(^self, char8 b, bool empty)`
* `void F.stop(^self, u8 kind)`
* `void F.indent(^self)`
* `i64 F.ntv(^self)`
* `u32 F.tv_lo(^self, i64 i)`
* `u32 F.tv_hi(^self, i64 i)`
* `bool F.is_comment(^self, i64 i)`
* `string8 F.tv_text(^self, i64 i)` — A comment's text, trailing blanks trimmed.
* `bool F.same_line(^self, u32 x, u32 y)`
* `i64 F.take_before(^self, u32 before)` — Claims the trivia before `before`; answers where the claim began.
* `bool F.pending_comment_before(^self, u32 before)`
* `i64 F.tv_at(^self, u32 at)` — Index of the trivia starting at `at`, or -1.
* `bool F.inside_a_comment(^self, u32 at)`
* `i64 F.column_of(^self, u32 at)`
* `u32 F.past_blanks(^self, u32 at)`
* `u32 F.code_at(^self, u32 at)` — The first byte of code at or after `at`.
* `void F.blanks(^self, i64 n)`
* `void F.place(^self, i64 from, i64 to, bool air)`
* `void F.leading(^self, u32 before)`
* `void F.leading_tight(^self, u32 before)`
* `void F.open(^self, u32 lo)`
* `void F.open_tight(^self, u32 lo)`
* `void F.close(^self, u32 at)`
* `void F.separate_at(^self, i64 at, i64 n)`
* `Mark F.mark(^self)`
* `void F.restore(^self, Mark m)`
* `bool F.within_budget(^self, i64 mark)`
* `i64 F.column(^self)`
* `u32 F.written_end(^self, u32 lo, u32 hi)` — The end of what the author wrote for lo..hi, brackets balanced, or NONE.
* `void F.as_written(^self, u32 start, u32 end)` — `text` (the author's lines, src[start..<end]) moved to this column.
* `bool F.fuses_at(^self, i64 at)`
* `void F.mend(^self)`
* `i32 format_text(string8 path, string8 src, ^const Style s, ^ref []char8 out, ^ref []char8 err)` — `dadoc --format`'s contract: 0 and the text in `out`; 1 when `src` does not…
* `const u8 FIT = 0`
* `const u8 FOLD = 1`
* `const u8 EXPLODE = 2`
* `const u8 AS_WRITTEN = 3`
* `const [30]string8 KEYS = ("budget", "align_members", "align_comments", "align_enum_values", "blank_before_item", "blank_before_foreign_item", "blank_after_package", "blank_after_imports", "blank_before_comment", "blank_after_when", "blank_before_block", "blank_after_block", "blank_run_cap", "blank_between_one_line_items", "space_around_binary", "space_around_assign", "space_around_decl_eq", "space_after_comma", "space_after_colon", "space_before_comment", "space_inside_parens", "space_inside_brackets", "space_inside_braces", "fold_suite", "fold_function_body", "fold_member_list", "fold_enum_body", "fold_foreign_type", "fold_params", "fold_statement")` — Numbers, by key: KEYS[i] is n[i]; the fold keys hold FIT, FOLD, ...
* `const [30]u8 DEFAULTS = (80, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, FIT, EXPLODE, EXPLODE, EXPLODE, EXPLODE, FIT, FIT)`
* `const i64 BUDGET = 0`
* `const i64 ALIGN_MEMBERS = 1`
* `const i64 ALIGN_COMMENTS = 2`
* `const i64 ALIGN_ENUM = 3`
* `const i64 B_ITEM = 4`
* `const i64 B_FOREIGN = 5`
* `const i64 B_PACKAGE = 6`
* `const i64 B_IMPORTS = 7`
* `const i64 B_COMMENT = 8`
* `const i64 B_WHEN = 9`
* `const i64 B_BEFORE_BLOCK = 10`
* `const i64 B_AFTER_BLOCK = 11`
* `const i64 B_CAP = 12`
* `const i64 B_ONE_LINE = 13`
* `const i64 S_BINARY = 14`
* `const i64 S_ASSIGN = 15`
* `const i64 S_DECL_EQ = 16`
* `const i64 S_COMMA = 17`
* `const i64 S_COLON = 18`
* `const i64 S_COMMENT = 19`
* `const i64 S_PARENS = 20`
* `const i64 S_BRACKETS = 21`
* `const i64 S_BRACES = 22`
* `const i64 F_SUITE = 23`
* `const i64 F_FN_BODY = 24`
* `const i64 F_MEMBERS = 25`
* `const i64 F_ENUM = 26`
* `const i64 F_FOREIGN = 27`
* `const i64 F_PARAMS = 28`
* `const i64 F_STATEMENT = 29`
* `type Style`
* `Style defaults()`
* `Style read_style(string8 text, ^ref []char8 why)` — Reads a `.dadofmt`; a bad line sets `line` and `why`.
* `Style style_for(string8 path, ^ref []char8 where, ^ref []char8 why)` — The style found from the directory of `path` upward (Style::found_from);…
