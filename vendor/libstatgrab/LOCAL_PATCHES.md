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

Upstream: https://github.com/libstatgrab/libstatgrab/pull/141, merged
to master as 97885479a7c228d2d2d64e13f5177c1cec8e99b3 (2026-08-13) but not
in a release yet (latest is 0.92.1). Drop this patch once statgrab
re-vendors a release that includes it.

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

### 0003-process-stats-initialise-sessid.patch (2026-10-13)

`src/libstatgrab/process_stats.c` — `sg_process_stats_item_init()` left
`sessid` unset. Only the kinfo_proc backends (BSD/Darwin) assign it;
the Linux, Solaris, HP-UX, and AIX backends never do, and the stats
vector is allocated without zeroing. `session_id` in
`sg_process_stats()` rows therefore carried leftover heap bytes on
Linux, including fragments of previously freed strings.

Fix: initialise `sessid` to 0 in the item init function. The BSD/Darwin
backends still overwrite it with the real session id.

Reproducer: `tests/033-process-session-id.phpt` asserts every row's
`session_id` is 0 on Linux. Against the unpatched library it fails with
values such as `-1094795586` (ASan's malloc fill) or arbitrary integers
on a normal build.

Upstream: https://github.com/libstatgrab/libstatgrab/pull/147. Drop this
patch once upstream merges it and statgrab re-vendors a release that
includes it.

### 0004-unsigned-bit-and-ethtool-speed-shifts.patch (2026-10-13)

`src/libstatgrab/tools.h` — `BIT_SET`, `BIT_ISSET`, and `BIT_GET`
shifted a signed `1`, so bit 31 of each `unsigned` word evaluated
`1 << 31`, which overflows `int`. Every caller uses `unsigned` arrays,
so `1u` keeps the same bit layout. Hit by `sg_get_fs_stats()` on hosts
with 32 or more mounts and by the diff getters with 32 or more items.

`src/libstatgrab/network_stats.c` — the Linux interface-speed path
called `ethtool_cmd_speed()` from `<linux/ethtool.h>`, which shifts the
16-bit `speed_hi` after promotion to `int`. For `SPEED_UNKNOWN`
(`0xFFFF`) the shift overflows. The speed is now computed in place
with `speed_hi` cast to `__u32` first, giving the same value.

Reproducer: build the bundled library and extension with
`-fsanitize=address,undefined` and run the PHPT suite with
`UBSAN_OPTIONS=halt_on_error=1`. Before the patch, `009-fs`,
`011-iface`, `016-oo-class`, and `023-tier-c-constants` abort with
`left shift of 1 by 31 places` (`disk_stats.c`) or `left shift of 65535
by 16 places` (`ethtool.h`); after it they pass.

Local only; not submitted upstream.
