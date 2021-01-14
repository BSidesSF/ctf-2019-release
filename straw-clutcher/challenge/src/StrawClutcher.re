#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/queue.h>
#include <err.h>
#include <sys/mman.h>
#include <errno.h>

char *g_Username, *g_ClientID;
bool g_LoggedIn;

#define FILENAMESIZE 40

struct filedata {
	char filename[FILENAMESIZE];
	long length;
	unsigned char *data;
	int mmap;

	SLIST_ENTRY(filedata) entries;
};

SLIST_HEAD(slisthead, filedata) head = 
	SLIST_HEAD_INITIALIZER(head);



static bool filename_exists(char *filename)
{
	struct filedata *fdp;

	SLIST_FOREACH(fdp, &head, entries) {
		if(strcmp(fdp->filename, filename) == 0) {
			return true;
		}
	}

	return false;
}

static bool get_data_small(struct filedata *fdp)
{
	unsigned char data[fdp->length];
	unsigned long max, cnt, tmp, sz, remainder;

	max = fdp->length;

	for(cnt = 0, tmp = 0; cnt < max;) {
		remainder = fdp->length - cnt;
		sz = 64;

		if(remainder > 4096) {
			sz = 4096;
		} else {
			sz = remainder;
		}

		// printf("sz is %ld\n", sz);

		tmp = fread(data + cnt, 1, sz, stdin);
		if(tmp <= 0) {
			printf("400. Unable to read data: %s\n", strerror(errno));
			return false;
		}

		cnt += tmp;
	}
	
	fdp->data = malloc(max);
	if(fdp->data == NULL) {
		printf("400 Unable to allocate memory\n");
		return false;
	}

	memcpy(fdp->data, data, max);
	printf("200 Entry created\n");
	return true;
}


static bool get_data(struct filedata *fdp)
{
	long cnt, tmp;

	if(fdp->length < (1024 * 1024)) {
		return get_data_small(fdp);
	}

	fdp->mmap = true;

	fdp->data = mmap(NULL, fdp->length, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if(fdp->data == MAP_FAILED) {
		printf("400 file too large\n");
		return true;
	}
	// printf("mmaping data\n");

	for(cnt = 0; cnt < fdp->length;) {
		tmp = fread(fdp->data + tmp, 1, fdp->length - tmp, stdin);
		if(tmp <= 0) {
			printf("400 unable to read data\n");
			return false;
		}
		cnt += tmp;
	}

	return true;
	
}

static bool upload_file(char *filename, long length)
{
	struct filedata *fdp;
		
	if(filename_exists(filename)) {
		printf("400 Filename already exists\n");
		return true;
	}


	if(length > (long)(1024 * 1024 * 8)) {
		printf("400 File size too large - %ld / %016x bytes.\n", length, length);
		return true;
	}

	fdp = calloc(sizeof(struct filedata), 1);
	if(! fdp) {
		errx(1, "calloc: can't allocate");
	}

	if(strlen(filename) >= FILENAMESIZE) {
		printf("400 File name too long\n");
		return false;
	}

	strncpy(fdp->filename, filename, strlen(filename));
	fdp->length = length;

	if(get_data(fdp) == false) {
		printf("400 Unable to get data :(\n");
		return false;
	}

	SLIST_INSERT_HEAD(&head, fdp, entries);
	return true;

}

static bool delete_file(char *filename)
{
	struct filedata *fdp;

	SLIST_FOREACH(fdp, &head, entries) {
		// i need previous entry :/ hmm.
		if(strcmp(fdp->filename, filename) == 0) {
			SLIST_REMOVE(&head, fdp, filedata, entries);
					
			if(fdp->mmap) {
				munmap(fdp->data, fdp->length);
			} else {
				free(fdp->data);
			}

			free(fdp);
			printf("200 Filename \"%s\" removed\n", filename);
			return true;
		}		
	}

	printf("400 Filename \"%s\" not found\n", filename);
	return true;
}

static bool download_file(char *filename)
{
	struct filedata *fdp;
	unsigned long cur, tmp;

	SLIST_FOREACH(fdp, &head, entries) {
		if(strcmp(fdp->filename, filename) == 0) break;
	}

	// printf("fdp is %p\n", fdp);

	if(!fdp) {
		printf("354 Can't download file - \"%s\" - no such file!\n", filename);
		return true;
	}

	printf("200 File download started. Prepare to get %ld bytes\n", fdp->length);
	
	for(cur = 0; cur < fdp->length;) {
		tmp = fwrite(fdp->data + cur, 1, fdp->length - cur, stdout);
		if(tmp <= 0) {
			printf("400 Can't write data: %s\n", strerror(errno));
			return false;
		}
		cur += tmp;	
	}
	
	printf("200 Data transferred!\n");
	return true;
}

/*
 * This section causes the fdp->length section to be overwritable, thus
 * modifying the "length" field, which then causes an info leak to be
 * possible.
 */

static bool rename_file(char *source, char *destination)
{
	struct filedata *fdp;

	SLIST_FOREACH(fdp, &head, entries) {
		if(strcmp(fdp->filename, source) == 0) {
			// Below is needed to avoid memcpy_chk and SSP.
			__builtin_memcpy(fdp->filename, destination, strlen(destination));

			printf("200 Filename renamed\n");
			return true;
		}
	}

	printf("400 File not found\n");
	return true;
} 

static bool truncate_file(char *source, long length)
{
	struct filedata *fdp;
	unsigned long newsz = length;

	SLIST_FOREACH(fdp, &head, entries) {
		if(strcmp(fdp->filename, source) == 0) {
			if(newsz >= fdp->length) {
				printf("400 New size must be smaller than existing size (%ld vs %ld)\n");
				return true;
			}

			fdp->length = length;
			printf("200 File resized correctly!\n");
			return true;
		}
	}

	printf("400 File not found!\n");
	return true;

}

static void list_files()
{
	struct filedata *fdp;


	printf("231. Files as follows\n");

	SLIST_FOREACH(fdp, &head, entries) {
		printf("232. \"%s\" - %ld bytes\n", fdp->filename, fdp->length);
	}

	printf("231 Done!\n");
}

static bool process_input()
{
	static char *linebuf, *p;
	const char *YYMARKER, *YYCURSOR, *id, *client; 

	if(linebuf == NULL) {
		linebuf = malloc(512);
		linebuf[511] = 0;
	}

	if(fgets(linebuf, 511, stdin) == NULL) return false;
	p = strchr(linebuf, '\n');
	if(p) *p = 0;
	p = strchr(linebuf, '\r');
	if(p) *p = 0;

	YYCURSOR = linebuf;

	/*!stags:re2c format = 'const char *@@;'; */
/*!re2c
        re2c:define:YYCTYPE = char;
        re2c:yyfill:enable = 0;

        end = "\x00";
	asciiStr = [a-z]+;
	clientStr = [a-zA-Z0-9@_-]+;
	formatStr = [a-mo-zA-MO-Z0-9\x21-\x2f]+;
	globStr = [a-zA-Z0-9\x21-\x2f\x7b-\x7f\x5b-\x60]+;
	filenameStr = [a-zA-Z0-9]+ "\x2e" [a-zA-Z0-9]{3};
	numStr = [0-9]+;
	space = "\x20";

        * { printf("error.. cursor is %s, marker is %s, and ch is '%c'\n", YYCURSOR, YYMARKER, yych); return false; }

	( 'login' ) space @id asciiStr space @client clientStr end {
		/*printf("hmm what do we have here\n");
		printf("id is %.*s, and client is %.*s\n", 
			(int) (client - id - 1), id,
			(int) (YYCURSOR - client), client); */

		if(strncmp(id, "anonymous", 9) == 0) {
			if(!g_Username) {
				g_Username = strndup(id, (int)(client - id - 1));
			}

			if(g_ClientID) free(g_ClientID);

			g_ClientID = strndup(client, (int)(YYCURSOR - client));

			printf("200 Successfully logged in!\n");
			g_LoggedIn = true;
			return true;
		}

		g_LoggedIn = false;
		printf("400 only supports anonymous logins!\n");
		return true;
	}

	( 'site' ) space ( 'index' | 'exec' ) space @id formatStr end {
		printf("300 Command \"%.*s\" is not supported", (int)(YYCURSOR - id), id);
		return true;
	}

	( 'site' ) space ( 'glob' ) space @id globStr end {
		printf("400 Command is not supported\n");
		return true;
	}

	( 'put' | 'stor' ) space @id filenameStr space @client numStr end {
		long len;
		bool ret;
		char *filename;

		len = strtoul(client, NULL, 10);
		if(len == 0) {
			printf("300 At least upload some data\n");
			return true;
		}

		filename = strndup(id, (int)(client - id - 1));
		ret = upload_file(filename, len);
		free(filename);

		return ret;
	} 

	( 'get' | 'retr' ) space @id filenameStr end {
		char *filename;
		bool ret;

		filename = strndup(id, (int)(YYCURSOR - id));
		
		ret = download_file(filename);
		free(filename);
	
		return ret;
	}
	
	( 'dele' ) space @id filenameStr end {
		char *filename;
		bool ret;

		filename = strdup(id);
		ret = delete_file(filename);
		free(filename);

		return ret;
	}

	( 'trunc' ) space @id filenameStr space @client numStr end {
		char *filename;
		long size;
		bool ret;

		size = strtol(client, NULL, 10);

		filename = strndup(id, (int)(client - id - 1));
		ret = truncate_file(filename, size);
		free(filename);
		
		return ret;
	}

	( 'rename' ) space @id filenameStr space @client filenameStr end {
		char *fn1, *fn2;
		bool ret;

		fn1 = strndup(id, (int)(client - id - 1));
		if(strlen(fn1) >= 32) {
			printf("300 Filename too long.\n");
			return true;
		}

		fn2 = strndup(client, (int)(YYMARKER - client));
		if(strlen(fn1) >= 32) {
			// Buggy here. Should be fn2
			printf("300 Destination filename too long.\n");
			return true;
		}

		// Check if filename exists in records

		ret = rename_file(fn1, fn2);

		free(fn1);
		free(fn2);
		return ret;

	}

	( 'list' ) {
		list_files();
		return true;
	}

	( 'help' ) end {
		printf("200. Recognized commands\n200. LOGIN userID clientID - perform login\n200. HELP - this command\n200. SITE GLOB parameter - perform server side globbing\n200. LIST - lists files on server\n200 Done!");
		return true;
	}

*/
	return false;
}

int main(int argc, char **argv)
{
	bool cont = true;

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	SLIST_INIT(&head);

	printf("200 WaterDragon File Transfer Daemon\n");

	while(cont) {
		cont = process_input();
	}

	printf("200 Thank you, have a nice day!\n");
}
