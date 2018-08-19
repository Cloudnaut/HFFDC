# HFFDC
Huge f\*cking file delta copy

A program which copies huge f\*ucking files without consuming the whole RAM over a really limited connection. It supports delta copy of really huge files by utilizing checksums (Tested with files >2 TB). 

Designed to improve the experience of regular backups. Further improvments are planed but not implemented yet.

Required RAM for Copy operation: 2 times the specified BlockSizeInBytes.

Required RAM for checksum operation: Specified BlockSizeInBytes.

How big should a block be?
Preferably the transfer speed of the connection.

```
Usage:
HFFDC.exe OPTION [PARAMETERS]

Options:
-gc, --generate-checksum
  [PARAMETERS: filePath blockSizeInBytes]
  Generates a checksum file for the specified file
  
-compcpy, --compare-and-copy
  [PARAMETERS: sourceFilePath targetFilePath blockSizeInBytes]
  Reads each block of the source and the targetfile, only differing blocks will be copied to destination

-chksmcpy, --checksum-copy
  [PARAMETERS: sourceFilePath targetFilePath]
  Reads the checksum files of each block, only blocks with differing checksums will be copied to destination
  Note: Requires source and target files to be checksumed with same blocksize

```

Note:
Because of lazy coding - Make sure the target file exists already and has exactly the same size as the source file.
Why? Because I just needed backups of my fixed size container files. Should be improved. Placeholder functions already implemented.

```
Example: (Sync changes of a fixed size image to the backup on your USB drive)

HFFDC.exe -gc big.img 20971520 //Create checksum file for your big image file with blocksize 20MB
HFFDC.exe -chksmcpy big.img "Z:\Backups\big.img" //Copy only differences. Assuming the target file has been checksumed once
//Now copy "big.img.hffdc" next to "Z:\Backups\big.img" since they should be equal

```
