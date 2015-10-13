// morsemuzak
//
// This little program takes a text input file and produces notelist seq blocks.
// For each character in the input a series of notes is produces that sounds out
// the morse code for that letter, space, or punctuation. The pitch series may
// be chosen randomly or sequentially from a chromatic scale or a user supplied 
// pitch series. The base duration for a dot, dash, or break may be specified.
//
// Each line in the input begins with a command letter followed by the options.
// s Text       - output Text as morse code
// n name       - name for the sequence, default is the sequence Text
//              - this is cleared after the next sequence is generated.
// p p1 p2 pn   - set the pitches to the indicated values. If the command is p+ pitches are added.
// v volume     - set the volume level
// u unit       - set the time unit for a "dot"
// a params     - parameters to the notes
//
// Command line parameters are:
// morsemuzak -utime -dtime -wtime -ltime -vn -ppitches -s{r|c} -iinfile -ooutfile
//      -utime = time of one unit
//      -dtime = time of one dot (default = unit x 2, dash = dot x 2)
//      -ltime = time between letters (default = unit x 3)
//      -wtime = time between words (default = unit x 7)
//      -ppitches = file containing pitches, one per line (max 256)
//      -s{r|c} = pitch handling (r = random selection, c = cycle)
//      -vn = volume level (default = 100)
//      -as = parameters
//      -iinfile = input file for text (default = stdin)
//      -ooutfile = output file (default = stdout)
//
// Daniel R. Mitchell - Sept. 2006

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

struct morse
{
	short letter;
	char *thebits;
};

morse morseltr[] =
{
	{ 'A', ".-" },
	{ 'B', "-..." },
	{ 'C', "-.-." },
	{ 'D', "-.." },
	{ 'E', "." },
	{ 'F', "..-." },
	{ 'G', "--." },
	{ 'H', "...." },
	{ 'I', ".." },
	{ 'J', ".---" },
	{ 'K', "-.-" },
	{ 'L', ".-.." },
	{ 'M', "--" },
	{ 'N', "-." },
	{ 'O', "---" },
	{ 'P', ".--." },
	{ 'Q', "--.-" },
	{ 'R', ".-." },
	{ 'S', "..." },
	{ 'T', "-" },
	{ 'U', "..-" },
	{ 'V', "...-" },
	{ 'W', ".--" },
	{ 'X', "-..-" },
	{ 'Y', "-.--" },
	{ 'Z', "--.." },
	{ 0, NULL }
};

morse morsenum[] =
{
	{ '0', "-----" },
	{ '1', ".----" },
	{ '2', "..---" },
	{ '3', "...--" },
	{ '4', "....-" },
	{ '5', "....." },
	{ '6', "-...." },
	{ '7', "--..." },
	{ '8', "---.." },
	{ '9', "----." },
	{ 0, NULL }
};

morse morsepunct[] =
{
	{ '.', ".-.-.-" },
	{ ',', "--..--" },
	{ ':', "---..." },
	{ '?', "..--.." },
	{ '\'', ".----." },
	{ '-', "-....-" },
	{ '/', "-..-." },
	{ '[', "-.--.-" },
	{ '\"', ".-..-." },
	{ 0, NULL }
};

double unit;
double dot;
double dash;
double lrest;
double wrest;
double ttotal;
long vollevel;
char *params = NULL;
FILE *infp;
FILE *outfp;

#define MAXPITCH 256
char *pitches[MAXPITCH];
char *stdptch[] = { "C8", "C#8", "D8", "D#8", "E8", "F8", "F#8", "G8", "G#8", "A8", "Bb8", "B8", NULL };
int npit = 0;
int pitseq = 0;
int nextpit = 0;

// Initialize the pitch set to a chromatic scale beginning at middle C
void initpitch()
{
	npit = 0;
	nextpit = 0;
	while (stdptch[npit] != NULL)
	{
		pitches[npit] = strdup(stdptch[npit]);
		npit++;
	}
}

// add the next pitch to the pitch set
void setpitch(char *inbuf)
{
	char *cp = inbuf;
	while (*cp && isspace(*cp))
		cp++;
	char *pstr = cp;
	while (*cp)
	{
		if (isspace(*cp))
			break;
		cp++;
	}
	*cp = '\0';
	if (npit < MAXPITCH && *pstr != '\0')
		pitches[npit++] = strdup(pstr);
}

// Clear the pitch set
void clearpitch()
{
	for (int i = 0; i < npit; i++)
	{
		if (pitches[i] != NULL)
		{
			free(pitches[i]);
			pitches[i] = NULL;
		}
	}
	npit = 0;
	nextpit = 0;
}

// Scan pitches from a data file line.
void scanpitch(char *line)
{
	char inbuf[128];
	char *cp;
	int ch;

	while (*line && npit < MAXPITCH)
	{
		cp = inbuf;
		while (isspace(*line))
			line++;
		while ((ch = *line) != 0)
		{
			line++;
			if (isspace(ch) || ch == ',' || ch == ';')
				break;
			*cp++ = ch;
		}
		*cp = '\0';
		setpitch(inbuf);
	}
}

// Load list of pitches from a file.
void loadpitch(char *fname)
{
	FILE *fp = fopen(fname, "r");
	if (fp == NULL)
	{
		perror(fname);
		exit(1);
	}

	char inbuf[128];
	char *cp;

	npit = 0;
	while (fgets(inbuf, 128, fp) != NULL)
	{
		if ((cp = strchr(inbuf, '\n')) != NULL)
			*cp = '\0';
		setpitch(inbuf);
	}

	fclose(fp);
}

// Write morse code for one character. The parameter points to the morse string.
void writemorse(char *p)
{
	fputs("\t{ ", outfp);

	int index;
	int n = strlen(p);
	int i;
	for (i = 0; i < n; i++)
	{
		if (i != 0)
			fputs(", ", outfp);
		if (pitseq == 0)
			index = rand() % npit;
		else
			index = nextpit;
		fprintf(outfp, "%s", pitches[index]);
		nextpit = (nextpit + 1) % npit;
	}

	fputs(" }, { ", outfp);

	for (i = 0; i < n; i++)
	{
		if (i != 0)
			fputs(", ", outfp);
		double t = p[i] == '.' ? dot : dash;
		fprintf(outfp, "%g", t);
		ttotal += t;
	}
	fprintf(outfp, " }, %ld", vollevel);
	if (params != NULL)
		fprintf(outfp, ", %s;\n", params);
	else
		fputs(";\n", outfp);
}


// Write the sequence.
// Output is in the form:
// seq "Text"
// begin
//   { pitches }, { rhythms }, volume, params ;
// end
//
// Each notelist line represents one character. Spaces result in a rest.
// 

void writeseq(char *cp, char *name)
{
	int newword = 1;

	while (isspace(*cp))
		cp++;

	if (name == NULL)
		name = cp;
	fprintf(outfp, "seq \"%s\"\nbegin\n", name);

	ttotal = 0;
	while (*cp)
	{
		int c = toupper(*cp);
		if (isspace(c))
		{
			if (!newword)
			{
				fprintf(outfp, "\tR, %g;\n", wrest);
				newword = 1;
				ttotal += wrest;
			}
		}
		else
		{
			if (!newword)
			{
				fprintf(outfp, "\tR, %g;\n", lrest);
				ttotal += lrest;
			}
			newword = 0;
			if (isalpha(c))
			{
				writemorse(morseltr[c - 'A'].thebits);
			}
			else if (isdigit(c))
			{
				writemorse(morsenum[c - '0'].thebits);
			}
			else if (ispunct(c))
			{
				for (int j = 0; morsepunct[j].letter != 0; j++)
				{
					if (morsepunct[j].letter == c)
					{
						writemorse(morsepunct[j].thebits);
						break;
					}
				}
			}
		}

		cp++;
	}
	fprintf(outfp, "! total=%g secs.\nend\n\n", ttotal);
}

// Display command line usage for invalid arguments.
void usage()
{
	fprintf(stderr, "use: morsemuzak -utime -dtime -wtime -ltime -vn -ppitches -s{r|c} -iinfile -ooutfile\n");
	fprintf(stderr, "     -utime = time of one unit\n");
	fprintf(stderr, "     -dtime = time of one dot (default = unit x 2, dash = dot x 2)\n");
	fprintf(stderr, "     -ltime = time between letters (default = unit x 3)\n");
	fprintf(stderr, "     -wtime = time between words (default = unit x 7)\n");
	fprintf(stderr, "     -ppitches = file containing pitches, one per line (max 256)\n");
	fprintf(stderr, "     -s{r|c} = pitch handling (r = random selection, c = cycle)\n");
	fprintf(stderr, "     -vn = volume level (default = 10000)\n");
	fprintf(stderr, "     -as = parameters\n");
	fprintf(stderr, "     -iinfile = input file for text (default = stdin)\n");
	fprintf(stderr, "     -ooutfile = output file (default = stdout)\n");

	exit(1);
}

// Program entry. will return 0 to shell on success, 1 on errors
int main(int argc, char* argv[])
{
	// Initial values. 1/10 second typical.
	unit = 0.1;
	dot = unit * 2;
	dash = unit * 4;
	lrest = unit * 3;
	wrest = unit * 7;
	vollevel = 100;

	infp = stdin;
	outfp = stdout;
	initpitch();

	// Process commandline arguments.
	for (int ndx = 1; ndx < argc; ndx++)
	{
		char *ap = argv[ndx];
		if (*ap == '-')
		{
			ap++;
			switch (*ap++)
			{
			case 'i':
				if ((infp = fopen(ap, "r")) == NULL)
				{
					perror(argv[2]);
					exit(1);
				}
				break;
			case 'o':
				if ((outfp = fopen(ap, "w")) == NULL)
				{
					perror(argv[3]);
					exit(1);
				}
				break;
			case 'u':
				unit = atof(ap);
				dot = unit * 2;
				dash = unit * 4;
				lrest = unit * 3;
				wrest = unit * 7;
				break;
			case 'd':
				dot = atof(ap);
				dash = dot * 2;
				break;
			case 'l':
				lrest = atof(ap);
				break;
			case 'w':
				wrest = atof(ap);
				break;
			case 'p':
				loadpitch(ap);
				break;
			case 's':
				if (*ap == 'r')
					pitseq = 0;
				else if (*ap == 'c')
					pitseq = 1;
				else
					usage();
				break;
			case 'v':
				vollevel = atol(ap);
				break;
			case 'a':
				params = strdup(ap);
				break;
			default:
				usage();
			}
		}
		else
			usage();
	}

	char inpln[1024];
	char *cp;
	char *name = NULL;
	int cmd;

	// Process the input file
	while (fgets(inpln, 1023, infp) != NULL)
	{
		inpln[1023] = '\0';

		if ((cp = strchr(inpln, '\n')) != NULL)
			*cp = '\0';
		if ((cmd = tolower(inpln[0])) == 0)
			continue;

		cp = &inpln[1];
		while (isspace(*cp))
			cp++;
		switch (cmd)
		{
		default:
			cp = inpln;
			// Fall through...
		case 's':
			writeseq(cp, name);
			if (name != NULL)
				free(name);
			name = NULL;
			break;
		case 'n':
			if (name != NULL)
				free(name);
			name = strdup(cp);
			break;
		case 'p':
			if (*cp == '+')
				cp++;
			else
				clearpitch();
			scanpitch(cp);
			break;
		case 'u':
			unit = atof(cp);
			dot = unit * 2;
			dash = unit * 4;
			lrest = unit * 3;
			wrest = unit * 7;
			break;
		case 'a':
			if (params != NULL)
				free(params);
			params = strdup(cp);
			break;
		case 'v':
			vollevel = atol(cp);
			break;
		}
	}

	// not really needed, since the run-time lib will clean up,
	// but I like things clean.

	if (infp != stdin)
		fclose(infp);
	if (outfp != stdout)
		fclose(stdout);

	clearpitch();

	return 0;
}
