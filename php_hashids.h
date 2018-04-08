/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: ZiHang Gao <ocdoco@gmail.com>                                |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_HASHIDS_H
#define PHP_HASHIDS_H

extern zend_module_entry hashids_module_entry;
#define phpext_hashids_ptr &hashids_module_entry

#define PHP_HASHIDS_VERSION "0.1.0" /* Replace with version number for your extension */

/* minimal alphabet length */
#define HASHIDS_MIN_ALPHABET_LENGTH 16u

/* separator divisor */
#define HASHIDS_SEPARATOR_DIVISOR 3.5f

/* guard divisor */
#define HASHIDS_GUARD_DIVISOR 12u

/* default salt */
#define HASHIDS_DEFAULT_SALT ""

/* default minimal hash length */
#define HASHIDS_DEFAULT_MIN_HASH_LENGTH 0u

/* default alphabet */
#define HASHIDS_DEFAULT_ALPHABET "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"

/* default separators */
#define HASHIDS_DEFAULT_SEPARATORS "cfhistuCFHISTU"

/* error codes */
#define HASHIDS_ERROR_OK                0
#define HASHIDS_ERROR_ALLOC             -1
#define HASHIDS_ERROR_ALPHABET_LENGTH   -2
#define HASHIDS_ERROR_ALPHABET_SPACE    -3
#define HASHIDS_ERROR_INVALID_HASH      -4
#define HASHIDS_ERROR_INVALID_NUMBER    -5

/* thread-safe hashids_errno indirection */
extern int *__hashids_errno_addr();
#define hashids_errno (*__hashids_errno_addr())

/* alloc & free */
extern void *(*_hashids_alloc)(size_t size);
extern void (*_hashids_free)(void *ptr);

/* the hashids "object" */
struct hashids_s {
    char *alphabet;
    char *alphabet_copy_1;
    char *alphabet_copy_2;
    size_t alphabet_length;

    char *salt;
    size_t salt_length;

    char *separators;
    size_t separators_count;

    char *guards;
    size_t guards_count;

    size_t min_hash_length;
};
typedef struct hashids_s hashids_t;

/* exported function definitions */
void hashids_shuffle(char *str, size_t str_length, char *salt, size_t salt_length);
void hashids_free(hashids_t *hashids);

/* init */
hashids_t * hashids_init(const char *salt, size_t min_hash_length, const char *alphabet);

/* encode */
size_t hashids_estimate_encoded_size(hashids_t *hashids, size_t numbers_count, unsigned long long *numbers);
size_t hashids_encode(hashids_t *hashids, char *buffer, size_t numbers_count, unsigned long long *numbers);
size_t hashids_encode_v(hashids_t *hashids, char *buffer, size_t numbers_count, ...);
size_t hashids_encode_hex(hashids_t *hashids, char *buffer, const char *hex_str);

/* decode */
size_t hashids_numbers_count(hashids_t *hashids, char *str);
size_t hashids_decode(hashids_t *hashids, char *str, unsigned long long *numbers);
size_t hashids_decode_hex(hashids_t *hashids, char *str, char *output);

#ifdef PHP_WIN32
#	define PHP_HASHIDS_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_HASHIDS_API __attribute__ ((visibility("default")))
#else
#	define PHP_HASHIDS_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#define HASHIDS_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(hashids, v)

ZEND_BEGIN_MODULE_GLOBALS(hashids)
	char      *salt;
  zend_long min_hash_length;
  char      *alphabet;
ZEND_END_MODULE_GLOBALS(hashids)

#if defined(ZTS) && defined(COMPILE_DL_HASHIDS)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif	/* PHP_HASHIDS_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
