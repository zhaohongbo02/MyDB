#include "rm.h"

RM_Record::RM_Record(RID rid, const int size){
    this->rid = rid;
    data = (BufType)new char[size];
}

RM_Record::RM_Record(RID rid, const BufType pdata, const int size){
    this->rid = rid;
    data = (BufType)new char[size];
    memcpy(data, pdata, size);
}

RM_Record::RM_Record(RM_Record& rec){
    if(data == NULL){
        data = (BufType)new char[rec.size];
    }
    this->rid = rec.rid;
    this->size = rec.size;
    memcpy(data, rec.GetData(), size);
}

BufType RM_Record::GetData() const{
    return data;
}

RID RM_Record::GetRid() const{
    return rid;
}