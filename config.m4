dnl $Id$
dnl config.m4 for extension hashids

PHP_ARG_ENABLE(hashids, whether to enable hashids support,
[  --enable-hashids           Enable hashids support])

if test "$PHP_HASHIDS" != "no"; then
  PHP_NEW_EXTENSION(hashids, hashids.c, $ext_shared)
fi
