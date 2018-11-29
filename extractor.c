
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <elf.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS  0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE -1
#endif

int load_file (char *filename, char **mem);
int get_section_index (char *elf, char *shname);
void check_elf_type (char *elf);

int main (int argc, char **argv)
{
	char *host, *output;
	int fd, index, size;

	Elf64_Ehdr *ehdr;
	Elf64_Phdr *phdr;
	Elf64_Shdr *shdr;

	if (argc != 4)
	{
		printf ("Usage: %s <host> <section name> <output>\n", argv[0]);
		exit (EXIT_SUCCESS);
	}

	load_file (argv[1], &host);
	
	check_elf_type (host);

	ehdr = (Elf64_Ehdr *)(host);
	phdr = (Elf64_Phdr *)(host + ehdr->e_phoff);
	shdr = (Elf64_Shdr *)(host + ehdr->e_shoff);

	if ((index = get_section_index (host, argv[2])) == -1)
	{
		fprintf (stderr, "Unkown section!\n");
		exit (EXIT_FAILURE);
	}
	size = shdr[index].sh_size;

	if ((fd = open (argv[3], O_CREAT | O_WRONLY, 0644)) < 0)
	{
		fprintf (stderr, "Create file %s failed\n", argv[3]);
		exit (EXIT_FAILURE);
	}
	if (!(output = (char *)malloc (size)))
	{
		fprintf (stderr, "Malloc %s failed\n", argv[3]);
		exit (EXIT_FAILURE);
	}
	memcpy (output, host + shdr[index].sh_offset, size);
	write (fd, output, size);

	close (fd);
	free (host);
	free (output);

	exit (EXIT_SUCCESS);
}

/*
 * get_section_index - Get target section index in section header
 */
int get_section_index (char *elf, char *shname)
{
	int len, i, shstrndx;
	char *shstrtab;

	Elf64_Ehdr *ehdr;
	Elf64_Shdr *shdr;

	ehdr = (Elf64_Ehdr *)(elf);
	shdr = (Elf64_Shdr *)(elf + ehdr->e_shoff);
	len = strlen (shname);
	shstrndx = ehdr->e_shstrndx;
	shstrtab =elf + shdr[shstrndx].sh_offset;

	for (i = 0; i < ehdr->e_shnum; i++)
	{
		if (!strncmp (&shstrtab[shdr[i].sh_name], shname, len))
			return i;
	}

	return -1;
}

int load_file (char *filename, char **mem)
{
        int fd, err_index;
	struct stat st;
        char *err_msg[] = {"Open", "Fstat", "Malloc", "Read"};

        if ((fd = open (filename, O_RDONLY)) < 0)
        {
                err_index = 0;
                goto FAILED;
        }
        if (fstat (fd, &st) < 0)
        {
                err_index = 1;
                goto FAILED;
        }
        if (!(*mem = (char *)malloc (st.st_size)))
        {
                err_index = 2;
                goto FAILED;
        }
        if (read (fd, *mem, st.st_size) != st.st_size)
        {
                err_index = 3;
                goto FAILED;
        }
        close (fd);

        return st.st_size;

FAILED:
        fprintf (stderr, "%s %s failed\n", err_msg[err_index], filename);
        exit (EXIT_FAILURE);
}

void check_elf_type (char *elf)
{
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf;

	if (elf[0] != 0x7f || strncmp (elf+1, "ELF", 3))
	{
		fprintf (stderr, "Target file is not an ELF file\n");
		exit (EXIT_FAILURE);
	}
	if (ehdr->e_type != ET_EXEC)
	{
		fprintf (stderr, "Target file is not an ELF executable\n");
		exit (EXIT_FAILURE);
	}
}
