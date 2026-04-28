<?php
	if (!extension_loaded('statgrab')) {
		dl('statgrab.' . PHP_SHLIB_SUFFIX);
	}
	if (!extension_loaded('statgrab')) {
		die("Please install or enable the statgrab extension.\n");
	}

	$functions = get_extension_funcs('statgrab');
	echo "Functions available in the test extension:\n";
	foreach($functions as $func) {
		echo $func."()\n";
		print_r($func());
	}
	echo "\n";
?>
