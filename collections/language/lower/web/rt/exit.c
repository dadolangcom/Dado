/* The `#WEB` runtime's exit: how the DadoScript runtime (`core:script`) ends a
 * program whose script failed with nothing to handle it. A module has no
 * process to end, so standard output is flushed and the status handed to the
 * host's `dado.exit`, which unwinds the module to the loader; the loader
 * answers the status as a native run's. The host never returns; a host that
 * did would reach the trap. Needs the console (its flush). */

void dado_rt__web_exit_status(int32_t status);

void dado_rt__web_exit_status(int32_t status) {
    dado_rt__web_flush();
    dado_rt__import_dado_exit(status);
    __builtin_trap();
}
