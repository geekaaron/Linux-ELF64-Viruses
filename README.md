# Linux-ELF64-Viruses

#### Files
+ **tvirus.c** - text padding virus
+ **dvirus.c** - data padding virus (still have some issue
+ **extractor.c** - Just a little elf tool that extract specified section
+ **hello.s** - Host file for infection that written by at&t assembly
+ **test.s** - Virus file for infection that written by at&t assembly
+ **Makefile** - make file for make

---

#### Step
```
1.	$ make all
2.	$ ./extractor test .text parasite
3.	$ ./tvirus hello parasite evil_hello 
```
You can use:
```
$ objcopy -O binary -j .text test
```
in step 2 too.
```
$ ./hello
$ ./evil_hello
```
And when you execute **hello** and **evil_hello** then you will see:
