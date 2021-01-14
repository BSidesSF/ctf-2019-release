A 100 byte XZ file has been converted into 800 binary bytes.

The bytes are then encoded as a high frequency for 1 and a slightly less high frequency for 0, ranging between around 17khz and 22khz.

These tones are then placed above another pieces of music.

To solve, apply a high pass filter to remove all the music.

Then convert to a spectrogram:

sox datagrab.wav -n spectrogram -x 6400 -y 640

From here, it is easy to crop and scale and resize and threshold until the image is a 800x1 pixel image of black and white pixels.

pngtopam data_img.png | dd skip=13 bs=1 > img_to_bin.bin

~/projects/misc_perl/file_to_hex.pl img_to_bin.bin | sed -r 's/ff/1/g' | sed -r 's/00/0/g' | perl -ne 'print pack("B*", $_);' > solve.bin

Solve.bin is the xz file.
