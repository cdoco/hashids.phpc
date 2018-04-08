# hashids.phpc

A php extension for Hashids

## Example

```php
echo (new Hashids('', 0))->encode(1, 2, 3, 4, 5) . "\n";
```

```php
echo (new Hashids('', 0))->encode([1, 2, 3, 4, 5]) . "\n";
```
