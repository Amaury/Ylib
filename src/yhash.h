/**
 * @header	yhash.h
 * @abstract	Declarations about hash computation from strings.
 * @link	https://www.python.org/dev/peps/pep-0456/
 * @link	Hash algorithms comparison: http://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed/145633#145633
 * @link	Recent benchmark: https://lonewolfer.wordpress.com/2015/01/05/benchmarking-hash-functions/
 * @link	Up-to-date benchmark: https://github.com/rurban/smhasher
 * @link	Very complete benchmark: http://aras-p.info/blog/2016/08/09/More-Hash-Function-Tests/
 * @link	Another benchmark: https://www.strchr.com/hash_functions
 * @link	Another benchmark: https://research.neustar.biz/2012/02/02/choosing-a-good-hash-function-part-3/
 * @link	http://www.cse.yorku.ca/~oz/hash.html
 * @link	http://en.literateprograms.org/Hash_function_comparison_%28C,_sh%29
 * @link	CityHash (algorithm from Google): https://github.com/google/cityhash
 * @link	CityHash article on Wikipedia: https://en.wikipedia.org/wiki/CityHash
 * @link	C port of CityHash: https://github.com/mulle-nat/cityhash
 * @link	FarmHash, CityHash's successor: https://github.com/google/farmhash
 * @link	C port of FarmHash64: https://github.com/fredrikwidlund/cfarmhash
 * @link	SipHash: https://131002.net/siphash/
 * @link	xxHash: https://github.com/Cyan4973/xxHash
 * @link	t1ha: https://github.com/leo-yuriev/t1ha
 * @link	CRC64 (Redis implementation): http://download.redis.io/redis-stable/src/crc64.c
 */
#pragma once

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

#include "y.h"

/** @typedef yhash_value_t	Type of a hash result. */
typedef uint32_t yhash_value_t;

/**
 * @function	yhash_compute
 *		Compute the hash value of a string, using the SDBM algorithm.
 * @see		http://www.cse.yorku.ca/~oz/hash.html
 * @see		http://en.literateprograms.org/Hash_function_comparison_%28C,_sh%29
 * @param	Key	The data to hash.
 * @return	The computed hash value.
 */
yhash_value_t yhash_compute(const char *key);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */

