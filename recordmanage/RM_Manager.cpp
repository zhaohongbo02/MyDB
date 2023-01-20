#include "rm.h"

RM_Manager::RM_Manager(FileManager* fileManager, BufPageManager* bufManager){
    this->fm = fileManager;
    this->bpm = bufManager;
}

int RM_Manager::CreateFile(const char* fileName, int recordSize){
     // Create the file
    if(!fm->createFile(fileName))  return -1;
    // Open the file
    int fileID;
    if(!fm->openFile(fileName, fileID)) return -1;
    FileHeader fileHeader;
    fileHeader.firstFreePage = 0;
    fileHeader.pageNum = 0;
    fileHeader.recordSize = recordSize;
    fileHeader.recordNum = std::min(MAX_RECORD_NUM, (PAGE_SIZE - (int)sizeof(PageHeader)) / recordSize);
    // Write file header to file
    int index;
    BufType buf = bpm->getPage(fileID, 0, index);
    memcpy(buf, &fileHeader, sizeof(fileHeader));
    bpm->markDirty(index);
    // bufManager->writeBack(index);
    fm->closeFile(fileID);
    return 0;
}

int RM_Manager::DestroyFile(const char* fileName){
    return remove(fileName);
}

int RM_Manager::OpenFile(const char* fileName, int &fileID){
    return fm->openFile(fileName, fileID);
}

int RM_Manager::CloseFile(int fileID){
    bpm->close();
    return fm->closeFile(fileID);
}