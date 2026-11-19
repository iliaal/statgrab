--TEST--
sg_valid_filesystems() stops building its list at memory_limit
--EXTENSIONS--
statgrab
--SKIPIF--
<?php
if (getenv('USE_ZEND_ALLOC') === '0') die('skip memory_limit is not enforced with USE_ZEND_ALLOC=0');
?>
--INI--
memory_limit=128M
--FILE--
<?php
/* 20000 entries that share one 4 KiB string cost about 1 MB here, but the
 * getter returns 20000 separate copies, about 80 MB. */
var_dump(sg_set_valid_filesystems(array_fill(0, 20000, str_repeat('x', 4096))));
ini_set('memory_limit', (string)(memory_get_usage(true) + 4 * 1024 * 1024));
$list = sg_valid_filesystems();
echo "not reached\n";
?>
--EXPECTF--
bool(true)

Fatal error: Allowed memory size of %d bytes exhausted %s in %s on line %d
