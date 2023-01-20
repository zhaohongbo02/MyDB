#ifndef RM_H
#define RM_H
#include "../filesystem/fs.h"
#include "../MyDB.h"
#include <string>
#include <cstring>
#include <stdlib.h>

#define MAX_RECORD_NUM 512
#define BITMAP_SIZE 512

struct FileHeader{
    int firstFreePage;
    int pageNum;
    int recordSize;
    int recordNum;
};

struct PageHeader{
    int nextFreePage;
    int recordNum;
    uchar bitmap[BITMAP_SIZE];
};

struct RID{
    int pageID;
    int slotID;
    RID(int page = 0, int slot = 0): pageID(page), slotID(slot){};
};

class RM_Record{
    public:
        RID rid;
        BufType data;
        int size;
    public:
        RM_Record(){data = NULL; size = 0; }
        RM_Record(RID rid, const int size);
        RM_Record(RID rid, const BufType pdata, const int size);
        RM_Record(RM_Record& rec);
        BufType GetData() const;
        RID GetRid() const;
};

class RM_FileHandle{     
    public:
        FileManager *fm;
        BufPageManager *bpm;
        FileHeader fileHeader;
        int fileID;
        RM_FileHandle(FileManager *fileManager, BufPageManager *bufManager, int fileID);
        int GetRec(const RID &rid, RM_Record &rec) const;
        int InsertRec(const BufType pData, RID &rid);
        int DeleteRec(const RID &rid);
        int UpdateRec(const RM_Record &rec);
        // int ForcePages();
        uint GetBit(uchar *bitmap, int soltIndex) const;
        void SetBit(uchar *bitmap, int soltIndex, uint bit);
        int GetFreeSolt(uchar *bitmap);
};

class RM_Manager{
    private:
        FileManager* fm;
        BufPageManager* bpm;
    public:
        RM_Manager(FileManager* fileManager, BufPageManager* bufPageManager);
        int CreateFile(const char *fileName, int recordsize);
        int DestroyFile(const char *fileName);
        int OpenFile(const char* fileName, int &fileID);
        int CloseFile(int fileID);
};

class RM_FileScan{
    private:
        RM_FileHandle *fileHandle;
        AttrType attrType;
        int attrLength;
        int attrOffset;
        ComOp comop;
        int intValue;
        float floatValue;
        char* stringValue;
        bool RecComOp(BufType data) const;
        int _curr_page;
        int _curr_slot;
    public:
        void OpenScan   (RM_FileHandle *fileHandle,
                        AttrType    attrType,
                        int         attrLength,
                        int         attrOffset,
                        ComOp       comop,
                        BufType     value);
        int GetNextRec(RM_Record &rec);
};
#endif
