//@ toys/dialect 31
//@ ### Stopping, pausing and asking after a running script
//@
//@ A script runs until it returns, and a Dado program that runs one on a
//@ thread of its own keeps the right to end it, from any thread.
//@ `#script_stop(vm, 100)` asks the script running in the VM to stop,
//@ waits up to 100
//@ milliseconds for its run to leave the engine, and forces it if it has
//@ not. Asked, the script meets an `Error` at its next loop or call — its
//@ deferred lines run, and a `try` around the work it was doing reaches the
//@ stop. Forced, it runs nothing more, not even a deferred line; a force
//@ waits for any Dado function the script is inside to return. Either way
//@ the engine then ends: every value it holds is released and its memory
//@ is freed at once, and a later call through any handle on it fails, with
//@ `core:script`'s code `script.STOPPED` under `try`. The
//@ timeout is milliseconds, and it is always written: `#script_stop(vm, 0)`
//@ forces at once, and `#script_stop(vm)` is refused naming both. An engine
//@ with no run in progress ends at once. `#delete(vm)` still frees the
//@ VM afterwards.
//@
//@ The run a stop interrupts answers `script.STOPPED` where it was written
//@ under `try`; written without one, a stop is not a failure — the program
//@ decided it — so the run answers its type's zero value and the program
//@ goes on.
//@
//@ `#script_pause(vm)` holds the run in progress at its next loop or call
//@ until `#script_resume(vm)`; while it is paused, the VM is busy, and a
//@ call or a `#script_new` on it fails with `script.BUSY`. A VM's runs come
//@ only from the thread that made it: one from another thread fails with
//@ `script.THREAD`, while its stop, pause, resume and queries come from any
//@ thread. A stop resumes a paused
//@ script into its polite phase. `#script_is_running(vm)` answers whether a
//@ run is in progress on the engine, paused or not, and
//@ `#script_is_paused(vm)` whether it is paused.
//@
//@ `i64 n = #script_pump(vm)` is the host's frame boundary for channels: it
//@ delivers every message waiting for the VM's handlers — the functions its
//@ scripts bound to the program's channels — and answers how many. A VM
//@ it cannot enter answers a negative code, `script.THREAD` from another
//@ thread, `script.STALE` once deleted, `script.STOPPED` once stopped, its
//@ text on the error sink. Messages are delivered as a call into the script
//@ returns, too; the pump is for a frame that calls nothing.
