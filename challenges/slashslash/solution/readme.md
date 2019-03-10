Zip file has a comment which is the passphrase for the encrpted file

$ unzip flag.zip
Archive:  flag.zip
SevenPinLock0123456


$ openssl enc -d -in flag.aes128cbc -out /dev/stdout -k SevenPinLock0123456 -aes-128-cbc
CTF{always_add_comments}

