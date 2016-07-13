/**
 * 	Author: David Newell
 * 	   
 * simulator.c
 * 
 * 	This program reads a memory trace and simulates a sequence of
 * 	virtual memory system operations, using a single level page table.
 *
 *			 usage: frames trace policy
 *				frames: number of frames to simulate in the page table
 *			 	 trace: name of file containing the memory trace input
 *			 		-v: enable verbose output mode (optional)
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PAGES 1024
#define LRU 0
#define LFU 1

struct pageInfo
{
	int frameNumber;
	unsigned long long mostRecentlyUsed;
	unsigned long long timesUsed;
};

/***** GLOBALS *****/
struct pageInfo g_pageTable[MAX_PAGES]; // I know this isn't quite like the
int *g_pageFrames = NULL;				// example given in the assignment 
int g_VERBOSE = 0;						// description, but dynamic allocation
unsigned int g_numberOfFrames = 0;		// occurs below in makeTable.
unsigned long long g_time = 0;			// Don't fret! :)

/***** FUNCTION PROTOTYPES *****/
int LRUFrameSelect();
int LFUFrameSelect();
void accessPage(int, int, int*);
void simIOandControl(int, const char*);
void makePageTable();
void freePageTable();

/**
 * LRUFrameSelect:
 * Selects a frame by Least Recently Updated algorithm.
 * Returns the frame number which was uses for the drop/store of the page
 */
int LRUFrameSelect()
{
	int i, LRUFrameNumber, LRUTime;

	for (i = 0; i < g_numberOfFrames; i++)
	{
		struct pageInfo *pageInfo = &g_pageTable[g_pageFrames[i]];
		if (i == 0 || pageInfo -> mostRecentlyUsed < LRUTime)
		{
			LRUFrameNumber = i;
			LRUTime = pageInfo -> mostRecentlyUsed;
		}
	}
	if (g_VERBOSE) {printf("   		(LRU: fr # %d, LRUTime: %d)", LRUFrameNumber+1, LRUTime);}
	return LRUFrameNumber;
}

/**
 * LFUFrameSelect:
 * Selects a frame based on LFU, defaults to FIFO page replacement on compulsory misses
 * Returns the frame number which was uses for the drop/store of the page
 */
int LFUFrameSelect()
{
	int i, LFUFrameNumber, LFUTimesUsed;

	for (i = 0; i < g_numberOfFrames; i++)
	{
		struct pageInfo *pageInfo = &g_pageTable[g_pageFrames[i]];
		if ( (pageInfo->timesUsed < LFUTimesUsed) || i == 0)
		{
			LFUFrameNumber = i;
			LFUTimesUsed = pageInfo->timesUsed;
		}
	}
	if (g_VERBOSE) {printf("   		(LFU: fr # %d, timesUsed: %d)", LFUFrameNumber+1, LFUTimesUsed);}
	return LFUFrameNumber;
}

/**
 * accessPage:
 * Handles page access - looks for empty frame to store page in,
 * and if found, stores page there. If not, calls a method based upon 
 * which policy the user has selected, and that method handles the 
 * process accordingly
 */
void accessPage(int page, int policy, int *faultCount)
{
	int i, pageFrame = -1;

	if (g_VERBOSE) {printf("\npage %d: access", page);}

	// tried to run program with a trace file outside of specifications. No joy.
	if (page >= MAX_PAGES)
	{
		fprintf(stderr, "Error: No page %d in current page file\n", page);
		exit(EXIT_FAILURE);
	}

	// Member/page exists in table, update struct
	if (g_pageTable[page].frameNumber != -1)
	{
			g_pageTable[page].timesUsed++;
			g_pageTable[page].mostRecentlyUsed = g_time;
			g_time++;
			return;
	}

	// Look for empty frame to store page in
	for (i = 0; i < g_numberOfFrames; i++)
	{
		if (g_pageFrames[i] == -1)
		{
			pageFrame = i;
			break;
		}
	}

	if (pageFrame == -1)
	{
	
		*faultCount += 1;	// Page fault occurred

		if (policy == LFU)	// Handle fault according to current policy
		{
			pageFrame = LFUFrameSelect();	
		} 
		else
		{
			pageFrame = LRUFrameSelect();
		}

		if(g_VERBOSE) { printf("\n     PAGE FAULT accessing %d\n       replaced frame %d of %d", page, pageFrame+1, g_numberOfFrames); }
	}

	if (g_pageFrames[pageFrame] != -1)
	{
		g_pageTable[g_pageFrames[pageFrame]].frameNumber = -1;
	}

	// Update global counters and return to handle next page, if any
	g_pageTable[page].frameNumber      = pageFrame;
	g_pageFrames[pageFrame]            = page;
	g_pageTable[page].mostRecentlyUsed = g_time++;
	g_pageTable[page].timesUsed++;
}

/**
 * simIOandControl:
 * Handles file I/O and runs the simulation line by line
 * on the trace file
 */
void simIOandControl(int policy, const char *trace_file)
{
	FILE *file;
	if ((file = fopen(trace_file, "r")))	
	{
		int faultCount = 0;
		char buffer[512];

		while (fgets(buffer, sizeof(buffer), file) != NULL)
		{
			if (buffer[0] != '\n')			// skip blank lines in trace file
			{
				int page = atoi(buffer);
				accessPage(page, policy, &faultCount);
			}

		}
	printf("\n\n %d page faults encountered during simulation \n\n", faultCount);
	} 
	else
	{
		fprintf(stderr, "\nFailed to open specified trace file: %s \n", trace_file);
		exit(EXIT_FAILURE); 
	}

}

/**
 * makePageTable:
 * Creates page table and initializes data struture members
 */
void makePageTable()
{
	// Allocate memory and initialize pageFrames
	g_pageFrames = (int*) malloc(g_numberOfFrames * sizeof(g_pageFrames[0]) );
	int i;
	for (i = 0; i < g_numberOfFrames; i++)
	{
		g_pageFrames[i] = -1;
	}

	// Initialize the table with all -1 frameNumber values
	for (i = 0; i < sizeof(g_pageTable) / sizeof(g_pageTable[0]); i++)
	{
		memset(&g_pageTable[i], 0, sizeof(g_pageTable[i]) );
		g_pageTable[i].frameNumber = -1;
	}
}

/**void makePageTable()
 * freePageTable:
 * Frees page table memory
 */
void freePageTable()
{
	free(g_pageFrames);
	return;
}


//***** MAIN *****//
int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		if ((argc == 5) && (strcmp(argv[4], "-v") == 0))
		{
			g_VERBOSE = 1;
			printf("\n..:: Output mode - Verbose ::..\n");
		}
		else 
		{
			printf("\nusage: %s frames trace policy  [-v]\n\n", argv[0]);
			printf("   frames: number of frames to simulate in the page table\n");
			printf("    trace: name of file containing the memory trace input\n");
			printf("   policy: the page replacement policy, either LRU or LFU.\n");
			printf("       -v: enable verbose output mode (optional)\n\n");
			fflush(stdout);
			return 1;
		}
	}

	// Process command line arguments and set globals/policy parameters
	g_numberOfFrames = atoi(argv[1]);
	int pageReplacePolicy;

	if (strcmp(argv[3], "LFU") == 0)
		{
			pageReplacePolicy = LFU;
		}
	else if (strcmp(argv[3], "LRU") == 0)
		{
			pageReplacePolicy = LRU;
		}
	else
		{
			printf("\nArgument 3 was: %s, must be either LRU or LFU", argv[3]);
			exit(EXIT_FAILURE);
		}

	makePageTable(); 
	simIOandControl(pageReplacePolicy, argv[2]);
	freePageTable();
	exit(EXIT_SUCCESS);
}
