// //////////////////////////////////////////////////////////
// search.h
// Copyright (c) 2014,2019 Stephan Brumme. All rights reserved.
// see http://create.stephan-brumme.com/disclaimer.html
//

#pragma once

#include <stddef.h> // size_t

// all functions are declared similar to strstr

/// naive approach (for C strings)
const char* searchSimpleString            (const char* haystack, const char* needle);
/// naive approach (for non-text data)
const char* searchSimple                  (const char* haystack, size_t haystackLength,
                                           const char* needle,   size_t needleLength);

/// Knuth-Morris-Pratt algorithm (for C strings)
const char* searchKnuthMorrisPrattString  (const char* haystack, const char* needle);
/// Knuth-Morris-Pratt algorithm (for non-text data)
const char* searchKnuthMorrisPratt        (const char* haystack, size_t haystackLength,
                                           const char* needle,   size_t needleLength);

/// Boyer-Moore-Horspool algorithm (for C strings)
const char* searchBoyerMooreHorspoolString(const char* haystack, const char* needle);
/// Boyer-Moore-Horspool algorithm (for non-text data)
const char* searchBoyerMooreHorspool      (const char* haystack, size_t haystackLength,
                                           const char* needle,   size_t needleLength);

/// Bitap algorithm / Baeza-Yates-Gonnet algorithm (for C strings)
const char* searchBitapString             (const char* haystack, const char* needle);
/// Bitap algorithm / Baeza-Yates-Gonnet algorithm (for non-text data)
const char* searchBitap                   (const char* haystack, size_t haystackLength,
                                           const char* needle,   size_t needleLength);

/// Rabin-Karp algorithm (for C strings)
const char* searchRabinKarpString         (const char *haystack, const char *needle);
/// Rabin-Karp algorithm (for non-text strings)
const char* searchRabinKarp               (const char* haystack, size_t haystackLength,
                                           const char* needle,   size_t needleLength);

/// super-fast for short strings (less than about 8 bytes), else use searchSimple or searchBoyerMooreHorspool
const char* searchNative                  (const char* haystack, size_t haystackLength,
                                           const char* needle,   size_t needleLength);
