#include "sm.h"
using namespace std;

int main(){
    FileManager *fm = new FileManager();
    BufPageManager *bpm = new BufPageManager(fm);
    RM_Manager *rm = new RM_Manager(fm, bpm);
    SM_Manager *sm = new SM_Manager(fm, bpm, rm);
    // sm->CreateDB("test_db");
    
    /*
    struct Attr{
    char attrName[MAX_NAME + 1];
    AttrType attrType;
    int attrLength;
    bool isPrimary;
    bool not_null;
};
    */
    int attrCount = 2;
    Attr attrs[2];
    strcpy(attrs[0].attrName, "Name");
    attrs[0].attrType = AttrType::STRING;
    attrs[0].attrLength = 10;
    attrs[0].isPrimary = true;
    attrs[0].not_null = true;
    strcpy(attrs[1].attrName, "Age");
    attrs[1].attrType = AttrType::INTEGER;
    attrs[1].isPrimary = false;
    attrs[1].not_null = false;

    sm->OpenDB("test_db");
    sm->CreateTable("person", 2, attrs);
    sm->CreateTable("person2", 2, attrs);
    sm->ShowTables();
    sm->DescTable("person");
    sm->CloseDB();
    return 0;
}