#ifndef SM_H
#define SM_H

#include "../filesystem/fs.h"
#include "../recordmanage/rm.h"
#include <fstream>
#include <vector>
using namespace std;

#define MAX_TABLE_NUM 10
#define MAX_COLUMN 20
#define MAX_NAME 50

struct Attr{
    char attrName[MAX_NAME + 1];
    AttrType attrType;
    int attrLength;
    bool isPrimary = false;
    bool not_null = false;
    void copy(Attr* attr){
        strcpy(attrName, attr->attrName);
        attrType = attr->attrType;
        attrLength = attr->attrLength;
        isPrimary = attr->isPrimary;
        not_null = attr->not_null;
    }
};

struct AttrCatEntry{
    char relName[MAX_NAME + 1];
    char attrName[MAX_NAME + 1];
    int attrOffset;
    AttrType attrType;
    int attrLength;
    bool isPrimary;
    bool not_null;
    void readAtrr(ifstream &fr){
        fr >> relName >> attrName >> attrOffset;
        int type;
        fr >> type;
        attrType = (AttrType)type;
        fr >> attrLength >> isPrimary >> not_null;
    }

    void writeAttr(ofstream &fw) const{
        fw << relName << endl << attrName << endl
        << attrOffset << endl << int(attrType) << endl
        << attrLength << endl << isPrimary << endl
        << not_null << endl;
    }
};

struct RelCatEntry{
    char relName[MAX_NAME + 1];
    int tupletLength;
    int attrCount;
    AttrCatEntry attrs[MAX_COLUMN];
    int next = -1;
    void readRel(ifstream &fr){
        fr >> relName >> tupletLength >> attrCount;
        for(int i = 0; i < attrCount; i++)
            attrs[i].readAtrr(fr);
    }

    void writeRel(ofstream &fw) const{
        fw << relName << endl << tupletLength
        << endl << attrCount << endl;
        for(int i = 0; i < attrCount; i++)
            attrs[i].writeAttr(fw);
    }

};

class SM_Manager{
    private:
        FileManager *fm;
        BufPageManager *bpm;
        RM_Manager *rm;
        int emptyrel[MAX_TABLE_NUM];
        int tableNum;
        BufType StringToValue(AttrType attrType, string value);
    public:
        char dbName[MAX_NAME + 1];
        RelCatEntry rels[MAX_TABLE_NUM];
        int head;
        SM_Manager(FileManager *fileManager, BufPageManager *bufManager, RM_Manager *rmManager);
        ~SM_Manager();
        int CreateDB(const char* DBName);
        int OpenDB(const char* DBName);
        int CloseDB();
        int DropDB(const char* DBName);

        int CreateTable(const char* tableName, int attrCount, Attr *attrs);
        int DropTable(const char* tableName);
        int ShowTables();
        int DescTable(const char* tableName);
        bool hasTable(const char* tableName) const;

        int InsertRecs(const char* tableName, RM_FileHandle *rf, vector<string> data, vector<AttrType> types);
        int DeleteRecs(const char* tableName, RM_FileHandle *rf, string attrName, ComOp comop, string value);
        int UpdateRecs(const char* tableName, RM_FileHandle *rf, string attrName, string value, string attrName1, ComOp comop, string value1);
        int SelectRecs(const char* tableName, RM_FileHandle *rf, vector<string> columns, string attrName, ComOp comop, string value);

        int DropPrimaryKey(const char* tableName, const char* keyName);
        int AddPrimaryKey(const char* tableName, const char* keyName);
};

#endif