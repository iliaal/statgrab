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

### 0002-vector-size-compute-diff-bitmap-from-last-vector.patch (2026-10-13)

`src/libstatgrab/vector.c` — `sg_vector_compute_diff()` sized its stack
`matched[]` bitmap from `cur_vector->used_count` but indexes it with `j`
over `last_vector->used_count`. When the item count shrinks across a
32-item word boundary between two diff polls (a network interface or
disk removed), `BIT_ISSET` reads and `BIT_SET` writes past the end of
the stack VLA. Reachable from `sg_get_network_io_stats_diff()`,
`sg_get_disk_io_stats_diff()`, and `sg_get_fs_stats_diff()`; in PHP,
from `sg_network_stats_diff()` and `sg_diskio_stats_diff()`.

Fix: size the bitmap from `last_vector->used_count`, the range `j`
actually spans.

Reproducer: `tests/c/diff-shrink.sh` serves 40 interfaces through a
wrapped `/proc/net/dev`, then 5. Against an ASan build of the unpatched
library it aborts with `dynamic-stack-buffer-overflow ... in
sg_vector_compute_diff`; after the patch it reports the 5 surviving
interfaces with correct deltas.

Upstream: https://github.com/libstatgrab/libstatgrab/pull/148. Drop this
patch once upstream merges it and statgrab re-vendors a release that
includes it.
