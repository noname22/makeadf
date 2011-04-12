#define MAKEADF_VERSION "0.1"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "adflib.h"

void version()
{
	printf("\n== MakeAdf v. " MAKEADF_VERSION " ==\n\n");
}

void usage()
{
	version();
	printf(
		"Usage:\n"
		"  makeadf [OPTION] TARGET FILE [...]\n"
		"\n"
		"  Available options:\n"
		"    -l [LABEL]    set volume label, default: \"empty\"\n"
		"\n"
		"  Example:\n"
		"    makeadf -l myfloppy myfloppy.adf file.txt\n"
		"\n"
	);
}

int createFloppy(char* filename, char* label, char** files, int numFiles)
{
	adfEnvInitDefault();

	// creates a DD floppy empty dump
	// cyl = 80, heads = 2, sectors = 11. HD floppies has 22 sectors

	struct Device* floppy = adfCreateDumpDevice(filename, 80, 2, 11);
	if(!floppy)
	{
		fprintf (stderr, "could not create new dump device / file: %s\n", filename);
		return -1;
	}

	// create the filesystem : OFS with DIRCACHE
	if(adfCreateFlop(floppy, label, 0 /*FSMASK_DIRCACHE*/) != RC_OK){
		fprintf (stderr, "could not create floppy in device / file: %s\n", filename);
		return -2;
	}

	// Mount the volume
	struct Volume* vol = adfMount(floppy, 0, FALSE);

	if(!vol){
		fprintf (stderr, "could not mount volume in: %s\n", filename);
		return -5;
	}
	
	for (int i = 0; i < numFiles; i++){
		printf ("Adding: %s\n", files[i]);
		struct File* file = adfOpenFile(vol, files[i], "w");
		if (!file) {
			fprintf (stderr, "could not create file '%s' in adf: %s\n", files[i], filename);
			return -3;
		};

		FILE* in = fopen(files[i], "r");
		if(!in){
			fprintf (stderr, "could not open (local) file: %s\n", files[i]);
			return -4;
		}

		while(true){
			int get = fgetc(in);
			if(get == EOF){ break; }
			unsigned char c = (unsigned char)get;
			adfWriteFile(file, 1, &c);
		}

		fclose(in);
		adfCloseFile(file);
	}

	adfUnMount(vol);
	adfUnMountDev(floppy);
	adfEnvCleanUp();

	printf("done\n");

	return 0;
}

int main(int argc, char** argv)
{
	char defaultLabel[] = "empty";
	char* label = defaultLabel;
	int c;

	while ((c = getopt (argc, argv, "l:")) != -1){
		switch (c)
		{
			case 'l':
				label = optarg;
				break;
			case '?':
				if (optopt == 'l'){
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				} else if (isprint (optopt)) {
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				} else {
					fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
				}
				return 1;
			default:
				abort ();
		}
	}

	if(argc - optind <= 1){
		usage();
		return 0;
	}

	return createFloppy(argv[optind], label, argv + optind + 1, argc - optind - 1);
}
