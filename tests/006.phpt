--TEST--
Check for hashids encode array arguments
--SKIPIF--
<?php if (!extension_loaded("hashids")) print "skip"; ?>
--FILE--
<?php

$hashids = new Hashids();

$hash = $hashids->encode([1, 2, 3, 4, 5]); // ADf9h9i0sQ
$numbers = $hashids->decode($hash); // [1, 2, 3, 4, 5]

echo $hash . "\n";
print_r($numbers);
?>
--EXPECT--
ADf9h9i0sQ
Array
(
    [0] => 1
    [1] => 2
    [2] => 3
    [3] => 4
    [4] => 5
)
