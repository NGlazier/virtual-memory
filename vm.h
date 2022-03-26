#include<iostream>
#include <string>
#include <string.h>
#include<fstream>
using namespace std;
#ifndef VM_H
#define VM_H


struct frame{
    bool dirty = 0;
    bool inuse = 0;
    int firstuse = -1;
    int lastuse = -1;
};

struct page{
    string type = "unmapped" ;
    bool ondisk = 0;
    int framenum = -1;
};

void dovm(bool fifo, string inputfile);
void doprint(frame farray[], page parray[]);
void getframe(bool fifo, bool write, int pnum, frame farray[], page parray[]);
void doline(bool fifo, bool write, string line, frame farray[], page parray[]);
void stealfifo(bool write, int pnum, frame farray[], page parray[]);
void steallru(bool write, int pnum, frame farray[], page parray[]);

#endif