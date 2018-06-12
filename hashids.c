/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_hashids.h"

zend_class_entry *hashids_ce;
hashids_t *hashids_entry;

ZEND_DECLARE_MODULE_GLOBALS(hashids)

ZEND_BEGIN_ARG_INFO(arginfo_hashids_construct, 0)
    ZEND_ARG_INFO(0, salt)
    ZEND_ARG_INFO(0, min_hash_length)
    ZEND_ARG_INFO(0, alphabet)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_hashids_encode, 0)
    ZEND_ARG_VARIADIC_INFO(0, argc)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_hashids_decode, 0)
    ZEND_ARG_INFO(0, hash)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_hashids_encode_hex, 0)
    ZEND_ARG_INFO(0, hex_str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_hashids_decode_hex, 0)
    ZEND_ARG_INFO(0, hash)
ZEND_END_ARG_INFO()


PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("hashids.salt", HASHIDS_DEFAULT_SALT, PHP_INI_ALL, OnUpdateString, salt, zend_hashids_globals, hashids_globals)
    STD_PHP_INI_ENTRY("hashids.min_hash_length", "0", PHP_INI_ALL, OnUpdateLong, min_hash_length, zend_hashids_globals, hashids_globals)
    STD_PHP_INI_ENTRY("hashids.alphabet", HASHIDS_DEFAULT_ALPHABET, PHP_INI_ALL, OnUpdateString, alphabet, zend_hashids_globals, hashids_globals)
PHP_INI_END()

/* branch prediction hinting */
#ifndef __has_builtin
#   define __has_builtin(x) (0)
#endif
#if defined(__builtin_expect) || __has_builtin(__builtin_expect)
#   define HASHIDS_LIKELY(x)        (__builtin_expect(!!(x), 1))
#   define HASHIDS_UNLIKELY(x)      (__builtin_expect(!!(x), 0))
#else
#   define HASHIDS_LIKELY(x)        (x)
#   define HASHIDS_UNLIKELY(x)      (x)
#endif

/* thread-local storage */
#ifndef TLS
#define TLS
#endif

/* thread-safe hashids_errno indirection */
TLS int __hashids_errno_val;
int * __hashids_errno_addr() {
    return &__hashids_errno_val;
}

/* default alloc() implementation */
static inline void * hashids_alloc_f(size_t size) {
    return calloc(size, 1);
}

/* default free() implementation */
static inline void hashids_free_f(void *ptr) {
    free(ptr);
}

void *(*_hashids_alloc)(size_t size) = hashids_alloc_f;
void (*_hashids_free)(void *ptr) = hashids_free_f;

/* fast ceil(x / y) for size_t arguments */
static inline size_t hashids_div_ceil_size_t(size_t x, size_t y)
{
    return x / y + !!(x % y);
}

/* fast ceil(x / y) for unsigned short arguments */
static inline unsigned short hashids_div_ceil_unsigned_short(unsigned short x, unsigned short y) {
    return x / y + !!(x % y);
}

/* fast log2(x) for unsigned long long */
const unsigned short hashids_log2_64_tab[64] = {
    63,  0, 58,  1, 59, 47, 53,  2,
    60, 39, 48, 27, 54, 33, 42,  3,
    61, 51, 37, 40, 49, 18, 28, 20,
    55, 30, 34, 11, 43, 14, 22,  4,
    62, 57, 46, 52, 38, 26, 32, 41,
    50, 36, 17, 19, 29, 10, 13, 21,
    56, 45, 25, 31, 35, 16,  9, 12,
    44, 24, 15,  8, 23,  7,  6,  5
};

static inline unsigned short hashids_log2_64(unsigned long long x)
{
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;

    /* pure evil : ieee abuse */
    return hashids_log2_64_tab[
        ((unsigned long long)((x - (x >> 1)) * 0x07EDD5E59A4E28C2)) >> 58];
}

/* shuffle loop step */
#define hashids_shuffle_step(iter) \
    if (i == 0) { break; }                                      \
    if (v == salt_length) { v = 0; }                            \
    p += salt[v]; j = (salt[v] + v + p) % (iter);               \
    temp = str[(iter)]; str[(iter)] = str[j]; str[j] = temp;    \
    --i; ++v;

/* consistent shuffle */
void hashids_shuffle(char *str, size_t str_length, char *salt, size_t salt_length)
{
    ssize_t i;
    size_t j, v, p;
    char temp;

    /* meh, meh */
    if (!salt_length) {
        return;
    }

    /* pure evil : loop unroll */
    for (i = str_length - 1, v = 0, p = 0; i > 0; /* empty */) {
        switch (i % 32) {
            case 31: hashids_shuffle_step(i);
            case 30: hashids_shuffle_step(i);
            case 29: hashids_shuffle_step(i);
            case 28: hashids_shuffle_step(i);
            case 27: hashids_shuffle_step(i);
            case 26: hashids_shuffle_step(i);
            case 25: hashids_shuffle_step(i);
            case 24: hashids_shuffle_step(i);
            case 23: hashids_shuffle_step(i);
            case 22: hashids_shuffle_step(i);
            case 21: hashids_shuffle_step(i);
            case 20: hashids_shuffle_step(i);
            case 19: hashids_shuffle_step(i);
            case 18: hashids_shuffle_step(i);
            case 17: hashids_shuffle_step(i);
            case 16: hashids_shuffle_step(i);
            case 15: hashids_shuffle_step(i);
            case 14: hashids_shuffle_step(i);
            case 13: hashids_shuffle_step(i);
            case 12: hashids_shuffle_step(i);
            case 11: hashids_shuffle_step(i);
            case 10: hashids_shuffle_step(i);
            case  9: hashids_shuffle_step(i);
            case  8: hashids_shuffle_step(i);
            case  7: hashids_shuffle_step(i);
            case  6: hashids_shuffle_step(i);
            case  5: hashids_shuffle_step(i);
            case  4: hashids_shuffle_step(i);
            case  3: hashids_shuffle_step(i);
            case  2: hashids_shuffle_step(i);
            case  1: hashids_shuffle_step(i);
            case  0: hashids_shuffle_step(i);
        }
    }
}

/* "destructor" */
void hashids_free(hashids_t *hashids)
{
    if (hashids) {
        if (hashids->alphabet) {
            _hashids_free(hashids->alphabet);
        }
        if (hashids->alphabet_copy_1) {
            _hashids_free(hashids->alphabet_copy_1);
        }
        if (hashids->alphabet_copy_2) {
            _hashids_free(hashids->alphabet_copy_2);
        }
        if (hashids->salt) {
            _hashids_free(hashids->salt);
        }
        if (hashids->separators) {
            _hashids_free(hashids->separators);
        }
        if (hashids->guards) {
            _hashids_free(hashids->guards);
        }

        _hashids_free(hashids);
    }
}

// Hashids construct.
PHP_METHOD(hashids, __construct) {

    zend_string *salt = NULL, *alphabet = NULL;
    zend_long min_hash_length = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "|SlS", &salt, &min_hash_length, &alphabet) == FAILURE) {
        return;
    }

    //not empty salt
    if (salt != NULL) {
        HASHIDS_G(salt) = ZSTR_VAL(salt);
    }

    //not empty min hash length
    if (min_hash_length) {
        HASHIDS_G(min_hash_length) = min_hash_length;
    }

    //not empty alphabet
    if (alphabet != NULL) {
        HASHIDS_G(alphabet) = ZSTR_VAL(alphabet);
    }

    //if the modified one reset
    if (salt != NULL || min_hash_length || alphabet != NULL) {
        hashids_entry = hashids_init(HASHIDS_G(salt), HASHIDS_G(min_hash_length), HASHIDS_G(alphabet));
    }

    RETURN_TRUE;
}

PHP_METHOD(hashids, encode) {

    zend_ulong i, argc = ZEND_NUM_ARGS();
    zval *args = NULL;

    char hash[512];

    ZEND_PARSE_PARAMETERS_START(1, -1)
        Z_PARAM_VARIADIC('+', args, argc)
    ZEND_PARSE_PARAMETERS_END();

    if (argc < 1) {
        WRONG_PARAM_COUNT;
    }

    //numbers
    unsigned long long numbers[Z_TYPE(args[0]) == IS_ARRAY ? zend_array_count(Z_ARRVAL(args[0])) : argc];

    if (Z_TYPE(args[0]) == IS_ARRAY) {
        zval *value = NULL;

        ZEND_HASH_FOREACH_NUM_KEY_VAL(Z_ARRVAL(args[0]), i, value) {
            numbers[i] = Z_LVAL_P(value);
        } ZEND_HASH_FOREACH_END();
    } else {
        //get all the variables
        for (i = 0; i < argc; i++) {
            zval zv = args[i];
            //convert to long
            convert_to_long_ex(&zv);
            numbers[i] = Z_LVAL(zv);
        }
    }

    //encode
    hashids_encode(hashids_entry, hash, sizeof(numbers) / sizeof(unsigned long long), numbers);
    RETURN_STRING(hash);
}

//hashids decode
PHP_METHOD(hashids, decode) {

    int i;
    zend_string *hash = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &hash) == FAILURE) {
        return;
    }

    //get numbers count
    size_t numbers_count = hashids_numbers_count(hashids_entry, ZSTR_VAL(hash));

    //decode
    unsigned long long numbers[numbers_count];
    hashids_decode(hashids_entry, ZSTR_VAL(hash), numbers);

    //array init
    array_init_size(return_value, numbers_count);

    for (i = 0; i < numbers_count; i++) {
        add_next_index_long(return_value, numbers[i]);
    }
}

//hashids encodeHex
PHP_METHOD(hashids, encodeHex) {

    char hash[512];
    zend_string *hex_str = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &hex_str) == FAILURE) {
        return;
    }

    //encode
    hashids_encode_hex(hashids_entry, hash, ZSTR_VAL(hex_str));
    RETURN_STRING(hash);
}

//hashids decodeHex
PHP_METHOD(hashids, decodeHex) {

    zend_string *hash = NULL;
    char hex_str[sizeof(unsigned long long) * 2 + 2];
    
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &hash) == FAILURE) {
        return;
    }

    //decode
    hashids_decode_hex(hashids_entry, ZSTR_VAL(hash), hex_str);
    RETURN_STRING(hex_str);
}

static const zend_function_entry hashids_methods[] = {
    PHP_ME(hashids, __construct, arginfo_hashids_construct, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
    PHP_ME(hashids, encode, arginfo_hashids_encode, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(hashids, decode, arginfo_hashids_decode, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(hashids, encodeHex, arginfo_hashids_encode_hex, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(hashids, decodeHex, arginfo_hashids_decode_hex, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_FE_END
};

/* GINIT */
PHP_GINIT_FUNCTION(hashids) {
    hashids_globals->salt = NULL;
    hashids_globals->min_hash_length = 0;
    hashids_globals->alphabet = HASHIDS_DEFAULT_ALPHABET;
}

PHP_MINIT_FUNCTION(hashids)
{
    REGISTER_INI_ENTRIES();

    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Hashids", hashids_methods);

    hashids_ce = zend_register_internal_class(&ce);
    hashids_entry = hashids_init(HASHIDS_G(salt), HASHIDS_G(min_hash_length), HASHIDS_G(alphabet));

    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(hashids)
{
    UNREGISTER_INI_ENTRIES();

    //free
    hashids_free(hashids_entry);
    return SUCCESS;
}

PHP_MINFO_FUNCTION(hashids)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "hashids support", "enabled");
    php_info_print_table_row(2, "Version", PHP_HASHIDS_VERSION);
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}

/*
Copyright (C) 2014 Latchezar Tzvetkoff

Permission is hereby granted, free of charge, 
to any person obtaining a copy of this software and associated documentation files (the "Software"), 
to deal in the Software without restriction, including without limitation the rights to use, copy, 
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial 
portions of the Software. d

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
DEALINGS IN THE SOFTWARE.
*/
/* common init */
hashids_t * hashids_init(const char *salt, size_t min_hash_length, const char *alphabet)
{
    hashids_t *result;
    size_t i, j, len;
    char ch, *p;

    hashids_errno = HASHIDS_ERROR_OK;

    /* allocate the structure */
    result = _hashids_alloc(sizeof(hashids_t));
    if (HASHIDS_UNLIKELY(!result)) {
        hashids_errno = HASHIDS_ERROR_ALLOC;
        return NULL;
    }

    /* allocate enough space for the alphabet */
    len = strlen(alphabet) + 1;
    result->alphabet = _hashids_alloc(len);

    /* extract only the unique characters */
    result->alphabet[0] = '\0';
    for (i = 0, j = 0; i < len; ++i) {
        ch = alphabet[i];
        if (!strchr(result->alphabet, ch)) {
            result->alphabet[j++] = ch;
        }
    }
    result->alphabet[j] = '\0';

    /* store alphabet length */
    result->alphabet_length = j;

    /* check length and whitespace */
    if (result->alphabet_length < HASHIDS_MIN_ALPHABET_LENGTH) {
        hashids_free(result);
        hashids_errno = HASHIDS_ERROR_ALPHABET_LENGTH;
        return NULL;
    }
    if (strchr(result->alphabet, 0x20) || strchr(result->alphabet, 0x09)) {
        hashids_free(result);
        hashids_errno = HASHIDS_ERROR_ALPHABET_SPACE;
        return NULL;
    }

    /* copy salt */
    result->salt_length = salt ? strlen(salt) : 0;
    result->salt = _hashids_alloc(result->salt_length + 1);
    if (HASHIDS_UNLIKELY(!result->salt)) {
        hashids_free(result);
        hashids_errno = HASHIDS_ERROR_ALLOC;
        return NULL;
    }
    strncpy(result->salt, salt, result->salt_length);

    /* allocate enough space for separators */
    len = strlen(HASHIDS_DEFAULT_SEPARATORS);
    j = (size_t)
        (ceil((float)result->alphabet_length / HASHIDS_SEPARATOR_DIVISOR) + 1);
    if (j < len + 1) {
        j = len + 1;
    }

    result->separators = _hashids_alloc(j);
    if (HASHIDS_UNLIKELY(!result->separators)) {
        hashids_free(result);
        hashids_errno = HASHIDS_ERROR_ALLOC;
        return NULL;
    }

    /* take default separators out of the alphabet */
    for (i = 0, j = 0; i < strlen(HASHIDS_DEFAULT_SEPARATORS); ++i) {
        ch = HASHIDS_DEFAULT_SEPARATORS[i];

        /* check if separator is actually in the used alphabet */
        if ((p = strchr(result->alphabet, ch))) {
            result->separators[j++] = ch;

            /* remove that separator */
            memmove(p, p + 1,
                strlen(result->alphabet) - (p - result->alphabet));
        }
    }

    /* store separators length */
    result->separators_count = j;

    /* subtract separators count from alphabet length */
    result->alphabet_length -= result->separators_count;

    /* shuffle the separators */
    if (result->separators_count) {
        hashids_shuffle(result->separators, result->separators_count,
            result->salt, result->salt_length);
    }

    /* check if we have any/enough separators */
    if (!result->separators_count
        || (((float)result->alphabet_length / (float)result->separators_count)
            > HASHIDS_SEPARATOR_DIVISOR)) {
        size_t separators_count = (size_t)ceil(
            (float)result->alphabet_length / HASHIDS_SEPARATOR_DIVISOR);

        if (separators_count == 1) {
            separators_count = 2;
        }

        if (separators_count > result->separators_count) {
            /* we need more separators - get some from alphabet */
            size_t diff = separators_count - result->separators_count;
            strncat(result->separators, result->alphabet, diff);
            memmove(result->alphabet, result->alphabet + diff,
                result->alphabet_length - diff + 1);

            result->separators_count += diff;
            result->alphabet_length -= diff;
        } else {
            /* we have more than enough - truncate */
            result->separators[separators_count] = '\0';
            result->separators_count = separators_count;
        }
    }

    /* shuffle alphabet */
    hashids_shuffle(result->alphabet, result->alphabet_length,
        result->salt, result->salt_length);

    /* allocate guards */
    result->guards_count = hashids_div_ceil_size_t(result->alphabet_length,
        HASHIDS_GUARD_DIVISOR);
    result->guards = _hashids_alloc(result->guards_count + 1);
    if (HASHIDS_UNLIKELY(!result->guards)) {
        hashids_free(result);
        hashids_errno = HASHIDS_ERROR_ALLOC;
        return NULL;
    }

    if (HASHIDS_UNLIKELY(result->alphabet_length < 3)) {
        /* take some from separators */
        strncpy(result->guards, result->separators, result->guards_count);
        memmove(result->separators, result->separators + result->guards_count,
            result->separators_count - result->guards_count + 1);

        result->separators_count -= result->guards_count;
    } else {
        /* take them from alphabet */
        strncpy(result->guards, result->alphabet, result->guards_count);
        memmove(result->alphabet, result->alphabet + result->guards_count,
            result->alphabet_length - result->guards_count + 1);

        result->alphabet_length -= result->guards_count;
    }

    /* allocate enough space for the alphabet copies */
    result->alphabet_copy_1 = _hashids_alloc(result->alphabet_length + 1);
    result->alphabet_copy_2 = _hashids_alloc(result->alphabet_length + 1);
    if (HASHIDS_UNLIKELY(!result->alphabet || !result->alphabet_copy_1
        || !result->alphabet_copy_2)) {
        hashids_free(result);
        hashids_errno = HASHIDS_ERROR_ALLOC;
        return NULL;
    }

    /* set min hash length */
    result->min_hash_length = min_hash_length;

    /* return result happily */
    return result;
}

/* estimate buffer size (generic) */
size_t hashids_estimate_encoded_size(hashids_t *hashids, size_t numbers_count, unsigned long long *numbers)
{
    int i, result_len;

    for (i = 0, result_len = 1; i < numbers_count; ++i) {
        if (numbers[i] == 0) {
            result_len += 2;
        } else if (numbers[i] == 0xFFFFFFFFFFFFFFFFull) {
            result_len += hashids_div_ceil_unsigned_short(
                hashids_log2_64(numbers[i]),
                hashids_log2_64(hashids->alphabet_length)) - 1;
        } else {
            result_len += hashids_div_ceil_unsigned_short(
                hashids_log2_64(numbers[i] + 1),
                hashids_log2_64(hashids->alphabet_length));
        }
    }

    if (numbers_count > 1) {
        result_len += numbers_count - 1;
    }

    if (result_len < hashids->min_hash_length) {
        result_len = hashids->min_hash_length;
    }

    return result_len + 2 /* fast log2 & ceil sometimes undershoot by 1 */;
}

/* encode many (generic) */
size_t hashids_encode(hashids_t *hashids, char *buffer, size_t numbers_count, unsigned long long *numbers)
{
    /* bail out if no numbers */
    if (HASHIDS_UNLIKELY(!numbers_count)) {
        buffer[0] = '\0';

        return 0;
    }

    size_t i, j, result_len, guard_index, half_length_ceil, half_length_floor;
    unsigned long long number, number_copy, numbers_hash;
    int p_max;
    char lottery, ch, temp_ch, *p, *buffer_end, *buffer_temp;

    /* return an estimation if no buffer */
    if (HASHIDS_UNLIKELY(!buffer)) {
        return hashids_estimate_encoded_size(hashids, numbers_count, numbers);
    }

    /* copy the alphabet into internal buffer 1 */
    strncpy(hashids->alphabet_copy_1, hashids->alphabet,
        hashids->alphabet_length);

    /* walk arguments once and generate a hash */
    for (i = 0, numbers_hash = 0; i < numbers_count; ++i) {
        number = numbers[i];
        numbers_hash += number % (i + 100);
    }

    /* lottery character */
    lottery = hashids->alphabet[numbers_hash % hashids->alphabet_length];

    /* start output buffer with it (or don't) */
    buffer[0] = lottery;
    buffer_end = buffer + 1;

    /* alphabet-like buffer used for salt at each iteration */
    hashids->alphabet_copy_2[0] = lottery;
    hashids->alphabet_copy_2[1] = '\0';
    strncat(hashids->alphabet_copy_2, hashids->salt,
        hashids->alphabet_length - 1);
    p = hashids->alphabet_copy_2 + hashids->salt_length + 1;
    p_max = hashids->alphabet_length - 1 - hashids->salt_length;
    if (p_max > 0) {
        strncat(hashids->alphabet_copy_2, hashids->alphabet,
            p_max);
    } else {
        hashids->alphabet_copy_2[hashids->alphabet_length] = '\0';
    }

    for (i = 0; i < numbers_count; ++i) {
        /* take number */
        number = number_copy = numbers[i];

        /* create a salt for this iteration */
        if (p_max > 0) {
            strncpy(p, hashids->alphabet_copy_1, p_max);
        }

        /* shuffle the alphabet */
        hashids_shuffle(hashids->alphabet_copy_1, hashids->alphabet_length,
            hashids->alphabet_copy_2, hashids->alphabet_length);

        /* hash the number */
        buffer_temp = buffer_end;
        do {
            ch = hashids->alphabet_copy_1[number % hashids->alphabet_length];
            *buffer_end++ = ch;
            number /= hashids->alphabet_length;
        } while (number);

        /* reverse the hash we got */
        for (j = 0; j < (buffer_end - buffer_temp) / 2; ++j) {
            temp_ch = *(buffer_temp + j);
            *(buffer_temp + j) = *(buffer_end - 1 - j);
            *(buffer_end - 1 - j) = temp_ch;
        }

        if (i + 1 < numbers_count) {
            number_copy %= ch + i;
            *buffer_end = hashids->separators[number_copy %
                hashids->separators_count];
            ++buffer_end;
        }
    }

    /* intermediate string length */
    result_len = buffer_end - buffer;

    if (result_len < hashids->min_hash_length) {
        /* add a guard before the encoded numbers */
        guard_index = (numbers_hash + buffer[0]) % hashids->guards_count;
        memmove(buffer + 1, buffer, result_len);
        buffer[0] = hashids->guards[guard_index];
        ++result_len;

        if (result_len < hashids->min_hash_length) {
            /* add a guard after the encoded numbers */
            guard_index = (numbers_hash + buffer[2]) % hashids->guards_count;
            buffer[result_len] = hashids->guards[guard_index];
            ++result_len;

            /* pad with half alphabet before and after */
            half_length_ceil = hashids_div_ceil_size_t(
                hashids->alphabet_length, 2);
            half_length_floor = floor((float)hashids->alphabet_length / 2);

            /* pad, pad, pad */
            while (result_len < hashids->min_hash_length) {
                /* shuffle the alphabet */
                strncpy(hashids->alphabet_copy_2, hashids->alphabet_copy_1,
                    hashids->alphabet_length);
                hashids_shuffle(hashids->alphabet_copy_1,
                    hashids->alphabet_length, hashids->alphabet_copy_2,
                    hashids->alphabet_length);

                /* left pad from the end of the alphabet */
                i = hashids_div_ceil_size_t(
                    hashids->min_hash_length - result_len, 2);
                /* right pad from the beginning */
                j = floor((float)(hashids->min_hash_length - result_len) / 2);

                /* check bounds */
                if (i > half_length_ceil) {
                    i = half_length_ceil;
                }
                if (j > half_length_floor) {
                    j = half_length_floor;
                }

                /* handle excessively excessive excess */
                if ((i + j) % 2 == 0 && hashids->alphabet_length % 2 == 1) {
                    ++i; --j;
                }

                /* move the current result to "center" */
                memmove(buffer + i, buffer, result_len);
                /* pad left */
                memmove(buffer,
                    hashids->alphabet_copy_1 + hashids->alphabet_length - i, i);
                /* pad right */
                memmove(buffer + i + result_len, hashids->alphabet_copy_1, j);

                /* increment result_len */
                result_len += i + j;
            }
        }
    }

    buffer[result_len] = '\0';
    return result_len;
}

/* numbers count */
size_t hashids_numbers_count(hashids_t *hashids, char *str)
{
    size_t numbers_count;
    char ch, *p;

    /* skip characters until we find a guard */
    if (hashids->min_hash_length) {
        p = str;
        while ((ch = *p)) {
            if (strchr(hashids->guards, ch)) {
                str = p + 1;
                break;
            }

            p++;
        }
    }

    /* parse */
    numbers_count = 0;
    while ((ch = *str)) {
        if (strchr(hashids->guards, ch)) {
            break;
        }
        if (strchr(hashids->separators, ch)) {
            numbers_count++;
            str++;
            continue;
        }
        if (!strchr(hashids->alphabet, ch)) {
            hashids_errno = HASHIDS_ERROR_INVALID_HASH;
            return 0;
        }

        str++;
    }

    /* account for the last number */
    return numbers_count + 1;
}

/* decode */
size_t hashids_decode(hashids_t *hashids, char *str, unsigned long long *numbers)
{
    size_t numbers_count;
    unsigned long long number;
    char lottery, ch, *p, *c;
    int p_max;

    numbers_count = hashids_numbers_count(hashids, str);

    if (!numbers) {
        return numbers_count;
    }

    /* skip characters until we find a guard */
    if (hashids->min_hash_length) {
        p = str;
        while ((ch = *p)) {
            if (strchr(hashids->guards, ch)) {
                str = p + 1;
                break;
            }

            p++;
        }
    }

    /* get the lottery character */
    lottery = *str++;

    /* copy the alphabet into internal buffer 1 */
    strncpy(hashids->alphabet_copy_1, hashids->alphabet,
        hashids->alphabet_length);

    /* alphabet-like buffer used for salt at each iteration */
    hashids->alphabet_copy_2[0] = lottery;
    hashids->alphabet_copy_2[1] = '\0';
    strncat(hashids->alphabet_copy_2, hashids->salt,
        hashids->alphabet_length - 1);
    p = hashids->alphabet_copy_2 + hashids->salt_length + 1;
    p_max = hashids->alphabet_length - 1 - hashids->salt_length;
    if (p_max > 0) {
        strncat(hashids->alphabet_copy_2, hashids->alphabet,
            p_max);
    } else {
        hashids->alphabet_copy_2[hashids->alphabet_length] = '\0';
    }

    /* first shuffle */
    hashids_shuffle(hashids->alphabet_copy_1, hashids->alphabet_length,
        hashids->alphabet_copy_2, hashids->alphabet_length);

    /* parse */
    number = 0;
    while ((ch = *str)) {
        if (strchr(hashids->guards, ch)) {
            break;
        }
        if (strchr(hashids->separators, ch)) {
            *numbers++ = number;
            number = 0;

            /* resalt the alphabet */
            if (p_max > 0) {
                strncpy(p, hashids->alphabet_copy_1, p_max);
            }
            hashids_shuffle(hashids->alphabet_copy_1, hashids->alphabet_length,
                hashids->alphabet_copy_2, hashids->alphabet_length);

            str++;
            continue;
        }
        if (!(c = strchr(hashids->alphabet_copy_1, ch))) {
            hashids_errno = HASHIDS_ERROR_INVALID_HASH;
            return 0;
        }

        number *= hashids->alphabet_length;
        number += c - hashids->alphabet_copy_1;

        str++;
    }

    /* store last number */
    *numbers = number;

    return numbers_count;
}

/* encode hex */
size_t hashids_encode_hex(hashids_t *hashids, char *buffer, const char *hex_str)
{
    int len;
    char *temp, *p;
    size_t result;
    unsigned long long number;

    len = strlen(hex_str);
    temp = _hashids_alloc(len + 2);

    if (!temp) {
        hashids_errno = HASHIDS_ERROR_ALLOC;
        return 0;
    }

    temp[0] = '1';
    strncpy(temp + 1, hex_str, len);

    number = strtoull(temp, &p, 16);

    if (p == temp) {
        _hashids_free(temp);
        hashids_errno = HASHIDS_ERROR_INVALID_NUMBER;
        return 0;
    }

    result = hashids_encode(hashids, buffer, 1, &number);
    _hashids_free(temp);

    return result;
}

/* decode hex */
size_t hashids_decode_hex(hashids_t *hashids, char *str, char *output)
{
    size_t result, i;
    unsigned long long number;
    char ch, *temp;

    result = hashids_numbers_count(hashids, str);

    if (result != 1) {
        return 0;
    }

    result = hashids_decode(hashids, str, &number);

    if (result != 1) {
        return 0;
    }

    temp = output;

    do {
        ch = number % 16;
        if (ch > 9) {
            ch += 'A' - 10;
        } else {
            ch += '0';
        }

        *temp++ = (char)ch;

        number /= 16;
    } while (number);

    temp--;
    *temp = 0;

    for (i = 0; i < (temp - output) / 2; ++i) {
        ch = *(output + i);
        *(output + i) = *(temp - 1 - i);
        *(temp - 1 - i) = ch;
    }

    return 1;
}

zend_module_entry hashids_module_entry = {
    STANDARD_MODULE_HEADER,
    "hashids",
    NULL,
    PHP_MINIT(hashids),
    PHP_MSHUTDOWN(hashids),
    NULL,
    NULL,
    PHP_MINFO(hashids),
    PHP_HASHIDS_VERSION,
    PHP_MODULE_GLOBALS(hashids),
    PHP_GINIT(hashids),
    NULL,
    NULL,
    STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_HASHIDS
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(hashids)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
