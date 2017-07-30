#ifndef WIN32GUI

//============================================================================

#include "qcc.h"
#include "hash.h"
#include <time.h>

#ifdef macintosh
#include <console.h>    // RIOT - SIOUX command-line argument support
#endif

char	sourcedir[1024];
char includedir[1024]; // you're gonna hate me for this

boolean summary;

void PrintFunction (char *name);

/*
============
main
============
*/


int main (int argc, char **argv)
{
	char	*src;
	char	*src2;
	char	filename[1024];
	int		p, crc, handle;
	time_t StartTime;
	int diff, i;

#ifdef macintosh
        argc = ccommand(&argv);     // RIOT
#endif

	print("--------------- frikqcc v2.6 ----------------");

	myargc = argc;
	myargv = argv;

	p = CheckParm("-hash");
	if (p)
	{
		print("hash value for %s is %i",argv[p+1], hash(argv[p+1]));
		exit(1);
	}
	if(CheckParm("-pause"))
		pr_pause = true;
	else if (CheckParm("-nopause"))
		pr_pause = false;
	else
		pr_pause = !(argc > 1);

	if ( CheckParm ("-?") || CheckParm ("-help") || CheckParm ("-h") || CheckParm("/?"))
	{
		print("frikqcc looks for progs.src in the current directory.");
		print("to look in a different directory: frikqcc -src <directory>");
//		print("to build a clean data tree: frikqcc -copy <srcdir> <destdir>");
//		print("to build a clean pak file: frikqcc -pak <srcdir> <packfile>");
		print("to set warning level: frikqcc -warn <warnlevel>");
		print("to decompile a progs.dat: frikqcc -dec <datfile>");
		print(" ");

		print("Optimizations:");
		print("\t/Ot\tshorten stores");
		print("\t/Oi\tshorten ifs");
		print("\t/Op\tnon-vector parms");
		print("\t/Oc\teliminate constant defs/names");
		print("\t/Od\teliminate duplicate defs");
		print("\t/Os\thash lookup in CopyString");
		print("\t/Ol\teliminate local, static and immediate names");
		print("\t/On\teliminate unneeded function names");
		print("\t/Of\tstrip filenames");
		print("\t/Ou\tremove unreferenced variable defs/names");
		print("\t/Oa\tremove constant arithmetic");
		print("\t/Oo\tAdd logic jumps");
		print("\t/Or\teliminate temps");
		print("\t/O1\tuse all optimizations except /Oo");
		print("\t/O2\tuse all optimizations");


		EndProgram(false);
		return 1;
	}
	if (CheckParm("-summary"))
		summary = true;
	else 
		summary = false;

	p = CheckParm("-d");
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
	p = CheckParm ("-src");
	if (p && p < argc-1 )
	{
		strcpy (sourcedir, argv[p+1]);
		if (sourcedir[strlen(sourcedir)-1] != '/')
			strcat (sourcedir, "/");
//		printf ("Source directory: %s\n", sourcedir);
	}
	else
		strcpy (sourcedir, "");
	// figure out the include path

	p = CheckParm ("-i");
	if (p && p < argc-1 )
	{
		strcpy (includedir, argv[p+1]);
		if (includedir[strlen(includedir)-1] != '/')
			strcat (includedir, "/");
	}
	else
	{
		strcpy (filename, argv[0]);
		StripFilename(filename);
		sprintf(includedir, "%s\\include\\", filename);
	}


	remove("error.log");

	InitData ();

	p = CheckParm ("-warn");
	if (p && p < argc-1 )
		warninglevel = atoi(argv[p+1]);
	else
		warninglevel = 2;
	logging = !(CheckParm ("-nolog"));

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

	if (CheckParm("/O2"))
	{
		pr_optimize_eliminate_temps = pr_optimize_shorten_ifs = pr_optimize_nonvec_parms 
		= pr_optimize_constant_names = pr_optimize_defs = pr_optimize_hash_strings = 
		pr_optimize_locals = pr_optimize_function_names = pr_optimize_filenames = 
		pr_optimize_unreferenced = pr_optimize_recycle = pr_optimize_logicops = pr_optimize_constant_arithmetic = 1;
	}
	else if (CheckParm("/O1"))
	{
		pr_optimize_eliminate_temps = pr_optimize_shorten_ifs = pr_optimize_nonvec_parms 
		= pr_optimize_constant_names = pr_optimize_defs = pr_optimize_hash_strings = 
		pr_optimize_locals = pr_optimize_function_names = pr_optimize_filenames = 
		pr_optimize_unreferenced = pr_optimize_recycle = pr_optimize_constant_arithmetic = 1;
	}
	if (CheckParm("/Debug"))
		pr_optimize_constant_names = pr_optimize_filenames = pr_optimize_function_names = pr_optimize_locals = 0;


// no cameras past this point
	StartTime = I_FloatTime();

	PR_BeginCompilation (PR_Malloc (0x100000), 0x100000);

	sprintf (filename, "%spreprogs.src", sourcedir);
	handle = open(filename, O_RDONLY);
	destfile[0] = 0;

	if(handle == -1)
	{
		close(handle);
		sprintf (filename, "%sprogs.src", sourcedir);
		LoadFile (filename, (void **)&src);
	
		src = COM_Parse (src);
		if (!src)
			Sys_Error ("No destination filename.  frikqcc -help for info.\n");
		strcpy (destfile, com_token);
	}
	else
	{
		close(handle);
		LoadFile (filename, (void **)&src2);

		if (!PR_CompileFile (src2, "preprogs.src") )
		{
			EndProgram(false);
			return 1;
		}
		if(!destfile[0])
		{
			print("Warning: No destination file was set, assuming progs.dat");
			strcpy(destfile, "progs.dat");
		}

		goto endcompilation;
	}



// compile all the files
	do
	{
		src = COM_Parse(src);
		if (!src)
			break;
		sprintf (filename, "%s%s", sourcedir, com_token);
		print ("%s", filename);
		LoadFile (filename, (void **)&src2);
		if(!PR_CompileFile (src2, com_token))
			EndProgram(false);
	} while (1);

endcompilation:

	if (!PR_FinishCompilation ())
	{
		EndProgram(false);
		return 1;
	}

	p = CheckParm ("-asm");
	if (p)
	{
		for (p++ ; p<argc ; p++)
		{
			if (argv[p][0] == '-')
				break;
			PrintFunction (argv[p]);
		}
	}



// write progdefs.h

	crc = PR_WriteProgdefs ("progdefs.h");
	
// write data file
	if (summary)
	{
		print("----------- Summary -----------");

		diff = I_FloatTime() - StartTime;
		print(" %02i:%02i elapsed time\n", (diff / 60) % 59, diff % 59);
	}
	WriteData (crc);
	if (CheckParm("-strings"))
		PrintStrings();
	if (CheckParm("-globals"))
		PrintGlobals();
	if (CheckParm("-functions"))
		PrintFunctions();
	if (CheckParm("-fields"))
		PrintFields();


// report / copy the data files
	CopyFiles ();

	//int sum = 0;
	//for (int i = 0 ; i < HSIZE1 ; i++)
	//	sum += stats[i] * stats[i];
	//printf("%6d sum\n", sum);
	if (summary)
	{
		if (pr_optimize_eliminate_temps || pr_optimize_shorten_ifs || pr_optimize_nonvec_parms
			|| pr_optimize_constant_names || pr_optimize_defs || pr_optimize_hash_strings ||
			pr_optimize_locals || pr_optimize_function_names || pr_optimize_filenames ||
			pr_optimize_unreferenced || pr_optimize_logicops || pr_optimize_recycle
			|| pr_optimize_constant_arithmetic) 
		{
			print("----------- Optimization Summary -----------");
			if (pr_optimize_eliminate_temps)
			{
				print("%d stores shortened", num_stores_shortened);
			}
			if (pr_optimize_shorten_ifs)
			{
				print("%d ifs shortened", num_ifs_shortened);
			}
			if (pr_optimize_nonvec_parms)
			{
				print("%d non-vector parms", num_nonvec_parms);
			}
			if (pr_optimize_constant_names)
			{
				print("%d bytes of constant defs/names eliminated", num_constant_names);
			}
			if (pr_optimize_defs)
			{
				print("%d duplicate defs eliminated", num_defs);
			}
			if (pr_optimize_hash_strings)
			{
				print("%d bytes of duplicate strings eliminated", num_strings);
			}
			if (pr_optimize_locals)
			{
				print("%d bytes of immediate and local names eliminated", num_locals_saved);
			}
			if (pr_optimize_function_names)
			{
				print("%d bytes of function names eliminated", num_funcs_saved);
			}
			if (pr_optimize_filenames)
			{
				print("%d bytes of filenames eliminated", num_files_saved);
			}
			if (pr_optimize_unreferenced)
			{
				print("%d unreferenced global defs eliminated", num_unreferenced);
			}
			if (pr_optimize_logicops)
			{
				print("%d logic jumps added", num_logic_jumps);
			}
			if (pr_optimize_recycle)
			{
				print("%d temporary globals recycled", num_recycled);
			}
			if (pr_optimize_constant_arithmetic)
			{
				print("%d constant arithmetic statements eliminated", num_constant_ops_saved);
			}
		}
	}


	EndProgram(true);
	return 1;
}



#endif
