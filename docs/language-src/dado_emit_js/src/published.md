//@ toys/dialect 33
//@ ### A JavaScript library reaches Dado's exports
//@
//@ A JavaScript file a script imports may import the program's own `export`s
//@ by the reserved name `dados:exports`:
//@
//@     import { exports, memory } from "dados:exports";
//@
//@     export function hit(n) {
//@         return exports === null ? 0 : exports.damage(n);
//@     }
//@
//@ On the web, `exports` holds each `export` of the Dado program as a
//@ function of its wasm instance, under its C name, and `memory` the
//@ instance's memory; both are filled before `main` runs, and nothing else
//@ the module exports is in them. A page reaches the same module by that
//@ name, which its import map holds. A native build has no instance, so
//@ both are `null` there.
