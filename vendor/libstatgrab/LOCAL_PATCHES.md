# Local patches against vendored libstatgrab 0.92.2

Pristine source is `libstatgrab-0.92.2.tar.gz` from
https://github.com/libstatgrab/libstatgrab/releases/tag/LIBSTATGRAB_0_92_2
(SHA-256 of release tarball, recorded for tamper detection: see
`patches/SHA256SUMS`; the detached `.asc` signatures by keys
`4AD86566CAD03297A9804702A12F416AA37F3BD7` and
`4BD95F908A5040E8D26CD6816C226B37FDF38D55` verified on 2026-11-27).
The tracked tree is that tarball minus its generated autotools output
(`configure`, `aclocal.m4`, `config.h.in`, `install-sh`,
`ltmain.sh`, `missing`), with the patches below applied in order.

Modifications below are required at LGPL 2.1 §2(a) "carry prominent
notices stating that you changed the files and the date of any change."
The patched files retain their LGPL 2.1+ headers; the patch chunks
themselves are also LGPL 2.1+.

## Active patches

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

### 0006-fs-stats-keep-mount-table-error.patch (2026-11-19)

`src/libstatgrab/disk_stats.c` — when `sg_get_fs_list_int()` failed (on
Linux, `setmntent()` could open none of `/proc/mounts`, `/etc/mnttab`,
`/etc/mtab`), `sg_get_fs_stats_int()` still resized the result vector
to the 0 valid entries it had found. `sg_vector_resize()` with a count of
0 frees the vector and calls `sg_clear_error()`, so `sg_get_fs_stats()`
returned NULL with `sg_get_error()` reporting `SG_ERROR_NONE`. In PHP,
`sg_fs_stats()` warned `libstatgrab: no error (code 0)` and
`sg_error_details()` returned `false`.

Fix: return the list error, after freeing the partial list, before the
resize.

Reproducer: `tests/c/fs-setmntent.sh` fails every `setmntent()` through
`--wrap=setmntent`. With this patch reverted it prints `error=no error
(0)` and fails; with the patch it reports `setmntent failed (33)` with
errno `EACCES`.

Local only; not submitted upstream.

### 0007-globals-delete-tls-key-on-last-destroy.patch (2026-11-19)

`src/libstatgrab/globals.c` — the per-thread storage key was created
once with `pthread_once()` and never deleted, with `sg_destroy_globals()`
as its destructor. A thread that used the library and exited after the
host unloaded it (an embed host shutting PHP down while worker threads are
alive) ran that destructor from unmapped code and crashed.

Fix: create the key on the first `sg_comp_init()` and delete it on the
last `sg_comp_destroy()`, after freeing the calling thread's storage.
Deleted keys run no destructors. The created flag is read without the
lock in `sg_comp_get_tls()`, so it is accessed with acquire/release
atomics. Storage of other threads still alive at
that point is leaked, once per unload. `sg_comp_get_tls()` returns NULL
while no key exists.

Reproducer: `tests/zts-unload.sh` against an ASan ZTS embed PHP. Without
the patch the worker thread dies with an ASan SEGV at an unmapped pc after
`php_embed_shutdown()`; with it all 6 ok lines print. The extension used
to pin itself with `RTLD_NODELETE` instead, which also kept libphp mapped
for hosts that `dlclose()` PHP.

Local only; not submitted upstream.

### 0008-tools-resolve-regex-in-glibc.patch (2026-11-19)

`src/libstatgrab/tools.h`, `src/libstatgrab/tools.c` — on glibc, the
library's `regcomp()`/`regexec()`/`regfree()` calls now go through
wrappers that look the functions up in libc with `dlopen(LIBC_SO,
RTLD_NOLOAD)` and `dlsym()`. A program that exports other functions
under those names (PHP linked against oniguruma's `libonig`, as in the
official Docker `php` images) otherwise receives the library's unversioned
references, so the `/proc/net/dev` and `/proc/filesystems` patterns never
matched: `sg_get_network_io_stats()` returned no rows and the default
valid-filesystems list was empty. Other C libraries keep the plain calls;
with musl (FrankenPHP's Alpine image, which also links `libonig`) the
library already gets musl's regex. `configure.ac` adds
`AC_SEARCH_LIBS(dlopen, [dl])` because glibc before 2.34 keeps
`dlopen()` in libdl.

Reproducer: `tests/c/regex-interpose.sh` links a program that defines its
own failing regex functions. Without the patch it reports `network
rows=0` and 18 foreign regex calls; with it, real rows and 0 foreign calls.

Local only; not submitted upstream.

## Dropped patches

Released upstream in 0.92.2 with the same code, so dropped when re-vendoring
it on 2026-11-27. The numbers stay retired so references to them in the
CHANGELOG and tests remain unambiguous.

- 0001-fix-sg-destroy-globals-off-by-one: upstream 97885479 (PR #141).
- 0002-vector-size-compute-diff-bitmap-from-last-vector: upstream fb9cf695
  (PR #148). `tests/c/diff-shrink.sh` still covers it.
- 0003-process-stats-initialise-sessid: upstream 3922b3f5 (PR #147).
  `tests/033-process-session-id.phpt` still covers it.
- 0005-process-stats-linux-cmdline-no-pointer: upstream bfb3dab8 (PR #146).
  `tests/c/cmdline-title.sh` still covers it.

0.92.2 also fixes two diff-vector bugs that no local patch carried, both
reachable from `sg_network_stats_diff()` and `sg_diskio_stats_diff()`:
a use-after-free after an empty or failed refresh (319fc05b,
GHSA-89gf-hjq6-43hf) and a heap overflow when a zero-item diff vector grows
to one item (444af296, GHSA-p29q-x496-4gjf). `tests/c/diff-shrink.sh` covers
both.
