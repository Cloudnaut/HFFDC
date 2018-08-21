# HFFDC
Huge f\*cking file delta copy - Backup Tool

A program which copies really big files without consuming the whole RAM over a really slow connection. It supports delta (also called differential) copy of really huge files by utilizing checksums (Tested with files > 2 TB) to update your backups. 

### Isn't rsync already doing this job?
For differential backups to an external drive, rsync always needs to read the whole file from the drive to calculate the checksums for the blocks. This isn't practical at all. Files on an backup drive normally don't change out of nowhere. That's why it's enough to store the checksums alongside the actual file. That's how HFFDC works.

## Main Use Case
You have really big files, e. g. VM disk images, database dumps, huge log files or big encrypted file container. You store backups of these files on an external medium, e. g. usb flash stick, external hard drive, network share, DVD or NAS. The files you're working with changed only partially. Why reuploading all your files byte by byte? Copy only changed parts of your files using the differential copy algorithm of HFFDC.

## Command Line Interface (CLI)
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

## Notes
How big should a block be?
Preferably the upstream transfer speed of the connection.

Required RAM for Copy operation: 2 times the specified BlockSizeInBytes.

Required RAM for checksum operation: Specified BlockSizeInBytes.

Because of lazy coding - Make sure the target file exists already and has exactly the same size as the source file.
Why? Because I just needed backups of my fixed size container files. Should be improved. Placeholder functions already implemented.


## Examples
```
Example 1: (Sync changes of a fixed size image to the backup on your USB drive)

HFFDC.exe -gc big.img 20971520 //Create checksum file for your big image file with blocksize 20MB
HFFDC.exe -chksmcpy big.img "Z:\Backups\big.img" //Copy only differences. Assuming the target file has been checksumed once
//Now copy "big.img.hffdc" next to "Z:\Backups\big.img" since they should be equal

```
