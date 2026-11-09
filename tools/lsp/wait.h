/* Timed reads of stdin for the LSP's idle check, in C: Dado has no timed read. */
#ifndef DADO_LSP_WAIT_H
#define DADO_LSP_WAIT_H
#include <stdint.h>
int32_t lsp_wait(int32_t ms);
int64_t lsp_read(void *buf, int64_t n);
#endif
