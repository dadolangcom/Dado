//@ procs/arguments 35
//@ ### Operands and arguments are evaluated left to right
//@
//@ A call evaluates its arguments left to right, and an operator evaluates
//@ its operands left to right. A subscript or a window evaluates its base
//@ before what is inside the brackets. An assignment fixes its place
//@ before the value on the right runs, so a store into an element whose
//@ index a call on the right moves lands where the index pointed before
//@ the call; a compound assignment also reads the place there, so it adds
//@ to the value from before the call.
//@
//@ The order is guaranteed for every write, and for every read a write
//@ could change. Where two operands of an operator can each only stop the
//@ program, as two subscripts out of bounds in `a[i] + a[j]` can, which
//@ of them is reported is unspecified.
//@
//@ C leaves these orders unspecified and its compilers disagree, so the
//@ compiler binds an earlier operand to a temporary, or a place's address
//@ to a pointer, wherever one of the two writes something the other could
//@ read. Where no operand writes, nothing is bound. `&&`, `||` and a
//@ conditional value evaluate their right side only when it is needed,
//@ after the left.
