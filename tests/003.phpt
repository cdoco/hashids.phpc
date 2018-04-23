--TEST--
Check for hashids static method call
--SKIPIF--
<?php if (!extension_loaded("hashids")) print "skip"; ?>
--FILE--
<?php

$hash = Hashids::encode(1, 2, 3, 4, 5); // ADf9h9i0sQ
$numbers = Hashids::decode($hash); // [1, 2, 3, 4, 5]

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