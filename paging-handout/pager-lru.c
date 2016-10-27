/*
 * File: pager-lru.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains an lru pageit
 *      implmentation.
 */

#include <stdio.h> 
#include <stdlib.h>
#include <limits.h>

#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) {
	/* This file contains the stub for an LRU pager */
	/* You may need to add/remove/modify any part of this file */

	/* Static vars */
	static int initialized = 0;
	static int tick = 1; // artificial time
	static int timestamps[MAXPROCESSES][MAXPROCPAGES];

	/* Local vars */
	int proc;
	int pc;
	int page;
	int oldpage;

	/* initialize static vars on first run */
	if (!initialized) {
		for (proc = 0; proc < MAXPROCESSES; proc++) {
			for (page = 0; page < MAXPROCPAGES; page++) {
				timestamps[proc][page] = 0;
			}
		}
		initialized = 1;
	}

	/* Trivial paging strategy (aka piece of crap)*/
	/* Select first active process */
	for (proc = 0; proc < MAXPROCESSES; proc++) {
		/* Is process active? */
		if (q[proc].active) {
			/* Dedicate all work to first active process*/
			pc = q[proc].pc; 		        // program counter for process
			page = pc / PAGESIZE; 		// page the program counter needs
			/* Is page swaped-out? */
			if (!q[proc].pages[page]) {
				/* Try to swap in */
				if (!pagein(proc, page)) {
					/* If swapping fails, swap out least recently used page */
					int leastRecent = INT_MAX;
					int oldestPage = -1;
					for (oldpage = 0; oldpage < q[proc].npages; oldpage++) {
						/* If the page is another page, if it's older than current and if it's actually paged in */
						if (oldpage != page && timestamps[proc][oldpage] < leastRecent && q[proc].pages[oldpage]) {
							leastRecent = timestamps[proc][oldpage];
							oldestPage = oldpage;
						}
					}
					pageout(proc, oldestPage);
				} else {
					timestamps[proc][page] = tick;
				}
			}
		}
	}

	/* advance time for next pageit iteration */
	tick++;
}
