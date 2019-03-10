Flag: CTF{parse_order_matters}

This is a few different vulnerabilities:

* Padding oracle to decrypt/encrypt the payload

* Hash extension to add a new "UID 0" line

* Parsing "bug" so if you have multiple UID lines, the last one is read

Solution.rb (in this directory) implements all of that, including a call out to
hash_extender (which I've included).
