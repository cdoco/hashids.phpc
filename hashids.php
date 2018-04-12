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
echo $hex . "\n";