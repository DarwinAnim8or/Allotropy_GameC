//-----------------------------------
// Windows GUI crap


#ifdef WIN32GUI


//#include <conio.h> 
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "resource.h"
#include "qcc.h"
#include "hash.h"

#include <time.h>

// this is getting downright ridiculous
char sourcedir[1024];
char includedir[1024]; // you're gonna hate me for this
char frikqccdir[1024];
char progssrc[1024];
// many k of extra junk, geeze...


boolean flagwarnings;
boolean summary;

void summary_print (char *format, ...)
{
	va_list         argptr;
	static char             string[1024];
	
	va_start (argptr, format);
	vsprintf (string, format,argptr);
	va_end (argptr);

	strcat(summary_buf, string);

}


char		*argv[50];
int			argc;

HINSTANCE	global_hInstance;
int	hwnd_dialog;
HWND hList;
HWND main_hWnd;
static HICON	hIcon;

// FIX ME, make safe
char *ED_Name[256];
int ED_IDNumber[256];
int ED_LineNumber[256];
int ED_count;

// special version of WinPrint that keeps track of error info too

void ErrorPrint(char *fname, int line, char *format, ...) 
{
	int ret;
	va_list         argptr;
	static char             string[1024];

	va_start (argptr, format);
	vsprintf (string, format,argptr);
	va_end (argptr);

	ED_count++;
 	ret = SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)string);
	if (ED_count >= 256)
	{
		WinPrint("Ran out of room");
		ED_count = 0;
	}
	ED_LineNumber[ED_count] = line;
	ED_Name[ED_count] = fname;
	ED_IDNumber[ED_count] = ret;

	SendMessage(hList, LB_SETITEMDATA, ret, ED_count);
	SendMessage(hList, LB_SETCURSEL, ret, 0);

}

void WinPrint(char *format, ...) 
{
	int ret;
	va_list         argptr;
	static char             string[1024];

	va_start (argptr, format);
	vsprintf (string, format,argptr);
	va_end (argptr);

 	ret = SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)string);
	SendMessage(hList, LB_SETITEMDATA, ret, -1);
	SendMessage(hList, LB_SETCURSEL, ret, 0);
}

void WinRun(char *command) 
{
  HINSTANCE hInstance = ShellExecute(NULL, "open", command, NULL, NULL, 1);  
  if (hInstance <= (HINSTANCE) 32)
  {
   //Error("Failed to run \"%s\", Error: %d", command, GetLastError());
	  WinPrint("Failed to run \"%s\", Error: %d", command, GetLastError());

  }

}

void PR_Clear (void);

void EditButton (void)
// for now...
{
	int ret1, ret2;
	ret1 = SendMessage(hList, LB_GETCURSEL, 0, 0);
	ret2 = SendMessage(hList, LB_GETITEMDATA, ret1, 0);
	if (ret2 == -1)
		return; // Nope, sorry...
	if (ED_IDNumber[ret2] != ret1)
		return; // that's and old compile

	WinRun(ED_Name[ret2]);


}

void CompileIt (int optimizeflag)
{
	char	*src;
	char	*src2;
	char	filename[1024];
	int		i, p, crc, handle;

	PR_Clear(); // hacky fun for the whole family!
	ED_count = 0;

	if (argc <= 1 || argv[1][0] == '-' || argv[1][0] == '/')
	{
		if (!GetCurrentDirectory (sizeof(sourcedir), sourcedir))
			Sys_Error ("Couldn't determine current directory");
		strcat(sourcedir, "\\");
//		sprintf (sourcedir, "");
		strcpy(progssrc, "progs.src");
	}
	else
	{
		strcpy (sourcedir, argv[1]);
		ExtractFilename(argv[1], progssrc);
		StripFilename(sourcedir);
//		SetCurrentDirectory(sourcedir);
		strcat (sourcedir, "\\");
//		sprintf (sourcedir, "");
	}
	p = CheckParm("d");
	if (p)
	for(i = p + 1; i < argc; i++)
	{
		if (argv[i][0] == '-' || argv[i][0] == '/')
			break;

		num_defines++;
		macrohash[hash(argv[i])] = num_defines;
		pr_defines[num_defines].name = (char *)PR_Malloc(strlen(argv[i]) + 1);
		strcpy(pr_defines[num_defines].name, argv[i]);
	}
	p = CheckParm ("-i");
	if (p && p < argc-1 )
	{
		strcpy (includedir, argv[p+1]);
		if (includedir[strlen(includedir)-1] != '/')
			strcat (includedir, "/");
	}

	if (CheckParm("-summary"))
		summary = true;
	else 
		summary = false;
	remove("error.log");
	InitData ();
	warninglevel = 2;
	logging = !(CheckParm ("-nolog"));
	pr_pause = !CheckParm("-nopause");
	pr_dumpasm = false;

	pr_optimize_eliminate_temps = CheckParm("/Ot");
	pr_optimize_shorten_ifs = CheckParm("/Oi");
	pr_optimize_nonvec_parms = CheckParm("/Op");
	pr_optimize_constant_names = CheckParm("/Oc");
	pr_optimize_defs = CheckParm("/Od");
	pr_optimize_hash_strings = CheckParm("/Os");
	pr_optimize_locals = CheckParm("/Ol");
	pr_optimize_function_names = CheckParm("/On");
	pr_optimize_filenames = CheckParm("/Of");
	pr_optimize_unreferenced = CheckParm("/Ou");
	pr_optimize_logicops = CheckParm("/Oo");
	pr_optimize_recycle = CheckParm("/Or");
	pr_optimize_constant_arithmetic = CheckParm("/Oa");



	if (CheckParm("/O2") || optimizeflag)
	{
		pr_optimize_eliminate_temps = pr_optimize_shorten_ifs = pr_optimize_nonvec_parms 
		= pr_optimize_constant_names = pr_optimize_defs = pr_optimize_hash_strings = 
		pr_optimize_locals = pr_optimize_function_names = pr_optimize_filenames = 
		pr_optimize_unreferenced = pr_optimize_logicops = pr_optimize_recycle = pr_optimize_constant_arithmetic = 1;
	}
	else if (CheckParm("/O1"))
	{
		pr_optimize_eliminate_temps = pr_optimize_shorten_ifs = pr_optimize_nonvec_parms 
		= pr_optimize_constant_names = pr_optimize_defs = pr_optimize_hash_strings = 
		pr_optimize_locals = pr_optimize_function_names = pr_optimize_filenames = pr_optimize_constant_arithmetic = 
		pr_optimize_unreferenced = pr_optimize_recycle = 1;
	}


	sprintf (filename, "%s%s", sourcedir,progssrc);
	LoadFile (filename, (void **)&src);

	if(STRCMP(progssrc, "preprogs.src"))
	{
		src = COM_Parse (src);
		if (!src)
		{
			Sys_Error ("No destination filename.  qcc -help for info.\n");
			return;
		}
		strcpy (destfile, com_token);

	// compile all the files
		do
		{
			src = COM_Parse(src);
			if (!src)
				break;
			sprintf (filename, "%s%s", sourcedir, com_token);
	//		printf ("%s\n", filename);
			LoadFile (filename, (void **)&src2);

			if (!PR_CompileFile (src2, com_token) )
			{
				EndProgram(false);
				return;
			}
				
		} while (1);
	}
	else
	{
		if (!PR_CompileFile (src, progssrc) )
		{
			EndProgram(false);
			return;
		}
		if(!destfile)
			strcpy(destfile, "progs.dat");

	}

	if (!PR_FinishCompilation ())
	{
		EndProgram(false);
		return;
		//Sys_Error ("compilation errors");
	}


// write progdefs.h

	crc = PR_WriteProgdefs ("progdefs.h");
	
// write data file
	WriteData (crc);

// report / copy the data files
	CopyFiles ();

	EndProgram(true);

}

INT CALLBACK MainDlgProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int ret;
	hList = GetDlgItem(hWnd, IDC_LIST1);

	switch (iMsg)
	{
		case WM_INITDIALOG:
		{
			SendMessage (hWnd, WM_SETICON, (WPARAM)TRUE, (LPARAM)hIcon);
			SendMessage (hWnd, WM_SETICON, (WPARAM)FALSE, (LPARAM)hIcon);
		} break;

		case WM_COMMAND:
		{
			if (LOWORD(wParam) == ID_OK)
			{
				ret = SendMessage(GetDlgItem(hWnd, IDC_CHECK1), BM_GETSTATE, 0, 0);
				CompileIt (ret & BST_CHECKED);
			}
				
			if (LOWORD(wParam) == ID_CANCEL)
			{
				EndDialog(hWnd, 1);
				exit(1);
			}
			if (LOWORD(wParam) == IDC_EDITBUTTON)
			{
				EditButton();
			}

		} break;

		case WM_CLOSE:
			EndDialog(hWnd, 1);
			exit(1);
			break;
		//default:
		//	return DefWindowProc(hWnd, iMsg, wParam, lParam);
	}

	return 0;
}


static char	*empty_string = "";


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG				msg;

	double			time, oldtime, newtime;
	MEMORYSTATUS	lpBuffer;
	int				t;
	WNDCLASS		wc;
	HDC				hdc;
	int				i, handle;
	RECT			rect;
	char filename[1024];
	char	*src2;

	argc = 1;
	argv[0] = empty_string;

    if (hPrevInstance)
        return 0;

	while (*lpCmdLine && (argc < 50))
	{
		while (*lpCmdLine && ((*lpCmdLine <= 32) || (*lpCmdLine > 126)))
			lpCmdLine++;

		if (*lpCmdLine)
		{
			argv[argc] = lpCmdLine;
			argc++;

			while (*lpCmdLine && ((*lpCmdLine > 32) && (*lpCmdLine <= 126)))
				lpCmdLine++;

			if (*lpCmdLine)
			{
				*lpCmdLine = 0;
				lpCmdLine++;
			}

		}
	}

	myargv = argv;
	myargc = argc;
	PR_BeginCompilation (PR_Malloc (0x100000), 0x100000);
	InitData ();

	GetModuleFileName(NULL, frikqccdir, sizeof(frikqccdir));
	StripFilename(frikqccdir);
	/*
	sprintf (filename, "%s\\frikqcc.cfg", frikqccdir);
	handle = open(filename,O_RDONLY | O_BINARY);

	if (handle == -1)
	{
		close(handle);
		WinRun(va("%s\\Associate.exe", frikqccdir));
		exit(1);
	}

	if (handle != -1)
	{
		LoadFile (filename, (void **)&src2);
		if (!PR_CompileFile (src2, filename) )
		{
			EndProgram(false);
			return TRUE;
		}
	}
	*/
	hIcon = LoadIcon (hInstance, MAKEINTRESOURCE (IDR_MAINFRAME));
	hwnd_dialog = DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINWIN), NULL, &MainDlgProc);
    return TRUE;
}



#endif