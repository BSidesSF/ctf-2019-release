#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdbool.h>

char *g_Username, *g_ClientID;
bool g_LoggedIn;

char **ftpglob(register char *v);
void blkfree(char **e);
extern char *home;

char linebuf[512];

static bool process_input()
{
	char *p;
	const char *YYMARKER, *YYCURSOR, *id, *client; 
	volatile char *curClient = g_ClientID;

	if(fgets(linebuf, 511, stdin) == NULL) return false;
	p = strchr(linebuf, '\n');
	if(p) *p = 0;
	p = strchr(linebuf, '\r');
	if(p) *p = 0;

	YYCURSOR = linebuf;

	/*!stags:re2c format = 'const char *@@;'; */
/*!re2c
        re2c:define:YYCTYPE = u_int8_t;
        re2c:yyfill:enable = 0;

        end = "\x00";
	asciiStr = [a-z]+;
	clientStr = [a-zA-Z0-9@_-]+;
	formatStr = [a-mo-zA-MO-Z0-9\x21-\x2f]+;
	globStr = [a-zA-Z0-9\x21-\x2f\x7b-\x7f\x5b-\x60]+;
	unicodeStr = [\x01-\x1f\x21-\xff]+;
	space = "\x20";

        * { printf("error.. cursor is %s, marker is %s, and ch is %02x / '%c'\n", YYCURSOR, YYMARKER, yych, yych); return false; }

	end {
		return true;
	}

	( 'login' ) space @id asciiStr space @client clientStr end {
		if(strncmp(id, "anonymous", 9) == 0) {
			if(!g_Username) {
				g_Username = strndup(id, (int)(client - id - 1));
			}

			if(curClient) free(curClient);

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
		char buf[128];
		snprintf(buf, sizeof(buf), "300 Command \"%.*s\" is not supported", (int)(YYCURSOR - id), id);
		printf(buf);
		return true;
	}

	( 'site' ) space ( 'glob' ) space @id globStr end {
		// perform wuftpd ftpglob()
		char **result, **iter;
		char *path;
		int i = 0;

		path = strndup(id, (int)(YYCURSOR - id));
		iter = result = ftpglob(path);

		if(iter) {
			while(*iter) {
				i++;
				printf("220. %s\n", *iter);
				iter++;
			}
			printf("200 Done - %d entries!\n", i);

			blkfree(result);
			free(result);
		} else {
			printf("400 No files found!\n");
		}

		free(path);
		return true;
	}

	( 'retr' | 'get' | 'put' | 'dele' | 'post' ) {
		printf("300 Command not implemented\n");
		return true;
	}

	( 'list' ) {
		printf("200. flag.txt\n200 Done!\n");
		return true;
	}

	( 'help' ) end {
		printf("200. Recognized commands\n200. LOGIN userID clientID - perform login\n200. HELP - this command\n200. SITE GLOB parameter - perform server side globbing\n200. LIST - lists files on server\n200 Done!");
		return true;
	}

	( 'cwd' ) space @id globStr end {
		char **results;

		results = ftpglob(id);
		if(! results) {
			printf("321 No such directory \"%s\"\n", id);
			return true;
		}
		
		// due to tilde open bracket issue here, we free() an arbitrary
		// pointer
		blkfree(&results[1]);

		printf("331 Unable to change working directory to \"%s\"\n", id);

		free(results[0]);
		free(results);

		return true;
	}

	( 'úsame' ) space @id unicodeStr space @client unicodeStr end {
		if(! g_Username) {
			if(strncmp(id, "anónimo", (int)(client - id - 1)) != 0) {
				printf("400 Login failed\n");
				return true;
			}

			g_Username = strndup(id, (int)(client - id - 1));
		}

		if(g_ClientID) {
			free(g_ClientID);
			g_ClientID = NULL;
		}

		g_ClientID = strdup(client);

		printf("200 Login OK\n");
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

	printf("200 WaterDragon File Transfer Daemon\n");

	home = "/home/user/BSidesSF/WaterDragon";

	while(cont) {
		cont = process_input();
	}

	printf("200 Thank you, have a nice day!\n");
}
