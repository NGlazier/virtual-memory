#include<iostream>
#include "vm.h"
#include <string>
#include <string.h>
#include<fstream>

using namespace std;
int debug = 0;
int usetime = 0; //used to track first and last use
int totalframes = 0;
int totalpages = 0;
int pagesreferenced = 0;
int pagesmapped = 0; //number of pages mapped
int pagesmissed = 0; //number of page misses
int framestaken = 0; //total number of frames taken from other pages
int frameswritten = 0; //total number of times a frame was written to disk
int framesrecovered = 0; //total number of times a frame was recovered from disk


int main(int argc, char *argv[]){
    if(argc == 3){
        if(strcmp(argv[1], "FIFO") != 0 && strcmp(argv[1], "LRU") != 0){
        cout << "Arugments should be of the form: FIFO|LRU inputfile.txt" << endl;
        exit(0);
        }
        string inputfile = argv[2];

        if(strcmp(argv[1], "FIFO") == 0){
            dovm(1, inputfile);
            //1 represents FIFO
        }
        if(strcmp(argv[1], "LRU") == 0){
            dovm(0, inputfile);
            //0 represents LRU
        }

    
    }else{
        cout << "Arugments should be of the form: FIFO|LRU inputfile.txt" << endl;
        exit(0);
    }
    return 0;
}




void dovm(bool fifo, string inputfile){
    if(debug){
        if(fifo){
            cout << "do fifo algorithm on " << inputfile << endl;
        }else{
            cout << "do lru algorithm on " << inputfile << endl;
        }}
    
    ifstream ifs;
    string line;
    //open file
    ifs.open(inputfile.c_str());
    if(!ifs.fail()){ 
    if(debug){
	    cout << "File opened succesfully" << endl;
    }
    
    //load total # of frames
    getline(ifs, line);
    if(debug){
        cout << "line: " << line << endl;
    }
    while(totalframes == 0){
        if(line[0] == '#'){
            //commented line. do nothing
        }else{
            totalframes = stoi(line);
            cout << "Num frames: " << totalframes << endl;
        }
    getline(ifs, line);
    }


    //load total # of pages
    while(totalpages == 0){
        if(line[0] == '#'){
            //commented line. do nothing
        }else{
            totalpages = stoi(line);
            cout << "Num pages: " << totalpages << endl;
            break;
        }
    getline(ifs, line);
    }

    //create array of frames and array of pages
    frame farray[totalframes];
    page parray[totalpages];
    if(fifo){
        cout << "Reclaim algorithm: FIFO" << endl;
    }else{
        cout << "Reclaim algorithm: LRU" << endl;
    }
    //read rest of file
    while(getline(ifs, line)){
        if(line[0] == '#'){
            //commented line. do nothing
        }else{
            if(line[0] == 'r'){
                
                if(debug){
                    cout << "line: " << line << endl;
                }
                line.erase(0,2);
                doline(fifo, 0 ,line, farray, parray);//0 represents read
                

            }else{
                if(line[0] == 'w'){
                   
                    if(debug){
                        cout << "line: " << line << endl;
                    }
                    line.erase(0,2);
                    doline(fifo, 1, line, farray, parray); //w represents write
                    
                }else{
                    if(line == "debug"){
                        debug = 1;
                    }else{
                        if(line == "nodebug"){
                            debug = 0;
                        }else{
                            if(line == "print"){
                                doprint(farray, parray);
                            }
                        }
                    }
                }
            }
            
        }
        
    }
    
	ifs.close();
    doprint(farray, parray); //print at end of file

    }else{
        cout << "ERROR: Input file could not be opened" << endl;
        exit(0);
    }
}


void doprint(frame farray[], page parray[]){
    cout << "FRAMES: " << endl;
    for(int i=0; i < totalframes; i++){
        cout << i << " inuse:" << farray[i].inuse << "  dirty:" << farray[i].dirty << " first_use:" << farray[i].firstuse << " last_use:" << farray[i].lastuse << endl;
    }
    cout << "PAGES: " << endl;
    for(int i=0; i < totalpages; i++){
        cout << i << " type: " << parray[i].type << "   ondisk:" << parray[i].ondisk <<  "   framenum:" ; 
        if(parray[i].framenum == -1){
            cout << "(unassigned)" << endl;
        }else{
            cout << parray[i].framenum << endl;
        }
    }
    cout << "Pages referenced: " << pagesreferenced << endl;
    cout << "Pages mapped: " << pagesmapped << endl;
    cout << "Page misses: " << pagesmissed << endl;
    cout << "Frames taken: " << framestaken << endl;
    cout << "Frames written to disk: " << frameswritten << endl;
    cout << "Frames recovered from disk: " << framesrecovered << endl;
}

void doline(bool fifo, bool write, string line, frame farray[], page parray[]){
  
    int pnum = stoi(line); //page number that is referenced
    //if the page is already mapped, we only need to update lastuse and if its a write, make frame dirty
    if(parray[pnum].type == "mapped"){
        if(debug){
            cout << "page " << pnum << " is already mapped" << endl;
        }
        farray[parray[pnum].framenum].lastuse = usetime;
        if(write == 1){
            farray[parray[pnum].framenum].dirty = 1;
        }
    }

    //if the page is unmapped, it will count as a page mapped, a page miss, and we must map to a frame
    if(parray[pnum].type == "unmapped"){
        if(debug){
            cout << "page " << pnum << " is unmapped" << endl;
        }
        pagesmapped++;
        pagesmissed++;
        getframe(fifo, write, pnum, farray, parray);
    }

    //if the page is taken it will count as a page miss and we must map to a frame
    if(parray[pnum].type == "taken"){
        if(debug){
            cout << "page " << pnum << " has been mapped then taken" << endl;
        }
        pagesmissed++;
        getframe(fifo, write, pnum, farray, parray);

    }
    //if the page is on disk, it will count as a frame recovered from disk.
    if(parray[pnum].ondisk == 1){
        framesrecovered++;
    }
    usetime++;//increase 'clock' time
    pagesreferenced++; 
}


void getframe(bool fifo, bool write, int pnum, frame farray[], page parray[]){
    //check for a frame thats not in use and map it to page
    for(int i=0; i < totalframes;i++){
        if(farray[i].inuse == 0){
            farray[i].inuse = 1;
            farray[i].lastuse = usetime;
            parray[pnum].type = "mapped";
            parray[pnum].framenum = i;
            farray[i].firstuse = usetime;
            if(debug){
            cout << "frame " << i << " is not in use" << endl;
            }
            if(write == 1){
                farray[i].dirty = 1;
            }
            return;
        }
    }
    
    //if all frames are in use, we must take one
    //we will take frames according to chosen algorithm
    framestaken++;
    if(debug){
        cout << "there are no frames available. one must be taken" << endl;
    }
    if(fifo){
        stealfifo(write, pnum, farray, parray);
    }else{
        steallru(write, pnum, farray, parray);
    }
}


void stealfifo(bool write, int pnum, frame farray[], page parray[]){
    //find the frame that was 'first in'
    int min = farray[0].firstuse;
    int minframe =0;
    for(int i =0; i < totalframes; i++){
        if(farray[i].firstuse < min){
            min = farray[i].firstuse;
            minframe = i;
        }
    }
    if(debug){
        cout << "frame with earliest first use time: " << minframe << endl;
    }
    
    //unmap last page and write it to disk if dirty.
    for(int i=0; i< totalpages; i++){
        if(parray[i].framenum == minframe){
            if(debug){
                cout << "page " << i << " was the last page to have this frame" << endl;
            }
            parray[i].framenum = -1;
            parray[i].type = "taken";
            if(farray[minframe].dirty == 1){
                parray[i].ondisk = 1;
                frameswritten++;
            }
            break;
        }
    }
    farray[minframe].dirty = 0;
    
    //map frame to new page referenced
    if(write == 1){
        if(debug){cout << "we are writing, making this frame dirty" << endl;}
        farray[minframe].dirty = 1;
    }
    farray[minframe].firstuse = usetime;
    farray[minframe].lastuse = usetime;
    parray[pnum].type = "mapped";
    parray[pnum].framenum = minframe;


    
}

void steallru(bool write, int pnum, frame farray[], page parray[]){
    //find frame that was least recently used
    int min = farray[0].lastuse;
    int minframe =0;
    for(int i =0; i < totalframes; i++){
        if(farray[i].lastuse < min){
            min = farray[i].lastuse;
            minframe = i;
        }
    }
    if(debug){
        cout << "frame with earliest last use time: " << minframe << endl;
    }
    
    //unmap last page and write it to disk if dirty.
    for(int i=0; i< totalpages; i++){
        if(parray[i].framenum == minframe){
            if(debug){
                cout << "page " << i << " was the last page to have this frame" << endl;
            }
            parray[i].framenum = -1;
            parray[i].type = "taken";
            if(farray[minframe].dirty == 1){
                parray[i].ondisk = 1;
                frameswritten++;
            }
            break;
        }
    }
    farray[minframe].dirty = 0;
    
    //map frame to new page referenced
    if(write == 1){
        if(debug){cout << "we are writing, making this frame dirty" << endl;}
        farray[minframe].dirty = 1;
    }
    farray[minframe].firstuse = usetime;
    farray[minframe].lastuse = usetime;
    parray[pnum].type = "mapped";
    parray[pnum].framenum = minframe;
}

