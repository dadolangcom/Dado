/* A C library's own 2-vector, for `linalg_test.dado` to hand to `core:linalg`
 * as it stands. Header-only and `static`, and every function here is called
 * by the test, so nothing is left unused under `-Werror=unused-function`.
 *
 * Inert for every other build of the package: a header beside a package is
 * read only by a `foreign` block that names it, and only the test does. */
#ifndef DADO_CORE_LINALG_TEST_H
#define DADO_CORE_LINALG_TEST_H

typedef struct { float x, y; } lt_vec2;

static lt_vec2 lt_make(float x, float y) {
    lt_vec2 v = { x, y };
    return v;
}

static float lt_cross(lt_vec2 a, lt_vec2 b) { return a.x * b.y - a.y * b.x; }

#endif /* DADO_CORE_LINALG_TEST_H */
