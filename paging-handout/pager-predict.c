/*
 * File: pager-predict.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 *  This file contains a predictive pageit
 *      implmentation.
 */

#include <stdio.h> 
#include <stdlib.h>
#include <limits.h>


#include "simulator.h"

#define MAXPREDICT 4

/*
General strategy will be to page in page 0 on first call and to use a tree of possible next pages based on current page. In most cases there is only one possible page next, some cases 2 and on particular case there are 3 options. If we restrict ourselves to 5 physical pages per process, we can achieve 100% accuracy because we will only need to have 4 pages paged in maximum for any process.

        ┌───┬───0───────┬───┬───┬───┐
        │   │   │       │   │   │   │
        │   │   1       │   │   │   │
        │  (5)  │      (1) (1) (3) (4)
        │   │   2       │   │   │   │
        │   │   │       │   │   │   │
        │   │   3       │   │   │   │
        │   ├───┴─*─┐   │   │   │   │
        │   4  ┌────10  │   │   │   │   
       (2)  │  │    ├───┘   │   │   │
        │   5  │    11      │   │   │
        │   │  │    ├───────┘   │   │
        │   6  │    12          │   │
        │   │  │    │           │   │
        │   7  │    13          │   │
        │   │  │ ┌──┼───────────┘   │
        │   8  │ │  14              │
        └───┤  │(3) └───────────────┘
            9─*┼─┘
            └──┘

    Legend:
     X : page number
    (X): process number/kind

     ┌─input
     X─input
     └─output
     * eager load next page because branch lands into end of page

 */

void pageit(Pentry q[MAXPROCESSES]) {
    /* Local vars */
    int proc, pc, page, oldpage;

    /* What the assignment refers to as "magic" */
    static int after[15][MAXPREDICT] = {
        {1 ,-1,-1,-1}, //0  on 0 we can only go to 1
        {2 ,-1,-1,-1}, //1  on 1 we can only go to 2 
        {3 ,-1,-1,-1}, //2  on 2 we can only go to 3
        {0 , 4,10,11}, //3  on 3 we can go to 0, 4 and 10 but we also need to prefetch 11 because we will need only the end of 10
        {5 ,-1,-1,-1}, //4  on 4 we can only go to 5
        {6 ,-1,-1,-1}, //5  on 5 we can only go to 6
        {7 ,-1,-1,-1}, //6  on 6 we can only go to 7
        {8 ,-1,-1,-1}, //7  on 7 we can only go to 8
        {0 , 9,-1,-1}, //8  on 8 we can go to 0 and 9
        {10,-1,-1,-1}, //9  on 9 we can only go to 10
        {0 ,11,-1,-1}, //10 on 10 we can go to 0 and 11
        {0 ,12,-1,-1}, //11 on 11 we can go to 0 and 12
        {0, 9, 13,-1}, //12 on 12 we can go to 0, 9 and 13
        {0 ,9 ,10,14}, //13 on 13 we can go to 0, 9 and 14 (prefetch 10 with 9)
        {0 ,-1,-1,-1}, //14 on 14 we can go to 0
    };

    /* Select first active process */
    for (proc = 0; proc < MAXPROCESSES; proc++) {
        /* Is process active? */
        if (q[proc].active) {
            pc = q[proc].pc;            // program counter for process
            page = pc / PAGESIZE;       // page the program counter needs
            /* Is page swaped-out? */
            /* Truthfully, this only happens on the first cycle of a process, my prediction algorithm yields 100% accuracy */
            if (!q[proc].pages[page]) {
                /* Since I have PERFECT prediction and I clean up after myself I know I can safely assume swap in will work so there we go... */
                pagein(proc, page);
                
            }
            // More magic
            // Check what page we're in and page in all the next ones 
            // Also evict all the ones that aren't currently used 
            // or aren't the next required ones
            // Note: in some cases we eager load the pages after the next ones
            // which is looking 2 pages ahead of time
            for (int i = 0; i < MAXPREDICT && after[page][i] > -1; i++) {
                pagein(proc, after[page][i]);
            }
            // Roomba
            for (oldpage = 0; oldpage < q[proc].npages; oldpage++) {
                //if this old page is not the current one and none of the possible future pages, evict it
                if (oldpage != page 
                 && oldpage != after[page][0] 
                 && oldpage != after[page][1] 
                 && oldpage != after[page][2] 
                 && oldpage != after[page][3]) {
                    pageout(proc, oldpage);
                }
            }
        }
    }
}
