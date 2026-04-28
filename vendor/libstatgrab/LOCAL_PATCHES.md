# Local patches against vendored libstatgrab 0.92.1

Pristine source is `libstatgrab-0.92.1.tar.gz` from
https://github.com/libstatgrab/libstatgrab/releases/tag/LIBSTATGRAB_0_92_1
(SHA-256 of release tarball, recorded for tamper detection: see
`patches/SHA256SUMS`).

Modifications below are required at LGPL 2.1 §2(a) "carry prominent
notices stating that you changed the files and the date of any change."
The patched files retain their LGPL 2.1+ headers; the patch chunks
themselves are also LGPL 2.1+.

## Active patches

### 0001-fix-sg-destroy-globals-off-by-one.patch (2026-04-27)

`src/libstatgrab/globals.c` — `sg_destroy_globals()` walked components
with `size_t i = lengthof(comp_info) - 1; while (i--) { ... }`. The
post-decrement-in-test pattern starts the body with `i = N-2`, so the
final component (index `N-1`, which is `sg_user_init`) never has its
`cleanup_fn` called. Result: the `user_stats` vector and every string
it owns (login_name, device, hostname, record_id) leak at process exit.

Fix: start `i = lengthof(comp_info)` so the body runs with `i = N-1`
through `i = 0`. Compute `zero_size` from `last_idx = N-1` separately.

Reproducer (under ASan, before patch):

```c
sg_init(1);
size_t n;
sg_get_user_stats(&n);
sg_shutdown();
```

→ `Direct leak of 1112 byte(s) ... sg_get_user_stats / sg_vector_create
/ sg_realloc`. Same shape against `sg_get_host_info`, `sg_get_load_stats`,
or any function that exercises `sg_user_init`'s vector.

After patch: zero leaks reported.

Submitted upstream as a PR against `libstatgrab/libstatgrab` (link goes
here once filed).
