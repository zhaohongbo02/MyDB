#include "rm.h"

RM_FileHandle::RM_FileHandle(FileManager *fileManager, BufPageManager *bufManager, int fileID){
    this->fm = fileManager;
    this->bpm = bufManager;
    this->fileID = fileID;
    // load fileHeader
    int index;
    BufType buf = bpm->getPage(fileID, 0, index);
    FileHeader* headerpage = (FileHeader *)buf;
    this->fileHeader = *headerpage;
    bpm->access(index);
}

int RM_FileHandle::InsertRec(const BufType pData, RID &rid){
    // judge if have free page
    int index;
    if(fileHeader.firstFreePage == 0){  // there is not free page, create a new page
        // update file header
        int index;
        BufType buf = bpm->getPage(fileID, 0, index);
        fileHeader.pageNum ++;
        fileHeader.firstFreePage = fileHeader.pageNum;
        memcpy(buf, &fileHeader, sizeof(FileHeader));
        bpm->access(index);
        bpm->markDirty(index);
        // create a new page and write page header
        buf = bpm->getPage(fileID, fileHeader.pageNum, index);
        PageHeader *ph = (PageHeader *)buf;
        ph->nextFreePage = 0;
        ph->recordNum = 0;
        // set bitmap of new page
        for(int i = 0; i < fileHeader.recordNum; i++)
            ph->bitmap[i] = 0;
        bpm->access(index);
        bpm->markDirty(index);
    }
    // write data to page
    int pageID = fileHeader.firstFreePage;
    BufType buf = bpm->getPage(fileID, pageID, index);
    PageHeader *ph = (PageHeader *)buf;
    uchar *bitmap = ph->bitmap;
    int slotID = GetFreeSolt(bitmap);
    rid.pageID = pageID;
    rid.slotID = slotID;
    uint8_t *offset = (uint8_t *)buf + sizeof(PageHeader) + slotID * fileHeader.recordSize;
    memcpy(offset, pData, fileHeader.recordSize);
    SetBit(bitmap, slotID, 1u);
    ph->recordNum += 1;
    bpm->access(index);
    bpm->markDirty(index);
    // bpm->writeBack(index);
    if(ph->recordNum == fileHeader.recordNum){  // this page is full, create a new page
        // update file header
        int fhindex;
        BufType buf = bpm->getPage(fileID, 0, fhindex);
        fileHeader.firstFreePage = ph->nextFreePage;
        memcpy(buf, &fileHeader, sizeof(FileHeader));
        bpm->access(fhindex);
        bpm->markDirty(fhindex);
    }
    return 0;
}

int RM_FileHandle::DeleteRec(const RID &rid){
    if(rid.pageID > fileHeader.pageNum) return -1;
    int index;
    BufType buf = bpm->getPage(fileID, rid.pageID, index);
    PageHeader *ph = (PageHeader *)buf;
    uchar *bitmap = ph->bitmap;
    if(GetBit(bitmap, rid.slotID) == 0) return -1;  // slot is free
    ph->recordNum --;
    SetBit(bitmap, rid.slotID, 0);
    bpm->access(index);
    bpm->markDirty(index);
    // bpm->writeBack(index);
    if(ph->recordNum == fileHeader.recordNum - 1){
        // if the num of records of current page is max record num - 1, update file header
        BufType buf = bpm->getPage(fileID, 0, index);
        ph->nextFreePage = fileHeader.firstFreePage;
        fileHeader.firstFreePage = rid.pageID;
        memcpy(buf, &fileHeader, sizeof(FileHeader));
        bpm->access(index);
        bpm->markDirty(index);
        // bpm->writeBack(index);
    }
    return 0;

}

int RM_FileHandle::UpdateRec(const RM_Record &rec){
    RID rid = rec.GetRid();
    BufType data = rec.GetData();
    if(rid.pageID > fileHeader.pageNum) return -1;
    int pageIndex;
    BufType buf = bpm->getPage(fileID, rid.pageID, pageIndex);
    PageHeader* ph = (PageHeader *)buf;
    uchar *bitmap = ph->bitmap;
    if(GetBit(bitmap, rid.slotID) == 0) return -1;
    uint8_t *offset = (uint8_t *)buf + sizeof(PageHeader) + rid.slotID * fileHeader.recordSize;
    memcpy(offset, data, fileHeader.recordSize);
    bpm->access(pageIndex);
    bpm->markDirty(pageIndex);
    // bpm->writeBack(pageIndex);
    return 0;
}

int RM_FileHandle::GetRec(const RID &rid, RM_Record &rec) const{
    if(rid.pageID > fileHeader.pageNum) return -1;
    int index;
    BufType buf = bpm->getPage(fileID, rid.pageID, index);
    PageHeader *ph = (PageHeader *)buf;
    uchar *bitmap = ph->bitmap;
    if(GetBit(bitmap, rid.slotID) == 0) // slot is free
        return -1;
    uint8_t *offset = (uint8_t *)buf + sizeof(PageHeader) + rid.slotID * fileHeader.recordSize;
    rec.rid = rid;
    rec.size = fileHeader.recordSize;
    memcpy(rec.data, offset, fileHeader.recordSize);
    bpm->access(index);
    return 0;
}

uint RM_FileHandle::GetBit(uchar *bitmap, int soltID) const{
    uint statu = bitmap[soltID];
    return statu;
}

void RM_FileHandle::SetBit(uchar *bitmap, int soltID, uint bit){
    if(soltID < fileHeader.recordNum){
        bitmap[soltID] = bit;
    }
}

int RM_FileHandle::GetFreeSolt(uchar *bitmap){
    for(int i = 0; i < fileHeader.recordNum; i++){
        uint bit = bitmap[i];
        if(bit == 0)    return i;
    }
    return -1;
}