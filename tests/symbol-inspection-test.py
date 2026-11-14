"""Exercise the real shell runners with deterministic inspection commands."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


class SymbolInspectionTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        (self.root / 'tests').mkdir()
        (self.root / 'modules').mkdir()
        (self.root / 'modules/statgrab.so').touch()
        (self.root / 'tests/c').mkdir()
        lib = self.root / 'vendor/libstatgrab/src/libstatgrab'
        (lib / '.libs').mkdir(parents=True)
        (lib / '.libs/libstatgrab.a').touch()
        (lib / 'statgrab.h').touch()
        self.bin = self.root / 'bin'
        self.bin.mkdir()
        for name in ('bundled-static.sh', 'zts-valid-filesystems.sh',
                     'c/cmdline-title.sh', 'c/diff-shrink.sh'):
            shutil.copyfile(Path(__file__).parent / name, self.root / 'tests' / name)
        self.command('uname', 'echo Linux')
        self.command('ldd', 'printf "%s\\n" "$LDD_OUTPUT"; exit "$LDD_STATUS"')
        self.command('nm', 'printf "%s\\n" "$NM_OUTPUT"; exit "$NM_STATUS"')
        self.command('php-config', 'case "$1" in --php-sapis) echo embed;; --prefix) echo "$TEST_ROOT";; esac')
        self.command('cc', 'echo unexpected-compiler-call >&2; exit 99')
        self.env = dict(os.environ, LIBSTATGRAB_DIR=str(self.root / 'vendor/libstatgrab'), PATH=str(self.bin) + os.pathsep + os.environ['PATH'],
                        TEST_ROOT=str(self.root), LDD_OUTPUT='libc.so.6', LDD_STATUS='0',
                        NM_OUTPUT='0000 T sg_init', NM_STATUS='0', CC=str(self.bin / 'cc'))

    def command(self, name, body):
        path = self.bin / name
        path.write_text('#!/bin/sh\n' + body + '\n')
        path.chmod(0o755)

    def run_script(self, name, *args):
        return subprocess.run(['sh', str(self.root / 'tests' / name), *args],
                              env=self.env, text=True, capture_output=True)

    def test_bundled_success(self):
        result = self.run_script('bundled-static.sh')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('PASS: bundled libstatgrab linked', result.stdout)

    def test_bundled_failures(self):
        cases = [
            {'LDD_STATUS': '23'},
            {'LDD_OUTPUT': 'libstatgrab.so.10'},
            {'NM_OUTPUT': '0000 T unrelated'},
            {'NM_OUTPUT': '', 'NM_STATUS': '23'},
            {'NM_OUTPUT': '0000 T sg_init', 'NM_STATUS': '23'},
        ]
        for change in cases:
            with self.subTest(change=change):
                old = self.env.copy()
                self.env.update(change)
                result = self.run_script('bundled-static.sh')
                self.env = old
                self.assertNotEqual(result.returncode, 0, result.stdout)
                self.assertNotIn('PASS: bundled libstatgrab linked', result.stdout)

    def test_zts_failed_inspection_stops_before_compiling(self):
        for output in ('', '                 U __asan_init'):
            with self.subTest(output=output):
                self.env.update(NM_OUTPUT=output, NM_STATUS='23', CI='')
                result = self.run_script('zts-valid-filesystems.sh', str(self.bin / 'php-config'))
                self.assertEqual(result.returncode, 23, result.stdout + result.stderr)
                self.assertNotIn('unexpected-compiler-call', result.stderr)
                self.assertNotIn('has no ASan runtime', result.stderr)


    def test_c_runners_failed_inspection_stops_before_compiling(self):
        for runner in ('cmdline-title', 'diff-shrink'):
            for output in ('', '                 U __asan_report_load8'):
                with self.subTest(runner=runner, output=output):
                    self.env.update(NM_OUTPUT=output, NM_STATUS='23')
                    result = self.run_script('c/' + runner + '.sh')
                    self.assertEqual(result.returncode, 23, result.stdout + result.stderr)
                    self.assertNotIn('unexpected-compiler-call', result.stderr)
                    self.assertNotIn('is not ASan-instrumented', result.stderr)


if __name__ == '__main__':
    unittest.main()
