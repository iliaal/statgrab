# Security policy

statgrab exposes libstatgrab's read-only host telemetry (CPU, memory,
swap, disk I/O, filesystems, network interfaces, processes, users, and
load) to PHP. The attack surface is deliberately thin: inputs are
OS-sourced system counters, not attacker-controlled data. The only
caller-supplied input path is `sg_set_valid_filesystems()` /
`Statgrab::setValidFilesystems()`.

## Supported versions

| Version | Supported          |
|---------|--------------------|
| 2.2.x   | :white_check_mark: |

The two most recent minor versions receive security fixes.

## Reporting a vulnerability

**Do not file a public GitHub issue for security vulnerabilities.**

Use GitHub's private security advisory feature at
<https://github.com/iliaal/statgrab/security/advisories/new>
or email Ilia Alshanetsky <ilia@ilia.ws> directly.

Please include:

- Affected statgrab version (`php -r 'echo phpversion("statgrab");'`)
- PHP version (`php -v`)
- How the extension is linked: system libstatgrab (and its version), or
  the bundled `--with-statgrab=bundled` build
- A minimal reproducing case (small enough to inline in the report)
- Impact: crash / memory corruption / info disclosure / DoS / etc.
- Whether you've coordinated disclosure with anyone else

Acknowledgement within 7 days, fix or status update within 30. Once a
fix is released the advisory becomes public.

## Scope

In scope:

- Crashes, memory corruption, or out-of-bounds reads in the wrapper
  code (`statgrab.c`) reachable from PHP, including the type conversions
  that turn libstatgrab structs into PHP arrays and objects.
- Any bug reachable through `sg_set_valid_filesystems()` /
  `Statgrab::setValidFilesystems()`, the single caller-tainted input:
  embedded-NUL handling, allocation overflow on large arrays, or a
  malformed filesystem-type string reaching native code.
- Arginfo / ZPP mismatches that cause undefined behavior reachable from
  PHP.
- Information disclosure beyond what the calling process is already
  entitled to read from the OS, for example returning another
  namespace's or user's data the caller should not see.

Out of scope:

- Vulnerabilities in libstatgrab itself. For system builds, report to
  the libstatgrab project and update your system package. The bundled
  build statically links a pinned libstatgrab; a relevant upstream fix
  ships here as a rebuild.
- The telemetry values themselves. System counters reflecting real host
  state (process lists, logged-in users, interface stats) are the
  extension working as intended, not a leak. Restrict who can run the
  PHP that calls it.
- Behavior when linked against a libstatgrab older than the 0.92
  minimum. Build against a supported version.
