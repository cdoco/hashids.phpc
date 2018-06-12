[![hashids](http://hashids.org/public/img/hashids.gif "Hashids")](http://hashids.org/)

[![Build Status][travis-image]][travis-url]
![PHP](https://img.shields.io/badge/PHP-%3E%3D7.0.0-blue.svg)
![PHP](https://img.shields.io/github/repo-size/cdoco/hashids.phpc.svg)

> A php extension for Hashids

## Requirement

- PHP 7 +

## Install

```shell
$ git clone https://github.com/cdoco/hashids.phpc.git
$ cd hashids.phpc
$ phpize && ./configure && make && make install
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

//or would you prefer to use a static method call
$hash = Hashids::encode(1, 2, 3, 4, 5); // ADf9h9i0sQ
$numbers = Hashids::decode($hash); // [1, 2, 3, 4, 5]
```

## Performance

php extension and only php code performance contrast.

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
$hex = $hashids->decodeHex($hash); // FFFFDD
```

Curses! #$%@
-------

This code was written with the intent of placing created ids in visible places, like the URL. Therefore, the algorithm tries to avoid generating most common English curse words by generating ids that never have the following letters next to each other:

    c, f, h, i, s, t, u

License
-------

MIT License. See the [LICENSE](LICENSE) file. You can use Hashids in open source projects and commercial products. Don't break the Internet. Kthxbye.

[travis-url]: https://travis-ci.org/cdoco/hashids.phpc
[travis-image]: https://travis-ci.org/cdoco/hashids.phpc.svg