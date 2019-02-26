// eFile.c
// Runs on either TM4C123 or MSP432
// High-level implementation of the file system implementation.
// Daniel and Jonathan Valvano
// August 29, 2016
#include <stdint.h>
#include "eDisk.h"

uint8_t Buff[512]; // temporary buffer used during file I/O
uint8_t Directory[256], FAT[256];
int32_t bDirectoryLoaded = 0; // 0 means disk on ROM is complete, 1 means RAM version active

// Return the larger of two integers.
static int16_t max(int16_t a, int16_t b){
  if(a > b){
    return a;
  }
  return b;
}


// if directory and FAT are not loaded in RAM,
// bring it into RAM from disk
// if bDirectoryLoaded is 0, 
//    read disk sector 255 and populate Directory and FAT
//    set bDirectoryLoaded = 1
// if bDirectoryLoaded is 1, simply return
static void MountDirectory(void){
	uint8_t sector_num = 255;
	if ( bDirectoryLoaded == 0 ) {
		if ( eDisk_ReadSector(Buff, sector_num) != RES_OK )	  // Error occured
			return;
		else {			 // Successful copying
			FAT[] = 
			Directory[] = 
			bDirectoryLoaded = 1;
		}
	}
	else
		return;
}

// Return the index of the last sector in the file
// associated with a given starting sector.
// Note: This function will loop forever without returning
// if the file has no end (i.e. the FAT is corrupted).
static uint8_t LastSector(uint8_t start){
	uint8_t m;
	if ( start == 255 )
		return 255;
	else {
		m = FAT[start];
		while ( m != 255 ) {
			start = m;
			m = FAT[start];
		}
	}
	return start;
}

// Return the index of the first free sector.
// Note: This function will loop forever without returning
// if a file has no end or if (Directory[255] != 255)
// (i.e. the FAT is corrupted).
static uint8_t FindFreeSector(void){
	uint8_t free_sector = -1;
	uint8_t last_sector;
	uint8_t i = 0;
	last_sector = LastSector(Directory[i]);
	while ( last_sector != 255 ) {
		free_sector = max(free_sector, last_sector);
		i++;
		last_sector = LastSector(Directory[i]);
	}
    return free_sector++; 
}

// Append a sector index 'n' at the end of file 'num'.
// This helper function is part of OS_File_Append(), which
// should have already verified that there is free space,
// so it always returns 0 (successful).
// Note: This function will loop forever without returning
// if the file has no end (i.e. the FAT is corrupted).
static uint8_t AppendFAT(uint8_t num, uint8_t n){
	uint8_t i, m;
	i = Directory[num];
	if ( i == 255 ) 			// Found available place.
		Directory[num] = n;		// Put sector number n to the directory indexed num.
	else {
		while ( i != 255 ) {
			m = FAT[i];
			if ( m == 255 ) {
				FAT[i] = n;
				return 0;
			}
			i = m;
		}
	}
	return 0;
}

// *********** OS_File_New *************
// Returns a file number of a new file for writing
// Inputs: none
// Outputs: number of a new file
// Errors: return 255 on failure or disk full
uint8_t OS_File_New(void){
	uint8_t i = 0;
	MountDirectory();				 // Bring disk into RAM if it is not.
	while ( i <= 255 ) {
		if ( Directory[i] == 255 )	 // Free file to append into.
			return i;				 // Return the index of that file.
		else     					 // Continue searching if not.
			i++;
	}
    return 255;	                    // Finished searching with no free file.
}

// *********** OS_File_Size *************
// Check the size of this file
// Inputs:  num, 8-bit file number, 0 to 254
// Outputs: 0 if empty, otherwise the number of sectors
// Errors:  none
uint8_t OS_File_Size(uint8_t num){
	uint8_t FATindex, sectors; 
	sectors = 0;
	FATindex = Directory[num];
	if ( FATindex == 255 )		// End of FAT, or empty
		return 0;
	while ( FAT[FATindex] != 255 ) {
		FATindex = FAT[FATindex];
		sectors++;
	}
    return (sectors + 1); 
}

// *********** OS_File_Append *************
// Save 512 bytes into the file
// Inputs:  num, 8-bit file number, 0 to 254
//          buf, pointer to 512 bytes of data
// Outputs: 0 if successful
// Errors:  255 on failure or disk full
uint8_t OS_File_Append(uint8_t num, uint8_t buf[512]){
	MountDirectory();
	uint8_t sector_index;
	sector_index = FindFreeSector();
	if ( sector_index == 255 )
		return 255;				 // Full buffer
	else {
		if ( eDisk_WriteSector(buf, sector_index) != RES_OK )
			return 255;			 // Error
		AppendFAT(num, sector_index);
	}
    return 0;       // Successful appending
}

// *********** OS_File_Read *************
// Read 512 bytes from the file
// Inputs:  num, 8-bit file number, 0 to 254
//          location, logical address, 0 to 254
//          buf, pointer to 512 empty spaces in RAM
// Outputs: 0 if successful
// Errors:  255 on failure because no data
uint8_t OS_File_Read(uint8_t num, uint8_t location,
                     uint8_t buf[512]){
	uint8_t FATindex, i;
	FATindex = Directory[num];
	if ( FATindex == 255 )
		return 255;
	for ( i = 0; i <= location; i++ ) {		 // Search for  that location in FAT
		FATindex = FAT[FATindex];
		if ( FATindex == 255 )
			return 255;
	}
	return (eDisk_ReadSector(buf, FATindex));
}

// ************ OS_File_Flush *************
// Update working buffers onto the disk
// Power can be removed after calling flush
// Inputs:  none
// Outputs: 0 if success
// Errors:  255 on disk write failure
uint8_t OS_File_Flush(void){
	uint8_t sector, start;
	start = Directory[254];
	sector = FAT[start];
	if ( bDirectoryLoaded ) {
		if ( eDisk_WriteSector(Buff, sector) != RES_OK )
			return 255;
	}
    return 0; 
}

// *********** OS_File_Format *************
// Erase all files and all data
// Inputs:  none
// Outputs: 0 if success
// Errors:  255 on disk write failure
uint8_t OS_File_Format(void){
	if ( eDisk_Format() != RES_OK )    // call eDiskFormat
		return 255;
	bDirectoryLoaded = 0;              // clear bDirectoryLoaded to zero
    return 0; 
}
