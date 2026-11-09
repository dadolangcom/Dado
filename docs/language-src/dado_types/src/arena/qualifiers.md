//@ types/distinct 30
//@ ### Where nominality lives
//@
//@ Two declaration words, and exactly one of them makes a type.
//@
//@ **`type` adds no identity.** It binds a name to a shape and stops
//@ there, so the name is a spelling and the shape is the type. Two
//@ declarations of the same slots are the same type however they were
//@ spelled or wherever they were written; handing one where the other is
//@ required is not a conversion, and there is nothing for a check to ask.
//@
//@ **`distinct` adds one.** The shape is computed exactly as it would be
//@ without the word and a tag goes on afterwards — a modifier, not a
//@ second kind of declaration, so nothing about the shape changes when the
//@ word appears. The tag is the package name joined to the declared name,
//@ so two packages declaring one name are two types and identity does not
//@ depend on the order files were read.
//@
//@ What that buys is the refusal. A temperature and a bare `f32` are the
//@ same float and only the tagged one declines to stand in the other's
//@ position; the diagnostic is `ERR0504`, raised at the argument and
//@ naming both shapes rather than at the declaration.
//@
//@ What it costs is that every crossing is now written out. Nothing
//@ converts on your behalf, so a tagged value reaching a routine that
//@ takes the underlying shape needs a conversion at the call site, and
//@ enough of those is a wrapper nobody wanted to maintain. Spend it where
//@ the mix-up is the bug — two floats measuring different things — and not
//@ on every name.
//@
//@ **The tag sits beside the shape and erases at transpile.** Operators
//@ are inherited within it, the emitted C is the underlying shape's C, and
//@ `TypeArena::underlying` reaches through for everything downstream that
//@ wants bytes rather than a label. A tag that changed the layout would be
//@ a different feature wearing this one's word.
//@
//@ `distinct` over a union is refused with `ERR0515` rather than quietly
//@ ignored: a union is already nominal by its tag set, so the word would
//@ promise a second tag that is not there.
