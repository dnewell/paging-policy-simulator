/**
 * simulator.c
 *
 * usage: simulator number_of_frames TRACE_FILE [LFU,LRU]
 *
 * This program reads a memory trace and simulates a
 * virtual memory system that uses a single level page table.
 *
 * Author: David Newell
 * SN: 250332100
 * For: Professor Hanan Lutfiyya
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PAGES 1024
#define LRU 0
#define LFU 1

struct pageRecord
{
	int frameNumber;
	unsigned long long mostRecentlyUsed;
	unsigned long long timesUsed;
};

struct pageRecord g_pageTable[MAX_PAGES]; //initialize structs
int *g_pageFrames = NULL;
int g_VERBOSE = 0;
unsigned int g_numberOfFrames = 0;
unsigned long long g_time = 0;

/**
 * @brief selects a frame based on LRU
 * @details note: TODO
 * @return the frame selected, or replaced
 */
int LRUFrameSelect()
{
	int i, LRUFrameNumber, LRUTime;

	for (i = 0; i < g_numberOfFrames; i++)
		{
			struct pageRecord *pageRecord = &g_pageTable[g_pageFrames[i]];
			if (i == 0 || pageRecord -> mostRecentlyUsed < LRUTime)
				{
					LRUFrameNumber = i;
					LRUTime = pageRecord -> mostRecentlyUsed;
				}
		}
	if (g_VERBOSE) {printf("   		(LRU: fr # %d, LRUTime: %d)", LRUFrameNumber+1, LRUTime);}
	return LRUFrameNumber;
}

/**
 * @brief selects a frame based on LFU
 * @details note: defaults to FIFO page replacement on compulsory misses
 * @return the frame selected, or replaced
 */
int LFUFrameSelect()
{
	int i, LFUFrameNumber, LFUTimesUsed;

	for (i = 0; i < g_numberOfFrames; i++)
		{
			struct pageRecord *pageRecord = &g_pageTable[g_pageFrames[i]];
			if (i == 0 || pageRecord->timesUsed < LFUTimesUsed)
				{
					LFUFrameNumber = i;
					LFUTimesUsed = pageRecord->timesUsed;
				}
		}
	if (g_VERBOSE) {printf("   		(LFU: fr # %d, timesUsed: %d)", LFUFrameNumber+1, LFUTimesUsed);}
	return LFUFrameNumber;
}

/**
 * @brief attempts to access a page
 * @details [long description]
 *
 * @param page [description]
 * @param policy [description]
 * @param faultCount [description]
 */
void accessPage(int page, int policy, int *faultCount)
{
	int i, pageFrame = -1;

	if (g_VERBOSE) {printf("\npage %d: access", page);}

	if (page >= MAX_PAGES)
		{
			fprintf(stderr, "Error: No page %d in current page file\n", page);
			exit(EXIT_FAILURE);
		}

	if (g_pageTable[page].frameNumber != -1)
		{
			goto update_usage; //I know, I know....
		}

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
		*faultCount += 1; //page fault

		if (policy == LRU)
			{
				pageFrame = LRUFrameSelect();
			}
		else
			{
				pageFrame = LFUFrameSelect();
			}
		if(g_VERBOSE) { printf("\n     PAGE FAULT accessing %d\n       replaced frame %d of %d", page, pageFrame+1, g_numberOfFrames); }
		}

	if (g_pageFrames[pageFrame] != -1)
		{
			g_pageTable[g_pageFrames[pageFrame]].frameNumber = -1;
		}

	g_pageTable[page].frameNumber = pageFrame;
	g_pageFrames[pageFrame] = page;

update_usage:
	g_pageTable[page].timesUsed++;
	g_pageTable[page].mostRecentlyUsed = g_time;

	g_time++;
}

/**
 * @brief handles file I/O
 * @details [long description]
 *
 * @param policy [description]
 * @param trace_file [description]
 */
void simulate(int policy, const char *trace_file)
{
	char buffer[256];
	int faultCount = 0;

	FILE *file = fopen(trace_file, "r");
	
	if (file == NULL)
		{
			fprintf(stderr, "\nFailed to open trace file%s", trace_file);
			exit(EXIT_FAILURE); 
		}

	while (fgets(buffer, sizeof(buffer), file) != NULL)
		{
			if (buffer[0] != '\n')			// skip blank lines
			{
				int page = atoi(buffer);
				accessPage(page, policy, &faultCount);
			}

		}
	printf("\n\n %d page faults encountered during simulation \n\n", faultCount);
}

/**
 * @brief creates page table
 * @details [long description]
 */
void makePageTable()
{
	int i;
	for (i = 0; i < sizeof(g_pageTable) / sizeof(g_pageTable[0]); i++)
		{
			memset(&g_pageTable[i], 0, sizeof(g_pageTable[i]));
			g_pageTable[i].frameNumber = -1;
		}
	g_pageFrames = malloc(g_numberOfFrames * sizeof(g_pageFrames[0]));
	for (i = 0; i < g_numberOfFrames; i++)
		{
			g_pageFrames[i] = -1;
		}
}

/**
 * @brief frees page table memory
 * @details [long description]
 */
void freePageTable()
{
	free(g_pageFrames);
}


/*** MAIN **********************************************************************/
int main(int argc, char *argv[])
{
	int pageReplacePolicy;

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

	g_numberOfFrames = atoi(argv[1]);

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
			return -1;
		}

	makePageTable();
	simulate(pageReplacePolicy, argv[2]);
	freePageTable();

	exit(EXIT_SUCCESS);
}
