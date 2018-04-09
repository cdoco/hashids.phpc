<?php
//encode
echo (new Hashids())->encode(1, 2, 3, 4, 5) . "\n";
echo (new Hashids())->encode([1, 2, 3, 4, 5]) . "\n";
//decode
var_dump((new Hashids())->decode("ADf9h9i0sQ"));
//encodeHex
echo (new Hashids('hhhhh', 30))->encodeHex('C0FFEE') . "\n";
//decodeHex
echo (new Hashids('hhhhh', 30))->decodeHex('xvwLqB4XMbJAEbXxOw6N5lO92YQrKG') . "\n";