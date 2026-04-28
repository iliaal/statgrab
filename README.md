# statgrab

[![Tests](https://github.com/iliaal/statgrab/actions/workflows/tests.yml/badge.svg?branch=master)](https://github.com/iliaal/statgrab/actions/workflows/tests.yml)
[![License: PHP-3.01](https://img.shields.io/badge/License-PHP--3.01-green.svg)](http://www.php.net/license/3_01.txt)
[![License: LGPL-2.1+ (vendored libstatgrab)](https://img.shields.io/badge/vendored-LGPL--2.1%2B-blue.svg)](LICENSE.libstatgrab)

A PHP extension wrapping [libstatgrab](https://libstatgrab.org), the
cross-platform system-statistics library. Originally released to PECL
in 2006 against libstatgrab 0.6; this 2.0.0 line is a full
modernization for the PHP 8.0+ era against libstatgrab 0.92+.

Supports PHP 8.0 through 8.5 on glibc Linux, musl, macOS, and *BSD.

## Install

### PIE (recommended on PHP 8.x)

[PIE](https://github.com/php/pie) is the PHP Foundation's PECL successor.
It installs from Packagist, builds against the active `php-config`, and
produces a loadable `.so`. Make sure libstatgrab is installed first
(see the From-source section below for the system package names), then:

```sh
pie install iliaal/statgrab
```

Then add `extension=statgrab` to your `php.ini`.

### PECL

The package remains in the PECL channel for legacy installers:

```sh
pecl install statgrab
```

### From source

```sh
sudo apt install libstatgrab-dev   # Debian/Ubuntu
# OR: brew install libstatgrab     # macOS
# OR: pkg install libstatgrab      # FreeBSD

phpize
./configure --with-statgrab
make
sudo make install
```

Then add `extension=statgrab` to your `php.ini`.

`config.m4` resolves libstatgrab through `pkg-config` first; if that
fails it falls back to a path probe (`/usr` and `/usr/local`). Pass
`--with-statgrab=<prefix>` to point at a custom install.

### Bundled libstatgrab (statically linked, leak-fixed)

The repo carries a vendored copy of libstatgrab 0.92.1 under
`vendor/libstatgrab/` with one local patch (see
`vendor/libstatgrab/LOCAL_PATCHES.md`) that fixes a process-exit leak
upstream hasn't released yet. To use it:

```sh
(cd vendor/libstatgrab && ./configure --enable-static --disable-shared --without-ncurses --with-pic && make)
phpize
./configure --with-statgrab=bundled
make
```

The resulting `.so` has no `libstatgrab.so` runtime dependency.

The vendored libstatgrab tree stays LGPL 2.1+ (see `LICENSE.libstatgrab`);
the extension code stays PHP-3.01 (see `LICENSE`). Dynamic-link or
static-link, neither license infects the other.

## API

### Procedural

The 2006 function names are preserved.

```php
sg_cpu_percent_usage(): array|false       // user/kernel/idle/iowait/swap/nice (%)
sg_cpu_totals(): array|false              // cumulative jiffies + ctx switches/syscalls/IRQs
sg_cpu_diff(): array|false                // jiffies since last call
sg_diskio_stats(): array|false            // [diskname => [read, written, time_frame]]
sg_diskio_stats_diff(): array|false
sg_fs_stats(): array|false                // mounted filesystems with size/used/inodes/...
sg_general_stats(): array|false           // os_name, hostname, uptime, ncpus, ...
sg_load_stats(): array|false              // min1, min5, min15
sg_memory_stats(): array|false            // total, free, used, cache (bytes)
sg_swap_stats(): array|false              // total, free, used (bytes)
sg_network_stats(): array|false           // [ifname => sent/received/packets/...]
sg_network_stats_diff(): array|false
sg_page_stats(): array|false              // pages_in, pages_out (cumulative)
sg_page_stats_diff(): array|false
sg_process_count(): array|false           // total, running, sleeping, stopped, zombie
sg_process_stats(?int $sort = null, int $limit = 0): array|false
sg_user_stats(): array|false              // [{login_name, device, pid, login_time, ...}]
sg_network_iface_stats(): array|false     // [ifname => {speed, duplex, active}]
```

### Object-oriented

```php
$sg = new Statgrab();
$sg->cpu();
$sg->host();
$sg->memory();
$sg->processes(Statgrab::SORT_CPU, 10);
$sg->disks(diff: true);
```

Class constants:

- `Statgrab::DUPLEX_FULL | DUPLEX_HALF | DUPLEX_UNKNOWN`
- `Statgrab::SORT_NAME | PID | UID | GID | SIZE | RES | CPU | TIME`
- `Statgrab::STATE_RUNNING | SLEEPING | STOPPED | ZOMBIE | UNKNOWN`

The `SG_*` global constants from 2006 are still defined for BC.

## Errors

Library-side errors emit `E_WARNING` with the libstatgrab error string
and code, and the function returns `false`. The OO surface follows the
same convention. Argument-count violations on no-arg functions throw
`ArgumentCountError` per modern PHP convention.

## Notable 2.0 BC breaks

- `sg_user_stats()` returns per-user records, not a flat array of
  usernames. The underlying `name_list` field was removed from
  libstatgrab 0.91+. Migrate callers to read `login_name` from each
  record.
- Numeric counters (memory totals, fs sizes, jiffies) are returned as
  `int` instead of stringified numbers. The 2006 release stringified
  via `snprintf("%lld")` because 32-bit PHP couldn't hold them; modern
  64-bit `zend_long` does.
- `sg_page_stats()` / `sg_page_stats_diff()` were swapped in 2006 and
  are now correct.
- `sg_process_stats()` fields `gid` and `egid` are now distinct from
  `uid` and `euid` (2006 had a copy-paste bug returning uid/euid for
  both).

See `CHANGELOG.md` for the full list.

## License

[PHP License 3.01](LICENSE).
