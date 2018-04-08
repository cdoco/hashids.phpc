# hashids.phpc

A php extension for Hashids

## Example

```php
echo (new Hashids('', 0))->encode(1, 2, 3, 4, 5) . "\n";
```

```php
echo (new Hashids('', 0))->encode([1, 2, 3, 4, 5]) . "\n";
```

```php
var_dump((new Hashids(''))->decode("J4aVoWzJiR"));
```

```php
echo (new Hashids('hhhhh', 30))->encodeHex('C0FFEE') . "\n";
```

```php
echo (new Hashids('hhhhh', 30))->decodeHex('Jq8PxAv5MIBpyv2A9L7S0sRN6WuUHT') . "\n";
```