// //////////////////////////////////////////////////////////
// search.c
// Copyright (c) 2014,2019 Stephan Brumme. All rights reserved.
// see http://create.stephan-brumme.com/disclaimer.html
//

// compiles with: gcc -Wall
// pretty much every decent C compiler should be able to eat this code, too
// note: some warnings about C++ comments and mixing declarations and code when enabling -pedantic
//       but they all disappear in C99 mode

#include "search.h"

#include <string.h> // strlen
#include <stdlib.h> // malloc / free


/// naive approach (for C strings)
const char* searchSimpleString(const char* haystack, const char* needle)
{
  // detect invalid input
  if (!haystack || !needle)
    return NULL;

  // until the end of haystack (or a match, of course)
  while (*haystack)
  {
    // compare current haystack to needle
    size_t i = 0;
    while (needle[i])
    {
      if (haystack[i] != needle[i])
        break;
      i++;
    }

    // needle fully matched (reached terminating NUL-byte)
    if (!needle[i])
      return haystack;

    // no match, step forward
    haystack++;
  }

  // not found
  return NULL;
}


/// naive approach (for non-text data)
const char* searchSimple(const char* haystack, size_t haystackLength,
                         const char* needle,   size_t needleLength)
{
  // detect invalid input
  if (!haystack || !needle || haystackLength < needleLength)
    return NULL;

  // match impossible if less than needleLength bytes left
  haystackLength -= needleLength - 1;
  // points beyond last considered start byte
  const char* haystackEnd = haystack + haystackLength;

  // until the end of haystack (or a match, of course ...)
  while (haystack != haystackEnd)
  {
    // compare current haystack to needle
    size_t i = 0;
    for (; i < needleLength; i++)
      if (haystack[i] != needle[i])
        break;

    // needle fully matched
    if (i == needleLength)
      return haystack;

    // no match, step forward
    haystack++;
  }

  // not found
  return NULL;
}


// //////////////////////////////////////////////////////////


/// Knuth-Morris-Pratt algorithm (for C strings)
const char* searchKnuthMorrisPrattString(const char* haystack, const char* needle)
{
  // detect invalid input
  if (!haystack || !needle)
    return NULL;

  // empty needle matches everything
  size_t needleLength = strlen(needle);
  if (needleLength == 0)
    return haystack;

  // try to use stack instead of heap (avoid slow memory allocations if possible)
  const size_t MaxLocalMemory = 256;
  int localMemory[MaxLocalMemory];
  int* skip = localMemory;
  // stack too small => allocate heap
  if (needleLength > MaxLocalMemory)
  {
    skip = (int*)malloc(needleLength * sizeof(int));
    if (skip == NULL)
      return NULL;
  }

  // prepare skip table
  skip[0] = -1;
  int i;
  for (i = 0; needle[i]; i++)
  {
    skip[i + 1] = skip[i] + 1;
    while (skip[i + 1] > 0 && needle[i] != needle[skip[i + 1] - 1])
      skip[i + 1] = skip[skip[i + 1] - 1] + 1;
  }

  // assume no match
  const char* result = NULL;
  int shift = 0;
  // search
  while (*haystack)
  {
    // look for a matching character
    while (shift >= 0 && *haystack != needle[shift])
      shift = skip[shift];

    // single step forward in needle and haystack
    haystack++;
    shift++;

    // reached end of needle => hit
    if (!needle[shift])
    {
      result = haystack - shift;
      break;
    }
  }

  // clean up heap (if used)
  if (skip != localMemory)
    free(skip);

  // points to match position or NULL if not found
  return result;
}


/// Knuth-Morris-Pratt algorithm (for non-text data)
const char* searchKnuthMorrisPratt(const char* haystack, size_t haystackLength,
                                   const char* needle,   size_t needleLength)
{
  // detect invalid input
  if (!haystack || !needle || haystackLength < needleLength)
    return NULL;

  // empty needle matches everything
  if (needleLength == 0)
    return haystack;

  // try to use stack instead of heap (avoid slow memory allocations if possible)
  const size_t MaxLocalMemory = 256;
  int localMemory[MaxLocalMemory];
  int* skip = localMemory;
  // stack too small => allocate heap
  if (needleLength > MaxLocalMemory)
  {
    skip = (int*)malloc(needleLength * sizeof(int));
    if (skip == NULL)
      return NULL;
  }

  // prepare skip table
  skip[0] = -1;
  size_t i;
  for (i = 0; i < needleLength; i++)
  {
    skip[i + 1] = skip[i] + 1;
    while (skip[i + 1] > 0 && needle[i] != needle[skip[i + 1] - 1])
      skip[i + 1] = skip[skip[i + 1]-1] + 1;
  }

  // assume no match
  const char* result = NULL;
  const char* haystackEnd = haystack + haystackLength;
  int shift = 0;
  // search
  while (haystack != haystackEnd)
  {
    // look for a matching character
    while (shift >= 0 && *haystack != needle[shift])
      shift = skip[shift];

    // single step forward in needle and haystack
    haystack++;
    shift++;

    // reached end of needle => hit
    if ((size_t)shift == needleLength)
    {
      result = haystack - shift;
      break;
    }
  }

  // clean up heap (if used)
  if (skip != localMemory)
    free(skip);

  // points to match position or NULL if not found
  return result;
}


// //////////////////////////////////////////////////////////


/// Boyer-Moore-Horspool algorithm (for C strings)
const char* searchBoyerMooreHorspoolString(const char* haystack, const char* needle)
{
  // detect invalid input
  if (!haystack || !needle)
    return NULL;

  // call routine for non-text data
  return searchBoyerMooreHorspool(haystack, strlen(haystack), needle, strlen(needle));
}


/// Boyer-Moore-Horspool algorithm (for non-text data)
const char* searchBoyerMooreHorspool(const char* haystack, size_t haystackLength,
                                     const char* needle,   size_t needleLength)
{
  // detect invalid input
  if (!haystack || !needle || haystackLength < needleLength)
    return NULL;

  // empty needle matches everything
  if (needleLength == 0)
    return haystack;

  // find right-most position of each character
  // and store its distance to the end of needle

  // default value: when a character in haystack isn't in needle, then
  //                we can jump forward needleLength bytes
  const size_t NumChar = 1 << (8 * sizeof(char));
  size_t skip[NumChar];
  size_t i;
  for (i = 0; i < NumChar; i++)
    skip[i] = needleLength;

  // figure out for each character of the needle how much we can skip
  // (if a character appears multiple times in needle, later occurrences
  //  overwrite previous ones, i.e. the value of skip[x] decreases)
  const size_t lastPos = needleLength - 1;
  size_t pos;
  for (pos = 0; pos < lastPos; pos++)
    skip[(unsigned char)needle[pos]] = lastPos - pos;

  // now walk through the haystack
  while (haystackLength >= needleLength)
  {
    // all characters match ?
    for (i = lastPos; haystack[i] == needle[i]; i--)
      if (i == 0)
        return haystack;

    // no match, jump ahead
    unsigned char marker = (unsigned char) haystack[lastPos];
    haystackLength -= skip[marker];
    haystack       += skip[marker];
  }

  // needle not found in haystack
  return NULL;
}


// //////////////////////////////////////////////////////////


/// Bitap algorithm / Baeza-Yates-Gonnet algorithm (for C strings)
const char* searchBitapString(const char* haystack, const char* needle)
{
  // detect invalid input
  if (!haystack || !needle)
    return NULL;

  // empty needle matches everything
  size_t needleLength = strlen(needle);
  if (needleLength == 0)
    return haystack;

  // create bit masks for each possible byte / ASCII character
  // each mask is as wide as needleLength
  const size_t MaxBitWidth = 8 * sizeof(int) - 1;
  // only if needleLength bits fit into an integer (minus 1), the algorithm will be fast
  if (needleLength > MaxBitWidth)
    return searchSimpleString(haystack, needle);

  // one mask per allowed character (1 byte => 2^8 => 256)
  // where all bits are set except those where the character is found in needle
  const size_t AlphabetSize = 256;
  unsigned int masks[AlphabetSize];
  size_t i;
  for (i = 0; i < AlphabetSize; i++)
    masks[i] = ~0;
  for (i = 0; i < needleLength; i++)
    masks[(unsigned char)needle[i]] &= ~(1 << i);

  // initial state mask has all bits set except the lowest one
  unsigned int state = ~1;
  const unsigned int FullMatch = 1 << needleLength;
  while (*haystack)
  {
    // update the bit array
    state  |= masks[(unsigned char)*haystack];
    state <<= 1;

    // if an unset bit "bubbled up" we have a match
    if ((state & FullMatch) == 0)
      return (haystack - needleLength) + 1;

    haystack++;
  }

  // needle not found in haystack
  return NULL;
}


/// Bitap algorithm / Baeza-Yates-Gonnet algorithm (for C strings)
const char* searchBitap(const char* haystack, size_t haystackLength,
                        const char* needle,   size_t needleLength)
{
  // detect invalid input
  if (!haystack || !needle || haystackLength < needleLength)
    return NULL;

  // empty needle matches everything
  if (needleLength == 0)
    return haystack;

  // create bit masks for each possible byte / ASCII character
  // each mask is as wide as needleLength
  const size_t MaxBitWidth  = 8 * sizeof(int) - 1;
  // only if needleLength bits fit into an integer (minus 1), the algorithm will be fast
  if (needleLength > MaxBitWidth)
    return searchNative(haystack, haystackLength, needle, needleLength);

  // one mask per allowed character (1 byte => 2^8 => 256)
  // where all bits are set except those where the character is found in needle
  const size_t AlphabetSize = 256;
  unsigned int masks[AlphabetSize];
  size_t i;
  for (i = 0; i < AlphabetSize; i++)
    masks[i] = ~0;
  for (i = 0; i < needleLength; i++)
    masks[(unsigned char)needle[i]] &= ~(1 << i);

  // points beyond last considered byte
  const char* haystackEnd = haystack + haystackLength;

  // initial state mask has all bits set except the lowest one
  unsigned int state = ~1;
  const unsigned int FullMatch = 1 << needleLength;
  while (haystack != haystackEnd)
  {
    // update the bit array
    state  |= masks[(unsigned char)*haystack];
    state <<= 1;

    // if an unset bit "bubbled up" we have a match
    if ((state & FullMatch) == 0)
      return (haystack - needleLength) + 1;

    haystack++;
  }

  // needle not found in haystack
  return NULL;
}


// //////////////////////////////////////////////////////////


/// Rabin-Karp algorithm
/** based on simple hash proposed by Raphael Javaux **/
const char* searchRabinKarpString(const char* haystack, const char* needle)
{
  // detect invalid input
  if (!haystack || !needle)
    return NULL;

  // empty needle matches everything
  if (!*needle)
    return haystack;

  // find first match of the first letter
  haystack = strchr(haystack, *needle);
  if (!haystack)
    return NULL;

  // now first letter of haystack and needle is identical
  // let's compute the sum of all characters of needle
  unsigned int hashNeedle   = *needle;
  unsigned int hashHaystack = *haystack;
  const char*  scanNeedle   = needle   + 1;
  const char*  scanHaystack = haystack + 1;
  while (*scanNeedle && *scanHaystack)
  {
    hashNeedle   += *scanNeedle++;
    hashHaystack += *scanHaystack++;
  }

  // if scanNeedle doesn't point to zero, then we have too little haystack
  if (*scanNeedle)
    return NULL;

  // length of needle
  const size_t needleLength = scanNeedle - needle;

  // walk through haystack and roll the hash
  for (;;)
  {
    // identical hash ?
    if (hashHaystack == hashNeedle)
    {
      // can be a false positive, therefore must check all characters again
      if (memcmp(haystack, needle, needleLength) == 0)
        return haystack;
    }

    // no more bytes left ?
    if (!*scanHaystack)
      break;

    // update hash
    hashHaystack -= *haystack++;
    hashHaystack += *scanHaystack++;
  }

  // needle not found in haystack
  return NULL;
}


/// Rabin-Karp algorithm
/** based on simple hash proposed by Raphael Javaux **/
const char* searchRabinKarp(const char* haystack, size_t haystackLength,
                            const char* needle,   size_t needleLength)
{
  // detect invalid input
  if (!haystack || !needle || haystackLength < needleLength)
    return NULL;

  // empty needle matches everything
  if (needleLength == 0)
    return haystack;

  // one byte beyond last position where a match can begin
  const char* haystackEnd = haystack + haystackLength - needleLength + 1;

  // find first match of the first letter
  haystack = (const char*)memchr(haystack, *needle, haystackLength);
  if (!haystack)
    return NULL;

  // now first letter of haystack and needle is identical
  // let's compute the sum of all characters of needle
  unsigned int hashNeedle   = 0;
  unsigned int hashHaystack = 0;
  size_t i;
  for (i = 0; i < needleLength; i++)
  {
    // not enough haystack left ?
    if (!haystack[i])
      return NULL;

    hashNeedle   += needle  [i];
    hashHaystack += haystack[i];
  }

  // walk through haystack and roll the hash computation
  while (haystack != haystackEnd)
  {
    // identical hash ?
    if (hashHaystack == hashNeedle)
    {
      // can be a false positive, therefore must check all characters again
      if (memcmp(haystack, needle, needleLength) == 0)
        return haystack;
    }

    // update hash
    hashHaystack += *(haystack + needleLength);
    hashHaystack -= *haystack++;
  }

  // needle not found in haystack
  return NULL;
}


// //////////////////////////////////////////////////////////


/// super-fast for short strings (less than about 8 bytes), else use searchSimple or searchBoyerMooreHorspool
const char* searchNative(const char* haystack, size_t haystackLength,
                         const char* needle,   size_t needleLength)
{
  // uses memchr() for the first byte, then memcmp to verify it's a valid match

  // detect invalid input
  if (!haystack || !needle || haystackLength < needleLength)
    return NULL;

  // empty needle matches everything
  if (needleLength == 0)
    return haystack;

  // shorter code for just one character
  if (needleLength == 1)
    return (const char*)memchr(haystack, *needle, haystackLength);

  haystackLength -= needleLength - 1;
  // points beyond last considered byte
  const char* haystackEnd = haystack + haystackLength;

  // look for first byte
  while ((haystack = (const char*)memchr(haystack, *needle, haystackLength)) != NULL)
  {
    // does last byte match, too ?
    if (haystack[needleLength - 1] == needle[needleLength - 1])
      // okay, perform full comparison, skip first and last byte (if just 2 bytes => already finished)
      if (needleLength == 2 || memcmp(haystack + 1, needle + 1, needleLength - 2) == 0)
        return haystack;

    // compute number of remaining bytes
    haystackLength = haystackEnd - haystack;
    if (haystackLength == 0)
      return NULL;

    // keep going
    haystack++;
    haystackLength--;
  }

  // needle not found in haystack
  return NULL;
}
