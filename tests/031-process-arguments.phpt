--TEST--
Process stats argument errors and limit boundaries for both APIs
--EXTENSIONS--
statgrab
--FILE--
<?php
$sg = new Statgrab();
foreach (['sg_process_stats', [$sg, 'processes']] as $call) {
    $name = is_string($call) ? $call : 'Statgrab::processes';
    foreach ([null, 0, 1, 2, 3, 4, 5, 6, 7] as $sort) {
        foreach ([-1, PHP_INT_MIN] as $limit) {
            try {
                $call($sort, $limit);
                echo "FAIL: negative limit accepted\n";
            } catch (ValueError $e) {
                $expected = "$name(): Argument #2 (\$num_entries) must be greater than or equal to 0";
                if ($e->getMessage() !== $expected) {
                    echo "FAIL: ", $e->getMessage(), "\n";
                }
            }
        }
    }

    /* Invalid sort still takes precedence when both arguments are invalid. */
    foreach ([-1, 8, PHP_INT_MIN, PHP_INT_MAX] as $sort) {
        foreach ([0, -1] as $limit) {
            $warnings = [];
            set_error_handler(function ($level, $message) use (&$warnings) {
                $warnings[] = [$level, $message];
                return true;
            });
            try {
                $result = $call($sort, $limit);
            } finally {
                restore_error_handler();
            }
            if ($result !== false || $warnings !== [[E_WARNING,
                "$name(): '$sort' is not a supported sorting mode"]]) {
                echo "FAIL: invalid sort result or warning changed\n";
            }
        }
    }

    foreach ([null, Statgrab::SORT_PID] as $sort) {
        $rows = $call($sort, 1);
        if (!is_array($rows) || count($rows) !== 1 || array_keys($rows) !== [0]) {
            echo "FAIL: limit 1\n";
        }
        foreach ([0, PHP_INT_MAX] as $limit) {
            $rows = $call($sort, $limit);
            if (!is_array($rows) || count($rows) < 1) {
                echo "FAIL: full result limit $limit\n";
            }
        }
    }
    echo "$name OK\n";
}
?>
--EXPECT--
sg_process_stats OK
Statgrab::processes OK
