# Changelog

All notable changes to this project are documented here. The format is
based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and
this project adheres to [Semantic Versioning](https://semver.org).

## [Unreleased]

## [2.2.0] - 2026-04-27

### Added

- Vendored libstatgrab 0.92.1 source tree under `vendor/libstatgrab/`,
  carrying one local patch (LGPL 2.1+, source-reviewable, with full
  diff under `vendor/libstatgrab/patches/`).
- `--with-statgrab=bundled` configure option. Statically links the
  patched libstatgrab into the extension `.so`, removing the runtime
  `libstatgrab.so` dependency and shipping the leak fix without
  waiting on system packages. Pre-build the lib first
  (`(cd vendor/libstatgrab && ./configure --enable-static --disable-shared --without-ncurses && make)`),
  then `./configure --with-statgrab=bundled`. System libstatgrab
  remains the default.
- `LICENSE.libstatgrab` (LGPL 2.1 text) at the repo root, sitting next
  to our PHP-3.01 `LICENSE`. Two licenses, neither infects the other:
  the libstatgrab tree under `vendor/` stays LGPL, our extension code
  stays PHP-3.01.
- **PIE install support.** `composer.json` declares `type: php-ext`
  with the full `php-ext` block (configure options, NTS support flag),
  so `pie install iliaal/statgrab` works once the package is registered
  on Packagist. PECL `pecl install statgrab` continues to work
  unchanged.

### Fixed

- libstatgrab leak at process exit: `sg_destroy_globals` walked the
  component table with `size_t i = lengthof(comp_info) - 1; while (i--)`,
  starting body iterations at `i = N-2` so the last component
  (`sg_user_init`) never had its `cleanup_fn` invoked. Net effect:
  the user-stats vector and every owned string (login_name, device,
  hostname, record_id) leaked at every process exit, ~1.1 KB. Patch
  shipped as `vendor/libstatgrab/patches/0001-fix-sg-destroy-globals-off-by-one.patch`
  and submitted upstream.
- Our own leak: `sg_get_cpu_percents_r` returns owned memory; we
  weren't calling `sg_free_stats_buf` on it. 144 bytes per call to
  `sg_cpu_percent_usage()` /  `Statgrab::cpu()`. Now freed.

### Changed

- Tests no longer require `ASAN_OPTIONS=detect_leaks=0` when run
  against the bundled libstatgrab. The env var is still set in
  `.release-config` and CI for safety against system libstatgrab
  shipping the unpatched library.

[Unreleased]: https://github.com/iliaal/statgrab/compare/2.2.0...HEAD
[2.2.0]: https://github.com/iliaal/statgrab/releases/tag/2.2.0

## [2.1.0] - 2026-04-27

Feature-additive, BC-preserving. Surfaces libstatgrab 0.92 capability that
2.0.0 dropped on the floor.

### Added

- `sg_valid_filesystems()` / `Statgrab::validFilesystems()` — query the
  list of fs types libstatgrab considers "real" (the default ~39-entry
  Linux list excludes nothing automatic, callers can narrow it).
- `sg_set_valid_filesystems(array)` / `Statgrab::setValidFilesystems()`
  — override that list. Subsequent `sg_fs_stats()` calls only return
  mounts whose `fs_type` is in the configured set. Throws `TypeError`
  on a non-string array entry.
- `sg_snapshot()` / `Statgrab::snapshot()` — re-seed libstatgrab's
  internal counters explicitly. Useful before a sliding-window cpu /
  diff sample in a long-running worker.
- `sg_error_details()` / `Statgrab::errorDetails()` — return the
  libstatgrab error code, message, errno, and arg as an array (or
  `false` if no error pending). Pair with the `false` return from any
  stat function for a richer diagnostic.
- `sg_cpu_percent_usage(int $source = Statgrab::CPU_PERCENT_ENTIRE)`
  / `Statgrab::cpu(int $source = ...)` — let callers choose between
  `CPU_PERCENT_ENTIRE` (cumulative since boot, default), `LAST_DIFF`
  (since previous internal snapshot), and `NEW_DIFF` (since this call).
  The default keeps the 2.0 behavior.
- New struct fields exposed in returns:
  - `sg_memory_stats`, `sg_load_stats`, `sg_swap_stats` — `systime`
  - `sg_process_stats` — `context_switches`,
    `voluntary_context_switches`, `involuntary_context_switches`,
    `systime`
- New class constants:
  - `Statgrab::CPU_PERCENT_ENTIRE | LAST_DIFF | NEW_DIFF`
  - `Statgrab::HOST_STATE_UNKNOWN | PHYSICAL | VIRTUAL_MACHINE |
    PARAVIRTUAL_MACHINE | HARDWARE_VIRTUALIZED` — interpret the
    `host_state` field on `sg_general_stats()`.
  - `Statgrab::FS_UNKNOWN | REGULAR | SPECIAL | LOOPBACK | REMOTE |
    LOCAL | ALLTYPES` — bitmask values for the `device_type` field on
    `sg_fs_stats()`.
  - `Statgrab::ERROR_NONE | INVALID_ARGUMENT | OPEN | OPENDIR |
    PERMISSION | UNSUPPORTED` — most useful libstatgrab error codes
    for programmatic catches.

[2.1.0]: https://github.com/iliaal/statgrab/releases/tag/2.1.0

## [2.0.0] - 2026-04-27

Full modernization of the 2006 PECL release. PHP 8.0–8.5 floor,
libstatgrab 0.92+ floor.

### Added

- `Statgrab` final class with method-style accessors (`->cpu()`,
  `->cpuStats()`, `->cpuDiff()`, `->disks(bool $diff = false)`,
  `->filesystems()`, `->host()`, `->load()`, `->memory()`, `->swap()`,
  `->network(bool $diff = false)`, `->pages(bool $diff = false)`,
  `->processes(?int $sort = null, int $limit = 0)`, `->processCount()`,
  `->users()`, `->interfaces()`).
- Class constants `Statgrab::DUPLEX_FULL/HALF/UNKNOWN`,
  `Statgrab::SORT_NAME/PID/UID/GID/SIZE/RES/CPU/TIME`,
  `Statgrab::STATE_RUNNING/SLEEPING/STOPPED/ZOMBIE/UNKNOWN`.
- `sg_general_stats()`: new fields `bitwidth`, `host_state`, `ncpus`,
  `maxcpus`, `systime` (in addition to the 2006 set).
- `sg_cpu_totals()` / `sg_cpu_diff()`: new fields `context_switches`,
  `voluntary_context_switches`, `involuntary_context_switches`,
  `syscalls`, `interrupts`, `soft_interrupts`.
- `sg_fs_stats()`: new fields `device_canonical`, `device_type`, `free`,
  `systime`.
- `sg_network_iface_stats()`: new fields `factor`, `systime`.
- `sg_process_count()`: new fields `unknown`, `systime`.
- `sg_process_stats()`: new field `session_id`.
- `MSHUTDOWN` calls `sg_shutdown()` (the 2006 release leaked init state).

### Changed

- **PHP floor: 8.0+.** PHP 4/5/7 are not supported.
- **libstatgrab floor: 0.92+.** The conditional `HAVE_OLD_STATGRAB`
  paths are gone.
- All number-of-bytes fields now return `int` instead of `string`.
  In 2006 we formatted them via `snprintf("%lld")` because PHP couldn't
  represent 64-bit ints on 32-bit platforms; on a modern 64-bit build
  `zend_long` covers TB/PB just fine. Callers comparing string totals
  will break.
- `sg_user_stats()` now returns an array of per-user records with
  fields `login_name`, `device`, `hostname`, `pid`, `login_time`,
  `record_id`, `systime`. The 2006 release returned a flat array of
  username strings derived from `name_list`; libstatgrab no longer
  exposes that field.
- `config.m4` resolves libstatgrab via `pkg-config` first, falling back
  to the legacy include/lib path probe. `--with-statgrab=DIR` still
  works.
- `php_statgrab.h` no longer ships `PHP_FUNCTION` declarations; they
  live in `statgrab_arginfo.h` (generated from `statgrab.stub.php`).
- Errors from libstatgrab now emit `E_WARNING` with both a code and a
  string from `sg_str_error`. The 2006 release used `%d: %s`; the new
  format reads `libstatgrab: <reason> (code N)`.
- `sg_drop_privileges()` failure now emits `E_NOTICE` instead of
  aborting via `E_ERROR`. The 2006 path was a hard kill on systems
  where the binary had no setuid bit to drop.
- `sg_cpu_percent_usage()` now samples cpu_stats and computes percents
  via `sg_get_cpu_percents_r` to avoid the NULL-on-first-call behavior
  of `sg_get_cpu_percents()` in 0.92.

### Fixed

- `sg_page_stats()` and `sg_page_stats_diff()` were swapped in the
  2006 release: `sg_page_stats` returned diff-since-last-call data and
  `sg_page_stats_diff` returned cumulative-since-boot. Now correct.
- `sg_process_stats()` fields `gid` / `egid` returned `uid` / `euid`
  in 2006. Now report the actual gid and egid.
- `add_assoc_*_ex` calls passed `sizeof("key")` (including NUL) as the
  key length, putting trailing NUL bytes into the array key. Now use
  `sizeof("key") - 1`.
- `MAKE_STD_ZVAL` and the `int dup` 4th arg of `add_assoc_string` (both
  removed in PHP 7) replaced with stack zvals and the modern signatures.
- `php_explode` zval-arg signature gone — `sg_user_stats` no longer
  uses it.
- All `TSRMLS_CC` / `TSRMLS_DC` macros removed (PHP 8 dropped them).

### Removed

- The conditional `HAVE_OLD_STATGRAB` / `HAVE_STATGRAB_09` build paths.
  Anything older than 0.92 is no longer supported.

### Known issues

- libstatgrab 0.92.1 leaks ~1KB of internal state on process exit
  (Ubuntu jammy package, not fixed upstream as of this writing). Not
  a per-call leak; harmless in practice. Tests run with
  `ASAN_OPTIONS=detect_leaks=0` to keep CI green.

[2.0.0]: https://github.com/iliaal/statgrab/releases/tag/2.0.0
