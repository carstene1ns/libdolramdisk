/*****************************************************************************
 * libdolramdisk
 * Copyright (c) 2012, 2014 carstene1ns <dev f4ke de>
 *
 * mkdolramdisk.c - Source file
 ****************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>

int main (int argc, char *argv[])
{
  char filename[256], ramdiskname[23];
  FILE *file_ramdisk, *file_include;
  DIR *directory_include;
  struct dirent *directory_iterator;
  size_t size;
  unsigned int file_count = 0, i, j;

  /* show header */
  printf("mkdolramdisk version 0.1\n"
         "========================\n\n");

  /* show help if needed */
  if (argc != 3)
  {
    printf("Invalid number of arguments supplied.\n"
           "Usage: mkdolramdisk <directory> <output_prefix>\n\n");

    return -1;
  }

  /* open directory for reading */
  directory_include = opendir(argv[1]);
  if (directory_include == NULL)
  {
    printf("Error opening directory, error returned was %d (%s)\n", errno, strerror(errno));

    return errno;
  }

  /* name of the ramdisk, will be used in various places */
  strncpy(ramdiskname, argv[2], 23);

  /* create file header for ramdisk contents */
  sprintf(filename, "%s.h", ramdiskname);
  file_ramdisk = fopen(filename, "w");
  if (file_ramdisk == NULL)
  {
    printf("Error opening %s for writing, error returned was %d (%s)\n", filename, errno, strerror(errno));

    return errno;
  }

  /* dynamic arrays, yay =) */
  char **filenames = malloc(sizeof(char *));
  size_t *filesizes = malloc(sizeof(size_t *));
  unsigned char **filecontents = malloc(sizeof(unsigned char *));

  while ((directory_iterator = readdir(directory_include)) != NULL)
  {
    /* get the relative filename */
    sprintf(filename,"%s/%s", argv[1], directory_iterator->d_name);

    /* don't include special directories */
    if (!strcmp(directory_iterator->d_name, ".") || !strcmp(directory_iterator->d_name, ".."))
      continue;

    /* filter out directories for now */
    if(directory_iterator->d_type == DT_DIR)
    {
      printf("Found Directory \"%s\", inclusion not supported (yet).\n", filename);
      continue;
    }

    /* open file to include */
    file_include = fopen(filename, "rb");
    if (file_include != NULL)
    {
      if (file_count > 0) {
        /* resize the arrays by one */
        filenames = realloc(filenames, (file_count + 1) * sizeof(char *));
        filesizes = realloc(filesizes, (file_count + 1) * sizeof(size_t));
        filecontents = realloc(filecontents, (file_count + 1) * sizeof(unsigned char *));
      }

      /* get file name */
      filenames[file_count] = malloc((strlen(directory_iterator->d_name) + 1) * sizeof(char));
      strcpy(filenames[file_count], directory_iterator->d_name);
      printf("Found file \"%s\", ", directory_iterator->d_name);

      /* get file size */
      fseek(file_include, 0, SEEK_END);
      size = ftell(file_include);
      rewind(file_include);
      filesizes[file_count] = size;
      printf("size is %lu bytes.\n", (unsigned long) size);

      /* get file content */
      filecontents[file_count] = malloc(size);
      fread(filecontents[file_count], size, 1, file_include);
      fclose(file_include);

      file_count++;
    }
    else
    {
      printf("Error opening \"%s\" for reading, error returned was %d (%s)\n", filename, errno, strerror(errno));

      return errno;
    }
  }
  closedir(directory_include);

  if (file_count > 0)
  {
    printf ("\nAdding %i file(s)...", file_count);
    fflush(stdout);

    fprintf(file_ramdisk, "#include <dolramdisk.h>\n\n");

    /* insert filenames in ramdisk */
    fprintf(file_ramdisk, "const char *_%s_filenames[%d] = { ", ramdiskname, file_count);
    for (i = 0; i < file_count; i++)
    {
      fprintf(file_ramdisk, "\"/%s\"", filenames[i]);
      if (i < file_count - 1)
        fprintf(file_ramdisk, ", ");
      else
        fprintf(file_ramdisk, " };\n");
    }

    /* insert filesizes in ramdisk */
    fprintf(file_ramdisk, "const size_t _%s_filesizes[%d] = { ", ramdiskname, file_count);
    for (i = 0; i < file_count; i++)
    {
      fprintf(file_ramdisk, "%lu", (unsigned long) filesizes[i]);
      if (i < file_count - 1)
      {
        fprintf(file_ramdisk, ", ");
      }
      else
      {
        fprintf(file_ramdisk, " };\n");
      }
    }

    /* insert file contents in ramdisk */
    fprintf(file_ramdisk, "const unsigned char * _%s_filecontents[%d] = { ", ramdiskname, file_count);
    for (i = 0; i < file_count; i++)
    {
      fprintf(file_ramdisk, "(const unsigned char *) \"");
      for (j = 0; j < filesizes[i]; j++)
        fprintf(file_ramdisk,"\\x%02x", filecontents[i][j]);

      if (i < file_count - 1)
        fprintf(file_ramdisk, "\",\n");
      else
        fprintf(file_ramdisk, "\" };\n\n");
    }

    /* write the ramdisk structure */
    fprintf(file_ramdisk, "dolramdisk %s = { \"%s\", %d, _%s_filenames, _%s_filesizes, _%s_filecontents };\n\n",
        ramdiskname, ramdiskname, file_count, ramdiskname, ramdiskname, ramdiskname);

    /* close the ramdisk */
    fclose(file_ramdisk);

    printf("OK!\n");
  }
  else
    printf("No files found in \"%s\"!\n", argv[1]);

  /* free all allocated data */
  for (i = 0; i < file_count; i++)
  {
    free(filenames[i]);
    free(filecontents[i]);
  }
  free(filesizes);
  free(filenames);
  free(filecontents);

  printf("\n");

  return 0;
}
