--TEST--
2.1: sg_cpu_percent_usage takes a $source arg (cumulative / sliding-window)
--EXTENSIONS--
statgrab
--FILE--
<?php
/* CPU_PERCENT_ENTIRE (default) — cumulative since boot, always available. */
$entire = sg_cpu_percent_usage();
echo is_array($entire) ? "default_ok\n" : "FAIL default\n";

$entire2 = sg_cpu_percent_usage(Statgrab::CPU_PERCENT_ENTIRE);
echo is_array($entire2) ? "entire_ok\n" : "FAIL entire\n";

$sg = new Statgrab();
$entire3 = $sg->cpu(Statgrab::CPU_PERCENT_ENTIRE);
echo is_array($entire3) ? "oo_entire_ok\n" : "FAIL oo entire\n";

$bad = @sg_cpu_percent_usage(99);
var_dump($bad);

echo "DONE\n";
?>
--EXPECT--
default_ok
entire_ok
oo_entire_ok
bool(false)
DONE
