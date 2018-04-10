--TEST--
Check for Hashids encode and decode
--SKIPIF--
<?php if (!extension_loaded("hashids")) print "skip"; ?>
--FILE--
<?php

$hashids = new Hashids();

$hash = $hashids->encode(1, 2, 3, 4, 5); // ADf9h9i0sQ
$numbers = $hashids->decode($hash); // [1, 2, 3, 4, 5]

echo $hash . "\n";
print_r($numbers);

//or would you prefer to use a static method call
$hash = Hashids::encode(1, 2, 3, 4, 5); // ADf9h9i0sQ
$numbers = Hashids::decode($hash); // [1, 2, 3, 4, 5]

echo $hash . "\n";
print_r($numbers);

//decode
$numbers = $hashids->decode("ADf9h9i0sQ");
print_r($numbers);

//encodeHex
$hash = $hashids->encodeHex('FFFFDD'); // rYKPAK
$hex = $hashids->decodeHex('rYKPAK'); // FFFFDD

echo $hash . "\n";
echo $hex;

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
ADf9h9i0sQ
Array
(
    [0] => 1
    [1] => 2
    [2] => 3
    [3] => 4
    [4] => 5
)
Array
(
    [0] => 1
    [1] => 2
    [2] => 3
    [3] => 4
    [4] => 5
)
rYKPAK
FFFFDD
