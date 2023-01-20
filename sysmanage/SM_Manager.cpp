#include "sm.h"
#include <sys/stat.h>
#include <sys/types.h>
using namespace std;

SM_Manager::SM_Manager(FileManager *fileManager, BufPageManager *bufManager, RM_Manager *rmManager){
    this->fm = fileManager;
    this->bpm = bufManager;
    this->rm = rmManager;
}

int SM_Manager::CreateDB(const char* DBName){
    int rc;
    rc = mkdir(DBName, S_IRWXU);
    chdir(DBName);
    ofstream out;
    out.open("META");
    out << 0 << endl;
    out.close();
    chdir("../");
    return rc;
}

int SM_Manager::OpenDB(const char* DBName){
    strcpy(dbName, DBName);
    chdir(DBName);
    ifstream fin("META");
    fin >> this->tableNum;
    for(int i = 0; i < tableNum; i++){
        rels[i].readRel(fin);
        rels[i].next = i + 1;
        emptyrel[i] = 1;
    }
    if(tableNum > 0){
        head = 0;
        rels[tableNum - 1].next = -1;
    }
    else    head = -1;
    fin.close();
    return 0;
}

int SM_Manager::CloseDB(){
    if(strcmp(dbName, "") == 0){
        return 0;
    }
    ofstream fout("META", ios_base::out);
    fout << tableNum << endl;
    int curr = head;
    while(curr != -1){
        rels[curr].writeRel(fout);
        curr = rels[curr].next;
    }
    strcpy(dbName, "");
    return chdir("../");
}

int SM_Manager::DropDB(const char* DBName){
    char s[1024];
    sprintf(s, "rm -r %s", DBName);
    return system(s);
}

int SM_Manager::CreateTable(const char* tableName, int attrCount, Attr *attrs){
    // search tablename, if it's exists, return -1
    int curr = head;
    while(curr != -1){
        if(strcmp(rels[curr].relName, tableName) == 0){
            cout << "Table already exists!" << endl;
            return -1;
        }
        curr = rels[curr].next;
    }
    int index = 0;
    if(head == -1){ // there are no table in current data base
        head = 0;
        index = 0;
    }
    else{
        curr = head;
        // find tail of rel list
        while(rels[curr].next != -1)    
            curr = rels[curr].next;
        // find first empty rel from left to right
        
        for(; index < MAX_TABLE_NUM; index++){
            if(emptyrel[index] == 0)
                break; 
        }
        rels[curr].next = index;
    }
    this->tableNum += 1;
    emptyrel[index] = 1;
    // write info to new RelCatEntry
    strcpy(rels[index].relName, tableName);
    rels[index].attrCount = attrCount;
    rels[index].tupletLength = 0;
    rels[index].next = -1;
    /*
    char relName[MAX_NAME + 1];
    char attrName[MAX_NAME + 1];
    int attrOffset;
    AttrType attrType;
    int attrLength;
    bool isPrimary;
    */
    for(int i = 0; i < attrCount; i++){
        strcpy(rels[index].attrs[i].relName, tableName);
        strcpy(rels[index].attrs[i].attrName, attrs[i].attrName);
        rels[index].attrs[i].attrOffset = rels[index].tupletLength;
        rels[index].attrs[i].attrType = attrs[i].attrType;
        rels[index].attrs[i].isPrimary = attrs[i].isPrimary;
        rels[index].attrs[i].not_null = attrs[i].not_null;
        // update tuple length
        if(attrs[i].attrType == AttrType::INTEGER || attrs[i].attrType == AttrType::FLOAT){
            rels[index].attrs[i].attrLength = 4;
            rels[index].tupletLength += 4;
        }
        else{
            rels[index].attrs[i].attrLength = attrs[i].attrLength;
            rels[index].tupletLength += attrs[i].attrLength;
        }
    }
    // create new file for table
    rm->CreateFile(tableName, rels[index].tupletLength);
    int fileID;
    rm->OpenFile(tableName, fileID);
    rm->CloseFile(fileID);  // write fileheader back to table file
    return 0;
}

int SM_Manager::DropTable(const char* tableName){
    int curr = head, last = head;
    int index = -1;
    while(curr != -1){
        if(strcmp(rels[curr].relName, tableName) == 0){
            index = curr;
            break;
        }
        last = curr;
        curr = rels[curr].next;
    }
    if(index == -1){
        cout << "There are not this Table!" << endl;
        return -1;
    }
    tableNum --;
    if(index == head){
        head = rels[head].next;
    }
    else{
        rels[last].next = rels[index].next;
    }
    rm->DestroyFile(tableName);
    return 0;
}

int SM_Manager::ShowTables(){
    int l_db = strlen(dbName) + 10;
    int len = l_db;
    int curr = head;
    while(curr != -1){
        len = std::max(len, (int)strlen(rels[curr].relName));
        curr = rels[curr].next;
    }
    cout << '+';
    for(int i = 0; i < len; i++)    cout << '-';
    cout << '+' << endl;
    int l = len - l_db;
    cout << '|' << "Tables_in_"<< dbName;
    for(int i = 0; i < l; i++)    cout << ' ';
    cout << '|' << endl;
    cout << '+';
    for(int i = 0; i < len; i++)    cout << '-';
    cout << '+' << endl;
    curr = head;
    while(curr != -1){
        int l = len - strlen(rels[curr].relName);
        cout << '|' << rels[curr].relName;
        for(int i = 0; i < l; i++)    cout << ' ';
        cout << '|' << endl;
        curr = rels[curr].next;
    }
    cout << '+';
    for(int i = 0; i < len; i++)    cout << '-';
    cout << '+' << endl;
    cout << tableNum << " tables in " << dbName << endl;
    return 0;
}

inline void print_(int count){
    for(int i = 0; i < count; i++)  cout << '-';
}

inline void printSpace(int count){
    for(int i = 0; i < count; i++)  cout << ' ';
}

int SM_Manager::DescTable(const char *tableName){
    int index = -1, curr = head;
    while(curr != -1){
        if(strcmp(rels[curr].relName, tableName) == 0){
            index = curr;
            break;
        }
        curr = rels[curr].next;
    }
    if(index == -1){
        cout << "There are not this Table!" << endl;
        return -1;
    }
    // find max len of attrName
    int maxAttrName = 5;
    int maxType = 4;
    int typelen = 0;
    for(int i = 0; i < rels[index].attrCount; i++){
        maxAttrName = std::max(maxAttrName, (int)strlen(rels[index].attrs[i].attrName));
        switch(rels[index].attrs[i].attrType){
        case INTEGER:
            break;
        case FLOAT:
            maxType = std::max(maxType, 5);
            break;
        case STRING:
            typelen = strlen("VARCHAR()") + strlen(to_string(rels[index].attrs[i].attrLength).c_str());
            maxType = std::max(maxType, typelen);
            break;
        default:
            break;
        }
    }
    cout << '+';    print_(maxAttrName);    cout << '+';    print_(maxType);    cout << '+';
    print_(strlen("PRIMARY"));  cout << '+';    print_(4);  cout << '+';    print_(7);  cout << '+' << endl;
    cout << "|Field";   printSpace(maxAttrName - 5);
    cout << "|Type";    printSpace(maxType - 4);
    cout << "|Key";     printSpace(strlen("PRIMARY") - 3);
    cout << "|Null|Default|" << endl;
    cout << '+';    print_(maxAttrName);    cout << '+';    print_(maxType);    cout << '+';
    print_(strlen("PRIMARY"));  cout << '+';    print_(4);  cout << '+';    print_(7);  cout << '+' << endl;
    RelCatEntry *rel = &rels[index];
    AttrCatEntry *attrs = rel->attrs;
    for(int i = 0; i < rel->attrCount; i++){
        cout << "|" << attrs[i].attrName;   printSpace(maxAttrName - strlen(attrs[i].attrName));
        switch(attrs[i].attrType){
        case INTEGER:
            cout << "|INT"; printSpace(maxType - 3);
            break;
        case FLOAT:
            cout << "|FLOAT";   printSpace(maxType - 5);
            break;
        case STRING:
            cout << "|VARCHAR(" << attrs[i].attrLength << ")";
            typelen = strlen("VARCHAR()") + strlen(to_string(attrs[i].attrLength).c_str());
            printSpace(maxType - typelen);
            break;
        }
        if(attrs[i].isPrimary)  cout << "|PRIMARY";
        else cout << "|       ";
        if(attrs[i].not_null)   cout << "|NO  ";
        else    cout << "|YES ";
        cout << "|NULL   |" << endl;
    }
    cout << '+';    print_(maxAttrName);    cout << '+';    print_(maxType);    cout << '+';
    print_(strlen("PRIMARY"));  cout << '+';    print_(4);  cout << '+';    print_(7);  cout << '+' << endl;
    return 0;
}

bool SM_Manager::hasTable(const char* tableName) const{
    int curr = head;
    while(curr != -1){
        if(strcmp(rels[curr].relName, tableName) == 0)
            return true;
        curr = rels[curr].next;
    }
    return false;
}

BufType SM_Manager::StringToValue(AttrType attrType, string value){
    // convert value with string type to value with BufType
    int* intv = new int;
    float* floatv = new float;
    char *d = new char[value.size() + 1];
    BufType comvalue;
    switch(attrType){
    case AttrType::INTEGER:
        *intv = atoi(value.c_str());
        comvalue = (BufType)intv;
        break;
    case AttrType::FLOAT:
        *floatv = (float)atof(value.c_str());
        comvalue = (BufType)floatv;
        break;
    case AttrType::STRING:
        memset(d, 0, sizeof(char) * (value.size() + 1));
        memcpy(d, value.c_str(), value.size());
        comvalue = (BufType)d;
        break;
    }
    return comvalue;
}

int SM_Manager::InsertRecs(const char* tableName, RM_FileHandle *rfh, vector<string> data, vector<AttrType> types){
    int index = -1, curr = head;
    while(curr != -1){
        if(strcmp(rels[curr].relName, tableName) == 0){
            index = curr;
            break;
        }
        curr = rels[curr].next;
    }
    if(types.size() != rels[index].attrCount){
        cout << "Numbers of attr dose not match column of the record!" << endl;
        return -1;
    }
    vector<BufType> datas;
    for (int i = 0; i < rels[index].attrCount; i++){
        if(rels[index].attrs[i].attrType != types.at(i)){
            cout << "Types of data do not match!" << endl;
            return -1;
        }
        datas.push_back(StringToValue(types[i], data[i]));
    }
    
    // check unique of primary key
    for(int i = 0; i < rels[index].attrCount; i++){
        if(rels[index].attrs[i].isPrimary){
            // find attr in table
            RM_FileScan rfs;
            rfs.OpenScan(rfh, rels[index].attrs[i].attrType,
                            rels[index].attrs[i].attrLength, 
                            rels[index].attrs[i].attrOffset, 
                            ComOp::EQ,
                            datas[i]);
            RID rid;
            RM_Record rec(rid, rels[index].tupletLength);
            int find = rfs.GetNextRec(rec);
            if(find == 0){
                cout << rels[index].attrs[i].attrName << " PRIMARY KEY conflicts!" << endl;
                return -2;
            }
        }
    }
    // create a record to insert
    uint8_t* rec = new uint8_t[rels[index].tupletLength];
    memset(rec, 0, rels[index].tupletLength);
    for (int i = 0; i < rels[index].attrCount; i++){
        if(rels[index].attrs[i].attrType == AttrType::STRING){
            int attrLength = rels[index].attrs[i].attrLength;
            char* stringdata = new char[attrLength];
            int l = std::min((int)strlen((const char*)datas[i]), attrLength);
            // stringdata[l] = 0;
            memcpy(stringdata, (const char*)datas[i], l);
            memcpy(rec + rels[index].attrs[i].attrOffset, stringdata, l);
            delete [] stringdata;
        }
        else {
            uint8_t* numdata = (uint8_t*)datas[i];
            memcpy(rec + rels[index].attrs[i].attrOffset, numdata, 4);
        }
    }
    // insert data
    RID rid;
    rfh->InsertRec((BufType)rec, rid);
    return 0;
}

int SM_Manager::DeleteRecs(const char* tableName, RM_FileHandle *rf, string attrName, ComOp comop, string value){
    // find table
    int ti = -1, curr = head;
    while(curr != -1){
        if(strcmp(rels[curr].relName, tableName) == 0){
            ti = curr;
            break;
        }
        curr = rels[curr].next;
    }
    RM_FileScan rfs;
    if(attrName == ""){
        // cout << "attrName == ''" << endl;
        int *v = new int; *v = 0;
        rfs.OpenScan(rf, AttrType::INTEGER, 4, 0, ComOp::NO, (BufType)v);
    }
    else{
        int ai = -1;
        for(int i = 0; i < rels[ti].attrCount; i++){
            if(strcmp(rels[ti].attrs[i].attrName, attrName.c_str()) == 0){
                ai = i;
                break;
            }
        }
        if(ai == -1){
            cout << "There are not column called " << attrName << " in " << tableName << endl;
            return -1;
        }

        BufType comvalue = StringToValue(rels[ti].attrs[ai].attrType, value);
        
        rfs.OpenScan(rf, rels[ti].attrs[ai].attrType,
                        rels[ti].attrs[ai].attrLength,
                        rels[ti].attrs[ai].attrOffset, 
                        comop,
                        comvalue);
    }
    RID rid;
    RM_Record rec(rid, rels[ti].tupletLength);
    int delNum = 0;
    int find = rfs.GetNextRec(rec);
    while(find != -1){
        // find a record
        rf->DeleteRec(rec.rid);
        delNum ++;
        find = rfs.GetNextRec(rec);
    }
    return delNum;
}
int SM_Manager::UpdateRecs(const char* tableName, RM_FileHandle *rf, string attrName1, string value, string attrName, ComOp comop, string value1){
    // find table
    int ti = -1, curr = head;
    while(curr != -1){
        if(strcmp(rels[curr].relName, tableName) == 0){
            ti = curr;
            break;
        }
        curr = rels[curr].next;
    }
    // find attr1
    int ai1 = -1;
    for(int i = 0; i < rels[ti].attrCount; i++){
        if(strcmp(rels[ti].attrs[i].attrName, attrName1.c_str()) == 0){
            ai1 = i;
            break;
        }
    }
    if(ai1 == -1){
        cout << "There are not column called " << attrName1 << " in " << tableName << endl;
        return -1;
    }
    BufType setvalue = StringToValue(rels[ti].attrs[ai1].attrType, value);
    RM_FileScan rfs;

    if(attrName == ""){
        // cout << "attrName == ''" << endl;
        int *v = new int; *v = 0;
        rfs.OpenScan(rf, AttrType::INTEGER, 4, 0, ComOp::NO, (BufType)v);
    }
    else{
        int ai = -1;
        for(int i = 0; i < rels[ti].attrCount; i++){
            if(strcmp(rels[ti].attrs[i].attrName, attrName.c_str()) == 0){
                ai = i;
                break;
            }
        }
        if(ai == -1){
            cout << "There are not column called " << attrName << " in " << tableName << endl;
            return -1;
        }

        BufType comvalue = StringToValue(rels[ti].attrs[ai].attrType, value1);
        
        rfs.OpenScan(rf, rels[ti].attrs[ai].attrType,
                        rels[ti].attrs[ai].attrLength,
                        rels[ti].attrs[ai].attrOffset, 
                        comop,
                        comvalue);
    }
    RID rid;
    RM_Record rec(rid, rels[ti].tupletLength);
    int updateNum = 0;
    int find = rfs.GetNextRec(rec);
    while(find != -1){
        // find a record and change rec
        uint8_t* begin = (uint8_t *)rec.data;
        if(rels[ti].attrs[ai1].attrType == AttrType::STRING){
            int attrLength = rels[ti].attrs[ai1].attrLength;
            char* stringdata = new char[attrLength];
            int l = std::min((int)value.size(), attrLength);
            // stringdata[l] = 0;
            memcpy(stringdata, value.c_str(), l);
            memset(begin + rels[ti].attrs[ai1].attrOffset, 0, attrLength);
            memcpy(begin + rels[ti].attrs[ai1].attrOffset, stringdata, l);
            delete [] stringdata;
        }
        else{
            uint8_t* numdata = (uint8_t*)setvalue;
            memcpy(begin + rels[ti].attrs[ai1].attrOffset, numdata, 4);
        }
        rf->UpdateRec(rec);
        updateNum ++;
        find = rfs.GetNextRec(rec);
    }
    return updateNum;
}

int SM_Manager::SelectRecs(const char* tableName, RM_FileHandle *rf, vector<string> columns, string attrName, ComOp comop, string value){
    // find table
    // cout << "Begin Select" << endl;
    int ti = -1, curr = head;
    while(curr != -1){
        if(strcmp(rels[curr].relName, tableName) == 0){
            ti = curr;
            break;
        }
        curr = rels[curr].next;
    }

    int *attrIndex;
    int attrNum;
    if(columns.at(0) == "*"){
        // cout << "columns.at(0) == '*'" << endl;
        attrNum = rels[ti].attrCount;
        attrIndex = new int[attrNum];
        for(int i = 0; i < attrNum; i++){
            attrIndex[i] = i;
        }
    }
    else{
        attrNum = columns.size();
        attrIndex = new int[columns.size()];
        for(int i = 0; i < columns.size(); i++){
            int ai = -1;
            for(int i = 0; i < rels[ti].attrCount; i++){
                if(strcmp(rels[ti].attrs[i].attrName, columns.at(i).c_str()) == 0){
                    ai = i;
                    break;
                }
            }
            if(ai == -1){
                cout << "There are not column called " << columns.at(i) << " in " << tableName << endl;
                return -1;
            }
            attrIndex[i] = ai;
        }
    }
    RM_FileScan rfs;
    if(attrName == ""){
        // cout << "attrName == ''" << endl;
        int *v = new int; *v = 0;
        rfs.OpenScan(rf, AttrType::INTEGER, 4, 0, ComOp::NO, (BufType)v);
    }
    else{
        int ai = -1;
        for(int i = 0; i < rels[ti].attrCount; i++){
            if(strcmp(rels[ti].attrs[i].attrName, attrName.c_str()) == 0){
                ai = i;
                break;
            }
        }
        if(ai == -1){
            cout << "There are not column called " << attrName << " in " << tableName << endl;
            return -1;
        }

        BufType comvalue = StringToValue(rels[ti].attrs[ai].attrType, value);
        
        rfs.OpenScan(rf, rels[ti].attrs[ai].attrType,
                        rels[ti].attrs[ai].attrLength,
                        rels[ti].attrs[ai].attrOffset, 
                        comop,
                        comvalue);
    }
    
    RID rid;
    RM_Record rec(rid, rels[ti].tupletLength);
    int showNum = 0;
    int find = rfs.GetNextRec(rec);
    cout << "|"; 
    for(int i = 0; i < attrNum; i++){
        cout << " " << rels[ti].attrs[attrIndex[i]].attrName << " |";
    }
    cout << endl;
    while(find != -1){
        // find a record
        cout << "|";
        uint8_t* offset;
        for(int i = 0; i < attrNum; i++){
            AttrType type = rels[ti].attrs[attrIndex[i]].attrType;
            int length = rels[ti].attrs[attrIndex[i]].attrLength;
            offset = (uint8_t *)rec.data + rels[ti].attrs[attrIndex[i]].attrOffset;
            if(type == STRING){
                char *str = new char[length];
                memset(str, 0, length);
                memcpy(str, offset, length);
                cout << " " << str << " |";
            }
            else if(type == INTEGER){
                cout << " " << *((int *)offset) << " |";
            }
            else if(type == FLOAT){
                cout << " " << *((float *)offset) << " |";
            }
        }
        cout << endl;
        showNum ++;
        find = rfs.GetNextRec(rec);
    }
    return showNum;
}

int SM_Manager::DropPrimaryKey(const char* tableName, const char* keyName){
    // find table with tableName
    int ti = -1, curr = head;
    while(curr != -1){
        if(strcmp(rels[curr].relName, tableName) == 0){
            ti = curr;
            break;
        }
        curr = rels[curr].next;
    }
    // find primary key
    int ai = -1;
    if(strcmp(keyName, "") == 0){
        for(int i = 0; i < rels[ti].attrCount; i++){
            if(rels[ti].attrs[i].isPrimary){
                ai = i;
                break;
            }
        }
        if(ai == -1){
            cout << "There are not PRIMARY KEY in " << tableName << endl;
            return -1;
        }
    }
    else{
        for(int i = 0; i < rels[ti].attrCount; i++){
            if(strcmp(rels[ti].attrs[i].attrName, keyName) == 0 && rels[ti].attrs[i].isPrimary){
                ai = i;
                break;
            }
        }
        if(ai == -1){
            cout << "There are not PRIMARY KEY called " << keyName << " in " << tableName << endl;
            return -1;
        }
    }
    rels[ti].attrs[ai].isPrimary = false;
    string db = string(dbName);
    CloseDB();
    OpenDB(db.c_str());
    return 0;
}

int SM_Manager::AddPrimaryKey(const char* tableName, const char* keyName){
    // find table
    int ti = -1, curr = head;
    while(curr != -1){
        if(strcmp(rels[curr].relName, tableName) == 0){
            ti = curr;
            break;
        }
        curr = rels[curr].next;
    }
    // check if there is primary key already
    for(int i = 0; i < rels[ti].attrCount; i++){
        if(rels[ti].attrs[i].isPrimary){
            cout << "There already is a PRIMARY KEY named " << rels[ti].attrs[i].attrName << endl;
            return -1;
        }
    }
    // if there are not primary key, find attr with keyName
    int ai = -1;
    for(int i = 0; i < rels[ti].attrCount; i++){
        if(strcmp(rels[ti].attrs[i].attrName, keyName) == 0){
            ai = i;
            break;
        }
    }
    if(ai == -1){
        cout << "There are not column called " << keyName << " in " << tableName << endl;
        return -1;
    }
    rels[ti].attrs[ai].isPrimary = true;
    string db = string(dbName);
    CloseDB();
    OpenDB(db.c_str());
    return 0;
}