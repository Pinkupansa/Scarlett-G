/**
 * Author: Simon Lindholm, chilli
 * Date: 2018-07-23
 * License: CC0
 * Source: http://codeforces.com/blog/entry/60737
 * Description: Hash map with mostly the same API as unordered\_map, but \tilde
 * 3x faster. Uses 1.5x memory.
 * Initial capacity must be a power of 2 (if provided).
 */
#pragma once

#include <ext/pb_ds/assoc_container.hpp> /** keep-include */
// To use most bits rather than just the lowest ones:
struct chash { // large odd number for C
	const uint64_t C = (unsigned long long)(4e18 * acos(0)) | 71;
	unsigned long long operator()(unsigned long long x) const { return __builtin_bswap64(x*C); }
};

using hash_table = __gnu_pbds::gp_hash_table<int, int, chash>;
using hash_set = __gnu_pbds::gp_hash_table<int, __gnu_pbds::null_type, chash>;

/** For CodeForces, or other places where hacking might be a problem:

const int RANDOM = chrono::high_resolution_clock::now().time_since_epoch().count();
struct chash { // To use most bits rather than just the lowest ones:
	const uint64_t C = ll(4e18 * acos(0)) | 71; // large odd number
	ll operator()(ll x) const { return __builtin_bswap64((x^RANDOM)*C); }
};
__gnu_pbds::gp_hash_table<ll, int, chash> h({},{},{},{}, {1 << 16});
*/