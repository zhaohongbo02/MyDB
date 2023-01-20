#include <iostream>
#include "rm.h"
using namespace std;

int testnum = 100;
int page[50000];
int slot[50000];
float data[50000];

int main(){
    FileManager *fm = new FileManager();
    BufPageManager *bfm = new BufPageManager(fm);
    RM_Manager *recmanager = new RM_Manager(fm, bfm);
    std::cout << recmanager->CreateFile("rmtest", sizeof(float)) << endl;
    int fileID;
    std::cout << recmanager->OpenFile("rmtest", fileID) << endl;
    RM_FileHandle *rmfh = new RM_FileHandle(fm, bfm, fileID);
    RID rid;
	for (int i = 1; i <= testnum; i++) {
		// int page, slot;
        RID recordID;
		float d = (float)i/100;
		rmfh->InsertRec((BufType)(&d), recordID);
		page[i] = recordID.pageID;
        slot[i] = recordID.slotID;
        data[i] = d;
        // if (i >= 497){
        //     float x497;
        //     rid.pagenum =page[497];
        //     rid.slotnum = slot[497];
        //     handle->GetRec(rid, (BufType)(&x497));
        //     // printf("current i %d #497: %f\n", i, x497);
        // }
        cout << i << endl;
	}
    
    for (int i = 1; i <= testnum; i++) {
		// int page, slot;
        RID recordID;
        float j;
        recordID.pageID = page[i];
        recordID.slotID = slot[i];
        if (i % 3 == 0){
            rmfh->DeleteRec(recordID);
        }
        if (i % 3 == 1){
            j = -(float)i/(float)200;
            RM_Record *newrec = new RM_Record(recordID, (BufType)(&j), sizeof(float));
        }
        cout << i << endl;
        // cout << recordID.pagenum << " " << recordID.slotnum << " " << i << " " << j << endl;
    }

    return 0;
}