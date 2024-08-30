# touch

The Unix `touch` command for the byte-critical.

GNU - 76000 bytes

this - 6972 bytes

~1090.07% smaller than GNU.

## build

`tcc touch.c -Oz -flto -fwhole-program -ffunction-sections -s`

gcc also works, but will generate a binary about twice as large.

