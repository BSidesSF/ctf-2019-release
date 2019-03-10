Flag: CTF{is_fun!!uffling_block_sh}

This is an ECB block shuffle vulnerability.

The trick is to set your first name to:

```
t:1}             est
```

And your last name `AA` (or anything 2 characters). That creates the hash:

```
{"first_name":"t:1}             est","last_name":"AA","is_admin":0}
```

In blocks:

```
{"first_name":"t
:1}             
est","last_name"
:"AA","is_admin"
:0}             
```

That creates an encrypted cookie that'll look something like (it'll change based
on your key):

```
6e892b6edbe4cef806c54a9ebb7404bf90f1ac11bbb379de9a21e6fefa7f2c0f27a5bb5f139ee6abdb09939e40f5f9f6da0b207c94fd83c5ba4fe0967d373f1976ec6ac003a2cb1fdc672b824473fef8
```

Or in blocks:

```
6e892b6edbe4cef806c54a9ebb7404bf # {"first_name":"t
90f1ac11bbb379de9a21e6fefa7f2c0f # :1}             
27a5bb5f139ee6abdb09939e40f5f9f6 # est","last_name"
da0b207c94fd83c5ba4fe0967d373f19 # :"AA","is_admin"
76ec6ac003a2cb1fdc672b824473fef8 # :0}
```

Then we replace the last block with the second:

```
6e892b6edbe4cef806c54a9ebb7404bf # {"first_name":"t
90f1ac11bbb379de9a21e6fefa7f2c0f # :1}             
27a5bb5f139ee6abdb09939e40f5f9f6 # est","last_name"
da0b207c94fd83c5ba4fe0967d373f19 # :"AA","is_admin"
90f1ac11bbb379de9a21e6fefa7f2c0f # :1}             
```

Then put them back together:

```
6e892b6edbe4cef806c54a9ebb7404bf90f1ac11bbb379de9a21e6fefa7f2c0f27a5bb5f139ee6abdb09939e40f5f9f6da0b207c94fd83c5ba4fe0967d373f1990f1ac11bbb379de9a21e6fefa7f2c0f
```

Replace your cookie with that value, and you're good to go!
