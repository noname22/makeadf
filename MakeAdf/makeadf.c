#define MAKEADF_VERSION "0.1"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <string.h>
#include <libgen.h>
#include <dirent.h>
#include <regex.h>
#include <stdarg.h>

#include "adflib.h"

typedef struct {
	bool recursive;
	char* label;
	char* adfFile;
	char* bootBlock;
} Settings;

void assertMsg(int eval, char *msg, ...)
{
	if(eval){
		return;
	}

	va_list fmtargs;
	char buffer[1024];

	va_start(fmtargs, msg);
	vsnprintf(buffer, sizeof(buffer) - 1, msg, fmtargs);
	va_end(fmtargs);

	fprintf(stderr, "%s\n", buffer);
	exit(1);
}

void version()
{
	printf("\n== MakeAdf v. " MAKEADF_VERSION " ==\n\n");
}

#if 0
void adfChdir(struct Volume* vol, const char* dir)
{
	struct List* list = adfGetDirEnt(struct Volume* vol, vol->curDirPtr);
	while(list){
		struct Entry* entry = (struct Entry*)list->content;
		if(entry->type == ST_DIR && !strcmp(entry->name, dir)){
			adfChangeDir(vol, dir);	
		}
		list = list->next;
	}
	
	fprintf(stderr, "could not cd into adf dir: %s\n", dir);
	exit(1);
}
#endif

void adfCopy(struct Volume* vol, const char* from, char* to)
{
	struct File* file = adfOpenFile(vol, to, "w");
	assertMsg(file != NULL, "could not create file '%s' in adf", from);

	FILE* in = fopen(from, "r");
	assertMsg(in != NULL, "could not open (local) file: %s", from);

	while(true){
		int get = fgetc(in);
		if(get == EOF){ break; }
		unsigned char c = (unsigned char)get;
		adfWriteFile(file, 1, &c);
	}

	fclose(in);
	adfCloseFile(file);
}

void usage()
{
	version();
	printf(
		"Usage:\n"
		"  makeadf [OPTION] TARGET FILE [...]\n"
		"\n"
		"  Available options:\n"
		"    -l [LABEL]     set volume label, default: \"empty\"\n"
		"    -r             recursively add files\n"
		"    -b [BOOTBLOCK] add a bootblock to the floppy from file\n"
		"\n"
		"  Example:\n"
		"    makeadf -l myfloppy myfloppy.adf file.txt\n"
		"\n"
	);
}

bool isDirectory(char* path)
{
	struct stat getStat;

	if(stat(path, &getStat) != 0){
		fprintf(stderr, "could not access file: %s\n", path);
		exit(1);
	}

	return S_ISDIR(getStat.st_mode);
}

void recursiveAdd(struct Volume* vol, const char* dir, const char* path, int depth)
{
	assertMsg(depth < 64, "directory structure too deep");

	struct dirent *entry;
	struct stat fs;

	DIR* d = opendir(dir);
	char* adfDir = basename(strdup(dir));

	char myPath[PATH_MAX];

	if(path){
		snprintf(myPath, PATH_MAX, "%s/%s", path, adfDir);
	}else{
		snprintf(myPath, PATH_MAX, "%s", adfDir);
	}
	
	printf(" [d] %s\n", myPath);

	assertMsg(d != NULL, "could not open directory");
	
	// Save local directory path
	char wd[PATH_MAX];
	assertMsg(getcwd(wd, PATH_MAX - 1) != NULL, "could not get working directory");

	// cd into local dir
	assertMsg(chdir(dir) >= 0, "could not change to directory: %s", dir);

	if(strcmp(adfDir, ".") != 0 && strcmp(adfDir, "..") != 0){
		// make directory in adf and cd into it
		assertMsg(adfCreateDir(vol, vol->curDirPtr, adfDir) == RC_OK, "could not make directory in adf");
		assertMsg(adfChangeDir(vol, adfDir) == RC_OK, "could not cd into new directory");
	}

	while ((entry = readdir(d))) {
		// skip . and ..
		if (!strcmp(".", entry->d_name) || !strcmp("..", entry->d_name)){
			continue;
		}

		assertMsg(stat(entry->d_name, &fs) >= 0, "could not stat: %s", entry->d_name);
		
		if ( S_ISDIR(fs.st_mode) ) {
			recursiveAdd(vol, entry->d_name, myPath, depth + 1);
		} else {
			printf(" [f] %s/%s\n", myPath, entry->d_name);
			adfCopy(vol, entry->d_name, entry->d_name);
		}
	}
	closedir(d);
	
	// Change up one dir in adf
	adfParentDir(vol);

	// Back out to saved local dir
	assertMsg(chdir(wd) >= 0, "could not change to prev dir");
}

void addBootBlock(struct Volume* vol, char* filename)
{
	unsigned char code[1024];
	memset(code, 0, sizeof(code));
	FILE* f = fopen(filename, "r");

	assertMsg(f != NULL, "could not open bootblock file: %s", filename);

	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	rewind(f);

	if(size != 1024){
		printf("warning: bootblock file is not 1024 bytes (it's %ld bytes)\n", size);
	}

	for(int i = 0; i < 1024; i++){
		int c = fgetc(f);
		if(c == EOF){
			break;
		}

		code[i] = (unsigned char)c;
	}

	assertMsg(adfInstallBootBlock(vol, code) == RC_OK, "could not install bootblock");
}

int createFloppy(Settings* settings, char** files, int numFiles)
{
	adfEnvInitDefault();

	// creates a DD floppy empty dump
	// cyl = 80, heads = 2, sectors = 11. HD floppies has 22 sectors

	struct Device* floppy = adfCreateDumpDevice(settings->adfFile, 80, 2, 11);
	if(!floppy)
	{
		fprintf (stderr, "could not create new dump device / file: %s\n", settings->adfFile);
		return 1;
	}

	// create the filesystem : OFS with DIRCACHE
	if(adfCreateFlop(floppy, settings->label, 0 /*FSMASK_DIRCACHE*/) != RC_OK){
		fprintf (stderr, "could not create floppy in device / file: %s\n", settings->adfFile);
		return 1;
	}

	// Mount the volume
	struct Volume* vol = adfMount(floppy, 0, FALSE);

	if(!vol){
		fprintf (stderr, "could not mount volume in: %s\n", settings->adfFile);
		return 1;
	}
	
	// Add the bootblock
	if(settings->bootBlock){
		printf("bootblock: %s\n", settings->bootBlock);
		addBootBlock(vol, settings->bootBlock);
	}
	
	for (int i = 0; i < numFiles; i++){
		if(isDirectory(files[i])){
			if(settings->recursive){
				recursiveAdd(vol, files[i], NULL, 0);
			}else{
				printf("skipping direcotry: %s\n", files[i]);
			}
			continue;
		}

		char* base = basename(strdup(files[i]));
		printf(" [f] %s\n", base);
		adfCopy(vol, files[i], base);
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
	Settings settings = {false, defaultLabel, NULL, NULL};

	int c;

	while ((c = getopt (argc, argv, "rl:b:")) != -1){
		switch (c)
		{
			case 'b':
				settings.bootBlock = optarg;
				break;
			case 'r':
				settings.recursive = true;
				break;
			case 'l':
				settings.label = optarg;
				break;
			case '?':
				if (optopt == 'l' || optopt == 'b'){
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

	settings.adfFile = argv[optind];

	return createFloppy(&settings, argv + optind + 1, argc - optind - 1);
}
