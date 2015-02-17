#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <io.h>
#include <Windows.h>




#define SECTION				"minecraft"

#define POINTER_COMMAND		"[Game Launcher/INFO] Command: "
#define POINTER_ERROR		"[Game Launcher/ERROR]"




static	char	root			[512];
static	char	root_game		[768];
static	char	root_java		[768];
static	char	root_launcher	[768];

static	char	path_config			[1024];
static	char	path_java			[1024];
static	char	path_launcher_jar	[1024];
static	char	path_launcher_log	[1024];


static	int		cfg_fastboot;
static	char	cfg_java		[1024];


static	char	command			[65536];
static	char *	directory;




static bool error (const char *format, ...) {
	va_list ap;

	va_start (ap, format);

	int		length	= vsnprintf (NULL, 0, format, ap);
	char *	buffer	= new char [length + 16];

	vsprintf (buffer, format, ap);

	va_end (ap);

	MessageBoxA (NULL, buffer, "", MB_ICONERROR | MB_OK);

	delete buffer;

	return false;
}




static bool load_directory (void) {
	if(GetModuleFileNameA (NULL, root, MAX_PATH) > 0) {
		char *	separator	= strrchr (root, '\\');

		if(separator == NULL) {
			return error ("获取的应用程序路径无效。");
		} else {
			* (separator + 1) = '\0';
		}
	} else {
		return error ("获取应用程序路径失败。");
	}

	sprintf (root_game,		"%s%s",		root,	".minecraft"	);
	sprintf (root_java,		"%s%s",		root,	"jre\\bin\\"	);
	sprintf (root_launcher,	"%s%s",		root,	"launcher\\"	);

	sprintf (path_config,			"%s%s",		root,			"minecraft.ini"		);
	sprintf (path_java,				"%s%s",		root_java,		"javaw.exe"			);
	sprintf (path_launcher_jar,		"%s%s",		root_launcher,	"HMCL-2.0.7.jar"	);
	sprintf (path_launcher_log,		"%s%s",		root_launcher,	"hmcl.log"			);

	return true;
}


static bool load_config (void) {
	cfg_fastboot	= (int) GetPrivateProfileIntA (SECTION, "fastboot", 0, path_config);

	GetPrivateProfileStringA (SECTION, "java", "", cfg_java, sizeof cfg_java, path_config);

	return true;
}


static bool load_java (void) {
	if(* cfg_java == '\0') {
		if(_access (path_java, 0) == -1) {
			sprintf (path_java, "javaw.exe");
		}
	} else {
		strcpy (path_java, cfg_java);
	}

	return true;
}



static bool check_fastboot (void) {
	if(cfg_fastboot == 0) {
		return false;
	}

	FILE *	fp	= fopen (path_launcher_log, "r");

	if(fp == NULL) {
		return false;
	}

	char	buffer	[65536];

	bool	status_read		= false;
	bool	status_error	= false;

	while(!feof (fp)) {
		if(fgets (buffer, sizeof buffer, fp) == NULL) {
			break;
		}

		char *	pointer_command	= strstr (buffer, POINTER_COMMAND	);
		char *	pointer_error	= strstr (buffer, POINTER_ERROR		);

		if(pointer_command != NULL) {
			pointer_command	+= strlen (POINTER_COMMAND);

			char * p	= pointer_command;

			while(* p != '\0') {
				switch(* p) {
					case '\r':
					case '\n':
						* p	= '\0';
				}

				p ++;
			}

			if(strlen (pointer_command) > 0) {
				status_read	= true;

				if(strstr (pointer_command, "javaw.exe ") == pointer_command) {
					sprintf (command, "\"%s\" %s", path_java, pointer_command + strlen ("javaw.exe "));
				} else {
					strcpy (command, pointer_command);
				}
			}
		}

		if(pointer_error != NULL) {
			status_error	= true;
		}
	}

	fclose (fp);

	if((status_read == true) && (status_error == false)) {
		return true;
	} else {
		return false;
	}
}




static void build_fastboot (void) {
	directory	= root_game;
}


static void build_launcher (void) {
	directory	= root_launcher;

	sprintf (command, "\"%s\" -jar \"%s\"", path_java, path_launcher_jar);
}




int CALLBACK WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	if(load_directory	( ) == false)	goto end;
	if(load_config		( ) == false)	goto end;
	if(load_java		( ) == false)	goto end;

	if(check_fastboot ( ) == true) {
		build_fastboot ( );
	} else {
		build_launcher ( );
	}

	STARTUPINFO			sa;
	PROCESS_INFORMATION	pa;

	memset (& sa, 0, sizeof sa);

	sa.cb	= sizeof sa;

	if(CreateProcessA (NULL, command, NULL, NULL, FALSE, 0, NULL, directory, & sa, & pa) == FALSE) {
		error ("启动命令失败：\n%s", command);
	}

	CloseHandle (pa.hProcess);
	CloseHandle (pa.hThread);


end:
	return 0;
}



