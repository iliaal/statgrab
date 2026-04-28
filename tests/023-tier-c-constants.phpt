--TEST--
2.1 Tier C: HOST_STATE_*, FS_*, ERROR_*, CPU_PERCENT_* class constants
--EXTENSIONS--
statgrab
--FILE--
<?php
/* CPU_PERCENT_* */
var_dump(Statgrab::CPU_PERCENT_ENTIRE === 0);
var_dump(Statgrab::CPU_PERCENT_LAST_DIFF === 1);
var_dump(Statgrab::CPU_PERCENT_NEW_DIFF === 2);

/* HOST_STATE_*: host_state field on sg_general_stats() must be one of these */
$valid_states = [
    Statgrab::HOST_STATE_UNKNOWN,
    Statgrab::HOST_STATE_PHYSICAL,
    Statgrab::HOST_STATE_VIRTUAL_MACHINE,
    Statgrab::HOST_STATE_PARAVIRTUAL_MACHINE,
    Statgrab::HOST_STATE_HARDWARE_VIRTUALIZED,
];
$h = sg_general_stats();
var_dump(in_array($h['host_state'], $valid_states, true));

/* FS_* bitmask: LOCAL is REGULAR | SPECIAL, ALLTYPES is the OR of four. */
var_dump(Statgrab::FS_LOCAL    === (Statgrab::FS_REGULAR | Statgrab::FS_SPECIAL));
var_dump(Statgrab::FS_ALLTYPES === (Statgrab::FS_REGULAR | Statgrab::FS_SPECIAL | Statgrab::FS_LOOPBACK | Statgrab::FS_REMOTE));

/* device_type values from sg_fs_stats() must be subsets of FS_ALLTYPES */
$fs = sg_fs_stats();
$bad = false;
foreach ($fs as $row) {
    if (($row['device_type'] & ~Statgrab::FS_ALLTYPES) !== 0) {
        $bad = true;
        break;
    }
}
var_dump($bad === false);

/* ERROR_* */
var_dump(Statgrab::ERROR_NONE === 0);
var_dump(Statgrab::ERROR_PERMISSION > 0);
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
