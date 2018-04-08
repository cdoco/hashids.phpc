<?php
echo (new Hashids(''))->encode(1, 2, 3, 4, 5) . "\n";

var_dump((new Hashids(''))->decode("J4aVoWzJiR"));

echo (new Hashids('hhhhh', 30))->encodeHex('C0FFEE') . "\n";

echo (new Hashids('hhhhh', 30))->decodeHex('Jq8PxAv5MIBpyv2A9L7S0sRN6WuUHT') . "\n";