# HFFDC
Huge f\*cking file delta copy

A program which copies huge f\*ucking files without consuming the whole RAM. It supports delta copy of really huge files by utilizing checksums. 

Designed to improve the experience of regular backups. After writing a backup the program always assumes that the TargetFile matches its checksum file. Therefore stored backups must not be modified. SourceFile checksums are always being recomputet. Further improvments are planed but not implemented yet. Usage ist currently pretty ugly.

Required RAM for Copy operation: 2 times the specified BlockSizeInBytes

How big should a block be?
Preferably the transfer speed of the connection

```
Usage:

Standard Copy
HFFDC.exe SourceFilePath TargetFilePath BlockSizeInBytes

Checksum Copy (Requires Source and Target files to be checksumed with same blocksize)
HFFDC.exe SourceFilePath TargetFilePath 123 123

Generate Checksum File
HFFDC.exe SourceFilePath BlockSizeInBytes

```

Note:
CLI will most likely change because it isn't intutive at all.
Because of lazy coding - Make sure the target file exists already and has exactly the same size as the source file.
Why? Because I just needed backups of my fixed size container files. Should be improved. Placeholder functions already implemented.
