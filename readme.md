This is a mirror of my library hosted at https://create.stephan-brumme.com/practical-string-searching/

# Practical String Searching

There is a huge number of scientific papers on fast, faster and super-fast string searching algorithms.
They usually prove theoretical performance in [O-Notation](https://en.wikipedia.org/wiki/O_notation) and most papers cover memory consumption as well.

However, theoretical performance isn't always the same as practical performance.
That's why I always want to measure real-world throughput: this article presents hopefully understandable C implementations of the most common generic string search algorithms.

In addition I also wrote a simple tool called `mygrep` that prints all lines of a file where a search phrase is found.
It doesn't come with all the bells and whistles of the Unix tool grep but achieves similar or sometimes even better speed.

## Algorithms
- simple loop / brute force
- `memchr`/`memcmp`
- `memmem`
- `strstr`
- [Knuth-Morris-Pratt](https://en.wikipedia.org/wiki/Knuth-Morris-Pratt_algorithm)
- [Boyer-Moore-Horspool](https://en.wikipedia.org/wiki/Boyer%E2%80%93Moore_string_search_algorithm)
- [Bitap aka Baeza-Yates-Gonnet](https://en.wikipedia.org/wiki/Bitap_algorithm)
- [Rabin-Karp](https://en.wikipedia.org/wiki/Rabin-Karp_algorithm)

## Interface
All C functions share the same interface:
`const char* search(const char* haystack,                        const char* needle);                     ` for strings
`const char* search(const char* haystack, size_t haystackLength, const char* needle, size_t needleLength);` for binary data

## More ...
See my website https://create.stephan-brumme.com/practical-string-searching/ for a live demo, code examples and benchmarks.
