Flag: CTF{i_beat_the_impossible}

The validation is entirely client-side. You can bypass it by opening the
developer console and running:

```
$.post('http://localhost:1444/move', 'move=ul')
$.post('http://localhost:1444/move', 'move=c')
$.post('http://localhost:1444/move', 'move=br')
location.reload()
```
