[![hashids](http://hashids.org/public/img/hashids.gif "Hashids")](http://hashids.org/)

[![License][license-image]][license-url]

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

you can set some options in php.ini, or set in the constructor.
i suggest you in php.ini setting, so you will get better performance.

```ini
[hashids]
extension=hashids.so

//default is empty
hashids.salt=cdoco

//default: 0
hashids.min_hash_length=20

//default: abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890
//you can use to set it according to your own, for example, is set to all lowercase
hashids.alphabet=abcdefghijklmnopqrstuvwxyz
```

## Quick Example

```php
$hashids = new Hashids();

$hash = $hashids->encode(1, 2, 3, 4, 5); // ADf9h9i0sQ
$numbers = $hashids->decode($hash); // [1, 2, 3, 4, 5]
```

## Performance

php extension and native php performance contrast.

![performance comparison](https://cdoco.com/images/performance.png "performance comparison")

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