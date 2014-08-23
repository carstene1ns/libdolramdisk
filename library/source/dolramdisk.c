/*****************************************************************************
 * libdolramdisk
 * Copyright (c) 2012, 2014 carstene1ns <dev f4ke de>
 *
 * dolramdisk.c - Source file
 ****************************************************************************/

#include <dolramdisk.h>

/* debug function that can be overwritten for redirection (file/net/etc.) */
#ifdef DRD_DEBUG
  #define drd_printf printf
#else
  /* dummy function, will be optimized away */
  void drd_printf(char *format, ...) { return; }
#endif

/* function prototypes, all are internal
   and will be available by the devoptab entries */

/* supported functions */

int _DRD_open_r (struct _reent *r, void *fileStruct, const char *path, int flags, int mode);
int _DRD_close_r (struct _reent *r, int fd);
ssize_t _DRD_read_r (struct _reent *r, int fd, char *ptr, size_t len);
off_t _DRD_seek_r (struct _reent *r, int fd, off_t pos, int dir);
int _DRD_fstat_r (struct _reent *r, int fd, struct stat *st);
int _DRD_stat_r (struct _reent *r, const char *path, struct stat *st);

#ifdef DRD_DEBUG
/* verbose unsupported functions */

ssize_t _DRD_write_r (struct _reent *r, int fd, const char *ptr, size_t len);
int _DRD_link_r (struct _reent *r, const char *existing, const char *newLink);
int _DRD_unlink_r (struct _reent *r, const char *name);
int _DRD_chdir_r (struct _reent *r, const char *name);
int _DRD_rename_r (struct _reent *r, const char *oldName, const char *newName);
int _DRD_mkdir_r (struct _reent *r, const char *path, int mode);
DIR_ITER* _DRD_diropen_r(struct _reent *r, DIR_ITER *dirState, const char *path);
int _DRD_dirreset_r (struct _reent *r, DIR_ITER *dirState);
int _DRD_dirnext_r (struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat);
int _DRD_dirclose_r (struct _reent *r, DIR_ITER *dirState);
int _DRD_statvfs_r (struct _reent *r, const char *path, struct statvfs *buf);
int _DRD_ftruncate_r (struct _reent *r, int fd, off_t len);
int _DRD_fsync_r (struct _reent *r, int fd);
#endif /* DRD_DEBUG */

/* file entry structure */

typedef struct
{
  dolramdisk *filesystem; /* points to the filesystem of the ramdisk*/
  unsigned int index;     /* index of the file in the ramdisk */
  size_t offset;          /* offset inside the file */
} FILE_STRUCT;

/* empty directory entry structure, directories are unsupported currently */

typedef struct
{
  unsigned int dummy;
} DIR_STATE_STRUCT;

/* devoptab providing standard file functions */

#ifdef DRD_DEBUG
  /* include verbose functions */

  static const devoptab_t dotab_drd =
  {
    "dolramdisk",
    sizeof (FILE_STRUCT),
    _DRD_open_r,
    _DRD_close_r,
    _DRD_write_r,     /* unsupported */
    _DRD_read_r,
    _DRD_seek_r,
    _DRD_fstat_r,
    _DRD_stat_r,
    _DRD_link_r,      /* unsupported */
    _DRD_unlink_r,    /* unsupported */
    _DRD_chdir_r,     /* unsupported */
    _DRD_rename_r,    /* unsupported */
    _DRD_mkdir_r,     /* unsupported */
    sizeof (DIR_STATE_STRUCT),
    _DRD_diropen_r,   /* unsupported */
    _DRD_dirreset_r,  /* unsupported */
    _DRD_dirnext_r,   /* unsupported */
    _DRD_dirclose_r,  /* unsupported */
    _DRD_statvfs_r,   /* unsupported */
    _DRD_ftruncate_r, /* unsupported */
    _DRD_fsync_r,     /* unsupported */
    NULL,             /* device data, will be filled with ramdisk structure */
    NULL,
    NULL
  };
#else
  static const devoptab_t dotab_drd = {
    "dolramdisk",
    sizeof (FILE_STRUCT),
    _DRD_open_r,
    _DRD_close_r,
    NULL,
    _DRD_read_r,
    _DRD_seek_r,
    _DRD_fstat_r,
    _DRD_stat_r,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    sizeof (DIR_STATE_STRUCT),
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL, /* device data, will be filled with ramdisk structure */
    NULL,
    NULL
  };
#endif /* DRD_DEBUG */

int _DRD_open_r (struct _reent *r, void *fileStruct, const char *path, int flags, int mode)
{
  FILE_STRUCT* fp = (FILE_STRUCT*) fileStruct;
  const devoptab_t *devops;
  unsigned int i;

  drd_printf("trying to open('%s').\n", path);

  /* select ramdisk by specified path */
  devops = GetDeviceOpTab(path);

  if (!devops) {
    /* not found */
    drd_printf("open('%s') failed. No ramdisk.\n", path);
    return -1;
  }

  /* file opened for writing only */
  if((flags & 0x03) == O_WRONLY) {
    drd_printf("open('%s') for writing failed. Readonly ramdisk.\n", path);
    return -1;
  }

  /* move the path pointer to the start of the actual path */
  if (strchr (path, ':') != NULL) {
    path = strchr (path, ':') + 1;
  }

  /* abort if path contains more than one ":" */
  if (strchr (path, ':') != NULL) {
    drd_printf("open('%s') failed. Only one ':' allowed in path.\n", path);
    return -1;
  }

  dolramdisk* this_ramdisk = devops->deviceData;

  /* loop through files, to find file by name */
  for (i = 0; i < this_ramdisk->numfiles; i++)
  {
    /* if file found */
    if (stricmp(this_ramdisk->filenames[i], path) == 0)
    {
      drd_printf("open('%s') succeeded.\n", path);
      fp->filesystem = this_ramdisk;
      fp->index = i;
      fp->offset = 0; /* begin of file */
      /* ok */
      return (int) fp;
    }
  }

  /* file not found */
  drd_printf("open('%s') failed. File not found.\n", path);
  return -1;
}

int _DRD_close_r (struct _reent *r, int fd)
{
  FILE_STRUCT* fp = (FILE_STRUCT*) fd;

  if (fp != NULL)
  {
    drd_printf("close('%s').\n", fp->filesystem->filenames[fp->index]);
    /* remove allocation */
    fp->filesystem = NULL;

    return 0;
  }

  drd_printf("attempt to close() NULL file pointer.\n");
  /* we do not care, so nothing to do */
  return 0;
}

ssize_t _DRD_read_r (struct _reent *r, int fd, char *ptr, size_t len)
{
  FILE_STRUCT* fp = (FILE_STRUCT*) fd;
  ssize_t read_len = 0;

  drd_printf("read('%s') asking for %d bytes\n", fp->filesystem->filenames[fp->index], len);

  if (fp != NULL)
  {
    /* if file not closed */
    if (fp->filesystem != NULL)
    {
      drd_printf("read() offset was %lu\n", (unsigned long) fp->offset);
      read_len = len;

      /* don't want to read past the end of file */
      if (fp->offset + len > fp->filesystem->filesizes[fp->index])
      {
        read_len = fp->filesystem->filesizes[fp->index] - fp->offset;
        drd_printf("attempt to read() after end of file (filesize is %lu). set read_len to %d.\n",
                   (unsigned long) fp->filesystem->filesizes[fp->index], read_len);
      }

      /* copy the filecontent to the buffer */
      memcpy(ptr, fp->filesystem->filecontents[fp->index] + fp->offset, read_len);

      fp->offset += read_len;
    }
  }

  drd_printf("read() returning %d bytes.\n", read_len);
  return read_len;
}

off_t _DRD_seek_r (struct _reent *r, int fd, off_t pos, int dir)
{
  FILE_STRUCT* fp = (FILE_STRUCT*) fd;
  off_t off = -1;

  if (fp != NULL)
  {
    /* if file not closed */
    if (fp->filesystem != NULL)
    {
      /* check the direction */
      switch(dir) {
        case SEEK_SET:
          /* start at beginning of file */
          off = pos;
          break;
        case SEEK_END:
          /* start at end of file, going backwards */
          off = fp->filesystem->filesizes[fp->index] - pos;
          break;
        case SEEK_CUR:
          /* start at current file position */
          off = fp->offset + pos;
          break;
        default:
          drd_printf("seek() with undefined direction.\n");
          break;
      }

      /* check if over file beginning or end */
      if (off < 0)
      {
        drd_printf("seek() overflow at beginning of file (%lu bytes).\n", (unsigned long) off);
        off = 0;
      }
      if (off > fp->filesystem->filesizes[fp->index])
      {
        drd_printf("seek() overflow at end of file (%lu bytes).\n",
                   (unsigned long) off - fp->filesystem->filesizes[fp->index]);
        off = fp->filesystem->filesizes[fp->index];
      }

      drd_printf("seek() setting offset from %lu to %lu.\n", (unsigned long) fp->offset, (unsigned long) off);
      /* set offset */
      fp->offset = off;
    }
  }

  if (off == -1)
  {
    drd_printf("seek() returning -1. Not a valid file handle.\n");
  }
  return off;
}

int _DRD_fstat_r (struct _reent *r, int fd, struct stat *st)
{
  FILE_STRUCT* fp = (FILE_STRUCT*) fd;

  if (fp != NULL)
  {
    /* only filesize is important */
    st->st_dev = 0;
    st->st_ino = 0;
    st->st_mode = 0;
    st->st_nlink = 0;
    st->st_uid = 0;
    st->st_gid = 0;
    st->st_rdev = 0;
    st->st_size = fp->filesystem->filesizes[fp->index];

    drd_printf("stat('%s') succeeded, file size is %lu.\n",
               fp->filesystem->filenames[fp->index],
               (unsigned long) fp->filesystem->filesizes[fp->index]);
    return 0;
  }

  /* NULL file pointer */
  drd_printf("attempt to stat() NULL file pointer.\n");
  return -1;
}

int _DRD_stat_r (struct _reent *r, const char *path, struct stat *st)
{
  const devoptab_t *devops;
  unsigned int i;

  st->st_dev = 0;
  st->st_ino = 0;
  st->st_mode = 0;
  st->st_nlink = 0;
  st->st_uid = 0;
  st->st_gid = 0;
  st->st_rdev = 0;
  st->st_size = 0;

  drd_printf("trying to stat('%s').\n", path);

  /* select ramdisk by specified path */
  devops = GetDeviceOpTab(path);

  if (!devops) {
    /* not found */
    drd_printf("stat('%s') failed. No ramdisk.\n", path);
    return -1;
  }

  /* move the path pointer to the start of the actual path */
  if (strchr (path, ':') != NULL) {
    path = strchr (path, ':') + 1;
  }

  /* abort if path contains more than one ":" */
  if (strchr (path, ':') != NULL) {
    drd_printf("stat('%s') failed. Only one ':' allowed in path.\n", path);
    return -1;
  }

  dolramdisk* this_ramdisk = devops->deviceData;

  /* loop through files, to find file by name */
  for (i = 0; i < this_ramdisk->numfiles; i++)
  {
    /* if file found */
    if (stricmp(this_ramdisk->filenames[i], path) == 0)
    {
      st->st_size = this_ramdisk->filesizes[i];

      drd_printf("stat('%s') succeeded, file size is %lu.\n",
                 this_ramdisk->filenames[i],
                 (unsigned long) this_ramdisk->filesizes[i]);
      return 0;
    }
  }

  /* file not found */
  drd_printf("stat('%s') failed. File not found.\n", path);
  return -1;
}

#if DRD_DEBUG == 1
/* verbose unsupported functions */

ssize_t _DRD_write_r (struct _reent *r, int fd, const char *ptr, size_t len)
{
  drd_printf("write() is not supported.\n");
  return -1;
}

int _DRD_link_r (struct _reent *r, const char *existing, const char *newLink)
{
  drd_printf("link() is not supported (%s -> %s).\n", existing, newLink);
  return -1;
}

int _DRD_unlink_r (struct _reent *r, const char *name)
{
  drd_printf("unlink() is not supported (%s).\n", name);
  return -1;
}

int _DRD_chdir_r (struct _reent *r, const char *name)
{
  drd_printf("chdir() is not supported (%s).\n", name);
  return -1;
}

int _DRD_rename_r (struct _reent *r, const char *oldName, const char *newName)
{
  drd_printf("rename() is not supported (%s -> %s).\n", oldName, newName);
  return -1;
}

int _DRD_mkdir_r (struct _reent *r, const char *path, int mode)
{
  drd_printf("mkdir() is not supported (%s).\n", path);
  return -1;
}

DIR_ITER* _DRD_diropen_r(struct _reent *r, DIR_ITER *dirState, const char *path)
{
  drd_printf("diropen() is not supported (%s).\n", path);
  return NULL;
}

int _DRD_dirreset_r (struct _reent *r, DIR_ITER *dirState)
{
  drd_printf("dirreset() is not supported.\n");
  return -1;
}

int _DRD_dirnext_r (struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat)
{
  drd_printf("dirnext() is not supported.\n");
  return -1;
}

int _DRD_dirclose_r (struct _reent *r, DIR_ITER *dirState)
{
  drd_printf("dirclose() is not supported.\n");
  return -1;
}

int _DRD_statvfs_r (struct _reent *r, const char *path, struct statvfs *buf)
{
  drd_printf("statvfs() is not supported.\n");
  return -1;
}

int _DRD_ftruncate_r (struct _reent *r, int fd, off_t len)
{
  drd_printf("ftruncate() is not supported.\n");
  return -1;
}

int _DRD_fsync_r (struct _reent *r, int fd)
{
  drd_printf("fsync() is not supported.\n");
  return -1;
}
#endif /* DRD_DEBUG */

/* initialize a ramdisk */

int dolramdiskInit(dolramdisk* ramdisk)
{
  devoptab_t* devops;

  devops = malloc(sizeof(devoptab_t));
  if(!devops) {
    /* out of memory */
    return -1;
  }

  /* copy over the devoptab structure and set name */
  memcpy(devops, &dotab_drd, sizeof(dotab_drd));
  devops->name = ramdisk->name;
  devops->deviceData = ramdisk;

  /* add the ramdisk to the device list */
  AddDevice(devops);

  /* ok */
  return 0;
}
