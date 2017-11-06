dnl $Id$
dnl config.m4 for extension hashids

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(hashids, for hashids support,
dnl Make sure that the comment is aligned:
dnl [  --with-hashids             Include hashids support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(hashids, whether to enable hashids support,
dnl Make sure that the comment is aligned:
dnl [  --enable-hashids           Enable hashids support])

if test "$PHP_HASHIDS" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-hashids -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/hashids.h"  # you most likely want to change this
  dnl if test -r $PHP_HASHIDS/$SEARCH_FOR; then # path given as parameter
  dnl   HASHIDS_DIR=$PHP_HASHIDS
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for hashids files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       HASHIDS_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$HASHIDS_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the hashids distribution])
  dnl fi

  dnl # --with-hashids -> add include path
  dnl PHP_ADD_INCLUDE($HASHIDS_DIR/include)

  dnl # --with-hashids -> check for lib and symbol presence
  dnl LIBNAME=hashids # you may want to change this
  dnl LIBSYMBOL=hashids # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $HASHIDS_DIR/$PHP_LIBDIR, HASHIDS_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_HASHIDSLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong hashids lib version or lib not found])
  dnl ],[
  dnl   -L$HASHIDS_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(HASHIDS_SHARED_LIBADD)

  PHP_NEW_EXTENSION(hashids, hashids.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
