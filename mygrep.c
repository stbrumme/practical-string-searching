// //////////////////////////////////////////////////////////
// mygrep.c
// Copyright (c) 2014,2019 Stephan Brumme. All rights reserved.
// see http://create.stephan-brumme.com/disclaimer.html
//

// gcc -O3 -std=c99 -Wall -pedantic search.c mygrep.c -o mygrep
// file size limited to available memory size because whole file is loaded into RAM

// enable GNU extensions, such as memmem()
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifdef _MSC_VER
/// GNU C++ has a super-optimized version of memmem() which Visual C++ lacks, here is a simple replacement using a function reference
#define memmem searchNative
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "search.h"

#include <string.h> // memmem()
#include <stdio.h>  // printf()
#include <stdlib.h> // malloc()


enum Algorithm
{
  UseBest
  , UseStrStr
  , UseMemMem
  , UseSimple
  , UseNative
  , UseKnuthMorrisPratt
  , UseBoyerMooreHorspool
  , UseBitap
  , UseRabinKarp
} algorithm;

enum
{
  ShowLines     = 0,
  ShowCountOnly = 1
} display;

int main(int argc, char* argv[])
{
  const char* syntax = "Syntax: ./mygrep searchphrase filename [--native|--memmem|--strstr|--simple|--knuthmorrispratt|--boyermoorehorspool|--bitap|--rabinkarp] [-c]\n";
  if (argc < 3 || argc > 5)
  {
    printf("%s", syntax);
    return -1;
  }

  // don't show lines, just count them
  display = ShowLines;
  if (argc == 5 && strcmp(argv[4], "-c")  == 0)
    display = ShowCountOnly;

  // use safer memmem() by default
  algorithm = UseBest;
  if (argc >= 4)
  {
    if      (strcmp(argv[3], "--native") == 0)
      algorithm = UseNative;
    else if (strcmp(argv[3], "--memmem") == 0)
      algorithm = UseMemMem;
    else if (strcmp(argv[3], "--strstr") == 0) // be careful: buffer overruns possible !!!
      algorithm = UseStrStr;
    else if (strcmp(argv[3], "--simple") == 0)
      algorithm = UseSimple;
    else if (strcmp(argv[3], "--knuthmorrispratt")   == 0 ||
             strcmp(argv[3], "--kmp")    == 0)
      algorithm = UseKnuthMorrisPratt;
    else if (strcmp(argv[3], "--boyermoorehorspool") == 0 ||
             strcmp(argv[3], "--bmh")    == 0)
      algorithm = UseBoyerMooreHorspool;
    else if (strcmp(argv[3], "--bitap")  == 0)
      algorithm = UseBitap;
    else if (strcmp(argv[3], "--rabinkarp") == 0)
      algorithm = UseRabinKarp;
    else if (strcmp(argv[3], "-c")       == 0)
      display = ShowCountOnly;
    else
    {
      printf("%s", syntax);
      return -2;
    }
  }

  // open file
  FILE* file = fopen(argv[2], "rb");
  if (!file)
  {
    printf("Failed to open file\n");
    return -3;
  }

  // determine its filesize
  fseek(file, 0, SEEK_END);
  long filesize = ftell(file);
  fseek(file, 0, SEEK_SET);
  if (filesize == 0)
  {
    printf("Empty file\n");
    return -4;
  }

  // allocate memory and read the whole file at once
  char* data = (char*) malloc(filesize + 2);
  if (!data)
  {
    printf("Out of memory\n");
    return -5;
  }
  fread(data, filesize, 1, file);
  fclose(file);

  // pad data to avoid buffer overruns
  data[filesize    ] = '\n';
  data[filesize + 1] = 0;

  // what  we look for
  const char*  needle         = argv[1];
  const size_t needleLength   = strlen(needle);
  // where we look for
  const char*  haystack       = data;
  const size_t haystackLength = filesize;

  // fence
  const char*  haystackEnd    = haystack + haystackLength;

  // "native" and "Boyer-Moore-Horspool" are in almost all cases the best choice
  if (algorithm == UseBest)
  {
    // when needle is longer than about 16 bytes, Boyer-Moore-Horspool is faster
    if (needleLength <= 16)
      algorithm = UseNative;
    else
      algorithm = UseBoyerMooreHorspool;
  }

  // search until done ...
  unsigned int numHits = 0;
  const char* current = haystack;
  for (;;)
  {
    // offset of current hit from the beginning of the haystack
    size_t bytesDone = current - haystack;
    size_t bytesLeft = haystackLength - bytesDone;

    switch (algorithm)
    {
    case UseMemMem:
      // correctly handled zeros, unfortunately much slower
      {
        const char* before = current;
        current = (const char*)memmem   (current, bytesLeft, needle, needleLength);
        // workaround for strange GCC behavior, else I get a memory access violation
        if (current)
        {
          int diff = current - before;
          current = before + diff;
        }
      }
      break;
    case UseStrStr:
      // much faster but has problems when bytes in haystack are zero,
      // requires both to be properly zero-terminated
      current = strstr                  (current,            needle);
      break;

    case UseSimple:
      // brute-force
      current = searchSimple            (current, bytesLeft, needle, needleLength);
      break;
    case UseNative:
      // brute-force for short needles, based on compiler-optimized functions
      current = searchNative            (current, bytesLeft, needle, needleLength);
      break;
    case UseKnuthMorrisPratt:
      // Knuth-Morris-Pratt
      current = searchKnuthMorrisPratt  (current, bytesLeft, needle, needleLength);
      break;
    case UseBoyerMooreHorspool:
      // Boyer-Moore-Horspool
      current = searchBoyerMooreHorspool(current, bytesLeft, needle, needleLength);
      break;
    case UseBitap:
      // Bitap / Baeza-Yates-Gonnet algorithm
      current = searchBitap             (current, bytesLeft, needle, needleLength);
      break;
    case UseRabinKarp:
      // Rabin-Karp algorithm
      current = searchRabinKarp         (current, bytesLeft, needle, needleLength);
      break;

    default:
      printf("Unknown search algorithm\n");
      return -6;
    }

    // needle not found in the remaining haystack
    if (!current)
      break;

    numHits++;

    // find end of line
    const char* right = current;
    while (right != haystackEnd && *right != '\n')
      right++;

    if (display == ShowCountOnly)
    {
      current = right;
      continue;
    }

    // find beginning of line
    const char* left = current;
    while (left != haystack && *left != '\n')
      left--;
    if (*left == '\n' && left != haystackEnd)
      left++;

    // send line to standard output
    size_t lineLength = right - left;
    fwrite(left, lineLength, 1, stdout);
    // and append a newline
    putchar('\n');

    // don't search this line anymore
    current = right;
  }

  if (display == ShowCountOnly)
    printf("%d\n", numHits);

  // exit with error code 1 if nothing found
  return numHits == 0 ? 1 : 0;
}
