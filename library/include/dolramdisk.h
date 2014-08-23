/*****************************************************************************
 * libdolramdisk
 * Copyright (c) 2012, 2014 carstene1ns <dev f4ke de>
 *
 * dolramdisk.h - Header file
 ****************************************************************************/

#ifndef DOLRAMDISK_H
#define DOLRAMDISK_H
 
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <gctypes.h>
#include <gccore.h>
#include <ogc/disc_io.h>
#include <sys/iosupport.h>

/* ramdisk structure, will be created by mkdolramdisk */

typedef struct
{
  const char          name[23];       /* name of the ramdisk */
  const unsigned int  numfiles;       /* count of files */
  const char          **filenames;    /* list with names */
  const size_t        *filesizes;     /* list with sizes */
  const unsigned char **filecontents; /* list with content */
} dolramdisk;

/* function prototypes */

int dolramdiskInit(dolramdisk*);
int dolramdiskInitDefault(dolramdisk*);

#ifdef __cplusplus
}
#endif

#endif /* DOLRAMDISK_H */
