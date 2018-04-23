--TEST--
Check for hashids hex static method call
--SKIPIF--
<?php if (!extension_loaded("hashids")) print "skip"; ?>
--FILE--
<?php

$hash = Hashids::encodeHex('FFFFDD'); // rYKPAK
$hex = Hashids::decodeHex('rYKPAK'); // FFFFDD

echo $hash . "\n";
echo $hex . "\n";
?>
--EXPECT--
rYKPAK
FFFFDD
