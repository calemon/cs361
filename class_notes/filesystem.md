# Filesystem

The filesystem is virtualized to appear to be one big drive

- There are actually multiple devices that all have their own files, however the filesystem is virtualized so that they appear the same

## Opening a file

1) Check it
2) Add it to FDT (File Descriptor Table)

## Index Block Allocation

### Inode = Metadata
- Mode/Permissions
- Location of blocks
- Create/Modify dates

Can keep track of free blocks in many ways:
- Linear searching (very expensive, especially as system grows)
- Bitmaps
  - `int bitmap[10];`

## File Allocation Table (FAT)

There have been multiple versions (8, 12, 16, 32-bit, then an exFAT)

