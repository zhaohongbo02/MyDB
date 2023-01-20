#include "rm.h"

void RM_FileScan::OpenScan(RM_FileHandle *fileHandle,
                        AttrType    attrType,
                        int         attrLength,
                        int         attrOffset,
                        ComOp       comop,
                        BufType     value){
    // cout << "Enter OpenScan" << endl;
    _curr_page = _curr_slot = 0;
    this->fileHandle = fileHandle;
    this->attrType = attrType;
    this->attrLength = attrLength;
    this->attrOffset = attrOffset;
    this->comop = comop;
    switch(attrType){
    case INTEGER:
        intValue = *((int *)value);
        break;
    case FLOAT:
        floatValue = *((float *)value);
        break;
    case STRING:
        stringValue = new char[attrLength];
        memcpy(stringValue, value, attrLength);
        break;
    }
}

bool RM_FileScan::RecComOp(BufType data) const{
    int intdata = 0;
    float floatdata = 0;
    int cmp = 0;
    if(comop == NO)  return true;
    switch(attrType){
    case INTEGER:
        intdata = *((int *)data);
        if(comop == EQ) return intdata == intValue;
        if(comop == NE) return intdata != intValue; 
        if(comop == LT) return intdata < intValue;
        if(comop == GT) return intdata > intValue;
        if(comop == LE) return intdata <= intValue;
        if(comop == GE) return intdata >= intValue;
        break;
    case FLOAT:
        floatdata = *((float *)data);
        if(comop == EQ) return floatdata == floatValue;
        if(comop == NE) return floatdata != floatValue; 
        if(comop == LT) return floatdata < floatValue;
        if(comop == GT) return floatdata > floatValue;
        if(comop == LE) return floatdata <= floatValue;
        if(comop == GE) return floatdata >= floatValue;
        break;
    case STRING:
        cmp = strcmp((char *)data, stringValue);
        if(comop == EQ)    return cmp == 0;
        if(comop == NE)    return cmp != 0;
        if(comop == LT)    return cmp < 0;
        if(comop == GT)    return cmp > 0;
        if(comop == LE)    return cmp <= 0;
        if(comop == GE)    return cmp >= 0;
        break;
    }
    return true;
}

int RM_FileScan::GetNextRec(RM_Record &rec){
    int pageNum = fileHandle->fileHeader.pageNum;
    int recordNum = fileHandle->fileHeader.recordNum;
    int recordSize = fileHandle->fileHeader.recordSize;
    while(1){
        if(_curr_slot >= recordNum){
            _curr_page ++;
            _curr_slot = 0;
        }
        if(_curr_page > pageNum)    return -1;
        int index;
        BufType buf = fileHandle->bpm->getPage(fileHandle->fileID, _curr_page, index);
        PageHeader *pageHeader = (PageHeader *)buf;
        uchar *bitmap = pageHeader->bitmap;
        if(fileHandle->GetBit(bitmap, _curr_slot) == 1u){    // there is a rec in (_curr_page, _curr_slot)
            // cout << _curr_page << ' ' << _curr_slot << endl;
            uint8_t *offset = (uint8_t *)buf + sizeof(PageHeader) + _curr_slot * recordSize;
            uint8_t *attr = offset + attrOffset;
            if(RecComOp((BufType)attr)){
                RID rid(_curr_page, _curr_slot);
                memcpy(rec.data, offset, recordSize);
                rec.rid = rid;
                _curr_slot ++;
                return 0;
            }
            else{
                _curr_slot++;
            }
        }else{
            _curr_slot ++;
        } 
    }
}