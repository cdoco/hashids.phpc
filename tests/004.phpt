--TEST--
Check for hashids hex method
--SKIPIF--
<?php if (!extension_loaded("hashids")) print "skip"; ?>
--FILE--
<?php

$hashids = new Hashids();

$hash = $hashids->encodeHex('FFFFDD'); // rYKPAK
$hex = $hashids->decodeHex('rYKPAK'); // FFFFDD

echo $hash . "\n";
echo $hex . "\n";
?>
--EXPECT--
rYKPAK
FFFFDD
