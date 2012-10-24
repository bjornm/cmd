#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#define MAX_ENTRIES 32
#define SNOOZE 1

struct {
	time_t mtime;
	FILE *fh;
	char filename[256];
} entries[MAX_ENTRIES];

int nextfree = 0;

void close_all()
{
	int i;
	for(i=0;;i++) {
		if(!entries[i].mtime) break;
		fclose(entries[i].fh);
		fprintf(stderr,"%s closed\n",entries[i].filename);
	}
}

void shutdown(int a)
{
	close_all();
	exit(0);
}

int captured(char *filename)
{
	int i;
	for(i=0;;i++) {
		if(!entries[i].mtime)
			break;
		if(strcmp(entries[i].filename,filename) == 0)
			return 1;
	}
	return 0;
}

void update()
{
	int i = 0;
	DIR *dir;
	struct dirent *ent;
	dir = opendir(".");	
	while ((ent = readdir(dir)) != NULL) {
		struct stat st_,*st=&st_;
		stat(ent->d_name, st);
		if(S_ISREG(st->st_mode) && !captured(ent->d_name)) {
			printf("*** capturing entry: %s ***\n",ent->d_name);
			strcpy(entries[nextfree].filename ,ent->d_name);
			entries[nextfree].mtime = st->st_mtime;
			entries[nextfree].fh = fopen(ent->d_name,"r");
			fseek(entries[nextfree].fh,0,SEEK_END);
			nextfree++;
		}
		i++;
		if(i == MAX_ENTRIES) {
			fprintf(stderr,"more than max files (%d) in folder\n",MAX_ENTRIES);
			break;
		}
	}
	entries[i].mtime = 0;
	closedir(dir);
}

int main(int argc, char **argv)
{
	memset(entries,0,sizeof(entries));
	signal(SIGINT, shutdown);
	while(1) {
		char buf[1024];
		struct stat st_,*st=&st_;
		int i;
		int foundit = 0;
		update();
		for(i=0;;i++) {
			if(!entries[i].mtime) break;
			stat(entries[i].filename, st);
			if(st->st_mtime > entries[i].mtime) {
				entries[i].mtime = st->st_mtime;
				while((fgets(buf,1024,entries[i].fh)) != NULL) {
					printf("%s",buf);
				}
				foundit = 1;
			}
		}
		if(!foundit)
        		sleep(SNOOZE);
	}

	return 0;
}
