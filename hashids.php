<?php
echo (new Hashids(''))->encode(1, 2, 3, 4, 5) . "\n";

var_dump((new Hashids(''))->decode("J4aVoWzJiR"));
