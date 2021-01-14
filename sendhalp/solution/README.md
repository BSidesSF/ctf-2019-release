Flag: CTF{i_can_windows}

To solve this problem, you have to either reverse engineer the encryption and
implement it (it's similar to RC4 with some small anti-reversing changes) or
directly call the function with the three correct arguments.

The version of libsendhalp.dll in this folder is modified to change the values.
Simply run the .exe file (also in this folder) with the encrypted file!

c:\...> sendhalp.exe ..\distfiles\flag.txt.enc out.txt
c:\...> type out.txt
Congratulations!

The flag is: CTF{i_can_windows}
