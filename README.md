<div align=center>
    <p><img src="https://camo.githubusercontent.com/d9ac39c754c40fae6d66696037c3a33540c2bd09/687474703a2f2f686173686964732e6f72672f7075626c69632f696d672f686173686964732e676966" alt="Hashids"/></p>
    <a target="_blank" href="https://opensource.org/licenses/MIT" title="License: MIT"><img src="https://img.shields.io/badge/License-MIT-blue.svg"></a>
</div>

> A php extension for Hashids

## Requirement

- PHP 7 +

## Install

```shell
$ git clone https://github.com/cdoco/hashids.phpc.git
$ phpize
$ ./configure
$ make && make install
```

php.ini

```ini
[hashids]
extension=hashids.so

hashids.salt=cdoco
hashids.min_hash_length=20
hashids.alphabet=abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890
```

## Quick Example

```php
$hashids = new Hashids();

$hash = $hashids->encode(1, 2, 3, 4, 5); // ADf9h9i0sQ
$numbers = $hashids->decode($hash); // [1, 2, 3, 4, 5]
```

## Other

```php
$hashids = new Hashids();

$hash = $hashids->encode(1, 2, 3, 4, 5); // ADf9h9i0sQ
$hash = $hashids->encode([1, 2, 3, 4, 5]); // ADf9h9i0sQ
```

construct parameter.

```php
new Hashids(string $salt, int $min_hash_length, string $alphabet);

//example
new Hashids("this is salt.", 20, 'abcdefghijklmnopqrstuvwxyz');
```

hex.

```php
$hashids = new Hashids();

$hash = $hashids->encodeHex('FFFFDD'); // rYKPAK
$numbers = $hashids->decodeHex($hash); // FFFFDD
```