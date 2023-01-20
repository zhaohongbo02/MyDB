#pragma once

#include "SQLBaseVisitor.h"
#include "../filesystem/fs.h"
#include "../recordmanage/rm.h"
#include "../sysmanage/sm.h"
#include <cstring>
#include <vector>
#include <time.h>
using namespace std;

class MyVisitor: public SQLBaseVisitor{
private:
    FileManager *fm;
    BufPageManager *bpm;
    SM_Manager *sm;
    RM_Manager *rm;
    vector<string> dbs;
    int dbNum;
    string curr_db = "";
    string curr_tb = "";
    vector<Attr> attrInfo;
    // for insert record
    vector<string> data;
    vector<AttrType> types;
    // for where operation
    string attrName = "";
    ComOp comop = ComOp::NO;
    string expr = "";
    bool where = false;
    // for update
    bool set = false;
    string attrName1;
    string svalue;
    // for select
    vector<string> selects;

    vector<string>::iterator finddb(string dbName){
        // if db is exists, return true, else return false
        vector<string>::iterator it = dbs.begin();
        while(it != dbs.end()){
            if(*it == dbName) return it;
            it++;
        }
        return dbs.end();
    }
    void WriteBack(){
        if(curr_db != "")
            sm->CloseDB();
        ofstream fout("META", ios_base::out);
        fout << dbNum << endl;
        vector<string>::iterator it = dbs.begin();
        while(it != dbs.end()){
            fout << *it << endl;
            it++;
        }
        fout.close();
    }
public:
    MyVisitor(){
        fm = new FileManager();
        bpm = new BufPageManager(fm);
        rm = new RM_Manager(fm, bpm);
        sm = new SM_Manager(fm, bpm, rm);
        ifstream fin("META");
        if(fin.is_open() == false) {
            dbNum = 0;
            dbs.clear();
            ofstream fout("META");
            fout << 0 << endl;
            fout.close();
        }
        else{
            fin >> dbNum;
            string dbName;
            for (int i = 0; i < dbNum; i++){
                fin >> dbName;
                dbs.push_back(dbName);
            }
            fin.close();
        }
    }

    ~MyVisitor() = default;

    void writeBack(){ WriteBack(); }

    std::any visitCreate_db(SQLParser::Create_dbContext *ctx) override{
        string dbName = ctx->Identifier()->getText();
        if(curr_db != ""){
            sm->CloseDB();
        }
        if(finddb(dbName) != dbs.end()){
            cout << "Database already exists!" << endl;
            return -1;
        }
        dbNum ++;
        dbs.push_back(dbName);
        int rc = sm->CreateDB(dbName.c_str());
        if(rc == 0){
            cout << "Create database successfully!" << endl;
        }
        return 0;
    }
    std::any visitDrop_db(SQLParser::Drop_dbContext *ctx) override{
        string dbName = ctx->Identifier()->getText();
        if(finddb(dbName) == dbs.end()){
            cout << "There are not " << dbName << "!" << endl;
            return -1;
        }
        dbs.erase(finddb(dbName));
        dbNum--;
        if(curr_db != "")
            sm->CloseDB();
        int rc = sm->DropDB(dbName.c_str());
        if(rc == 0){
            cout << "Drop database successfully!" << endl;
        }
        return 0;
    }

    std::any visitShow_dbs(SQLParser::Show_dbsContext *ctx) override{
        vector<string>::iterator it = dbs.begin();
        int len = strlen("All Databases");
        while(it != dbs.end()){
            // cout << *it << endl;
            len = std::max(len, (int)(*it).size());
            it++;
        }
        cout << '+';
        for(int i = 0; i < len; i++)    cout << '-';
        cout << '+' << endl;
        cout << "|All Databases";
        for(int i = 0; i < len - (int)strlen("All Databases"); i++)  cout << ' ';
        cout << '|' << endl;
        cout << '+';
        for(int i = 0; i < len; i++)    cout << '-';
        cout << '+' << endl;
        it = dbs.begin();
        while(it != dbs.end()){
            cout << '|' << *it;
            for(int i = 0; i < len - (int)(*it).size(); i++)    cout << ' ';
            cout << '|' << endl;
            it++;
        }
        cout << '+';
        for(int i = 0; i < len; i++)    cout << '-';
        cout << '+' << endl;
        cout << "Totally has " << dbNum << " databases" << endl;
        return 0;
    }

    std::any visitUse_db(SQLParser::Use_dbContext *ctx) override{
        string dbName = ctx->Identifier()->getText();
        if(finddb(dbName) == dbs.end()){
            cout << "There are no " << dbName << "!" << endl;
            return -1;
        }
        if(curr_db != ""){
            sm->CloseDB();
        }
        curr_db = dbName;
        int rc = sm->OpenDB(dbName.c_str());
        if (rc == 0){
            cout << "Set work data base to " << dbName << " successfully!" << endl;
        }
        return rc;
    }

    std::any visitShow_tables(SQLParser::Show_tablesContext *ctx) override{
        if(curr_db == ""){
            cout << "Please select a database with 'USE DATABASE_NAME'" << endl;
            return -1;
        }
        sm->ShowTables();
        return 0;
    }

    std::any visitCreate_table(SQLParser::Create_tableContext *ctx) override{
        if(curr_db == ""){
            cout << "Please select a database with 'USE DATABASE_NAME'" << endl;
            return -1;
        }
        string tableName = ctx->Identifier()->getText();
        attrInfo.clear();
        visitField_list(ctx->field_list());
        Attr attrs[MAX_COLUMN];
        for(int i = 0; i < attrInfo.size(); i++){
            attrs[i] = attrInfo.at(i);
        }
        sm->CreateTable(tableName.c_str(), attrInfo.size(), attrs);
        return 0;
    }

    std::any visitField_list(SQLParser::Field_listContext *ctx) override{
        // return a array of Attr
        for(auto f : ctx->field()){
            std::any res = f->accept(this);
        }
        return 0;

    }

    std::any visitNormal_field(SQLParser::Normal_fieldContext *ctx) override{
        // return a Attr
        string attrName = ctx->Identifier()->getText();
        Attr newAttr;
        strcpy(newAttr.attrName, attrName.c_str());
        string attrType = ctx->type_()->getText();
        if(attrType == "INT"){
            newAttr.attrType = AttrType::INTEGER;
        }
        else if(attrType == "FLOAT"){
            newAttr.attrType = AttrType::FLOAT;
        }
        else{
            newAttr.attrType = AttrType::STRING;
            string l = ctx->type_()->Integer()->getText();
            newAttr.attrLength = atoi(l.c_str());
        }
        if(ctx->Null()){
            newAttr.not_null = true;
        }
        attrInfo.push_back(newAttr);
        return 0;
    }

    std::any visitPrimary_key_field(SQLParser::Primary_key_fieldContext *ctx) override{
        string attrName = ctx->identifiers()->Identifier(0)->getText();
        // find attr and change isPrimary
        vector<Attr>::iterator it = attrInfo.begin();
        while(it != attrInfo.end()){
            if((*it).attrName == attrName)  break;
            it++;
        }
        if(it == attrInfo.end()){
            cout << "There are no " << attrName << " in this table." << endl;
            return 0;
        }
        (*it).isPrimary = true;
        return 0;
    }

    std::any visitDescribe_table(SQLParser::Describe_tableContext *ctx) override{
        string tableName = ctx->Identifier()->getText();
        if(curr_db == ""){
            cout << "Please select a database with 'USE DATABASENAME'" << endl;
            return -1;
        }
        sm->DescTable(tableName.c_str());
        return 0;
    }

    std::any visitDrop_table(SQLParser::Drop_tableContext *ctx) override{
        string tableName = ctx->Identifier()->getText();
        if(curr_db == ""){
            cout << "Please select a database with 'USE DATABASENAME'" << endl;
            return -1;
        }
        sm->DropTable(tableName.c_str());
        return 0;
    }

    std::any visitInsert_into_table(SQLParser::Insert_into_tableContext *ctx) override{
        string tableName = ctx->Identifier()->getText();
        if(curr_db == ""){
            cout << "Please select a database with 'USE DATABASENAME'" << endl;
            return -1;
        }
        if(sm->hasTable(tableName.c_str()) == false){
            cout << "There are no table " << tableName << " in this database" << endl;
            return -1;
        }
        curr_tb = tableName;
        clock_t start, stop;
        start = clock();
        int res = std::any_cast<int>(ctx->value_lists()->accept(this));
        stop = clock();
        double deltTime = (double)(stop - start) / CLOCKS_PER_SEC;
        if(res == 0)    cout << "Insert OK, sec(" << deltTime << ")" << endl;
        else    cout << "Insert fault" << endl;
        return 0;
    }

    std::any visitValue_lists(SQLParser::Value_listsContext *ctx) override{
        int fileID, rc = 0;
        rm->OpenFile(curr_tb.c_str(), fileID);
        RM_FileHandle rfh(fm, bpm, fileID);
        for(auto vl : ctx->value_list()){
            vl->accept(this);
            int res = sm->InsertRecs(curr_tb.c_str(), &rfh, data, types);
            if(res != 0){
                rc = res;
                break;
            }
        }
        rm->CloseFile(fileID);
        return rc;
    }

    std::any visitValue_list(SQLParser::Value_listContext *ctx) override{
        // update data and types
        data.clear();
        types.clear();
        for(auto v : ctx->value()){
            v->accept(this);
        }
        return 0;
    }

    std::any visitValue(SQLParser::ValueContext *ctx) override{
        if(where){
            if(ctx->Integer()){
                expr = ctx->Integer()->getText();
            }
            else if(ctx->Float()){
                expr = ctx->Float()->getText();
            }
            else if(ctx->String()){
                expr = ctx->String()->getText();
            }
            return 0;
        }
        else if(set){
            if(ctx->Integer()){
                svalue = ctx->Integer()->getText();
            }
            else if(ctx->Float()){
                svalue = ctx->Float()->getText();
            }
            else if(ctx->String()){
                svalue = ctx->String()->getText();
            }
            return 0;
        }

        if(ctx->Integer()){
            string v = ctx->Integer()->getText();
            // int intv = atoi(v.c_str());
            data.push_back(v);
            types.push_back(AttrType::INTEGER);
        }
        else if(ctx->Float()){
            string v = ctx->Float()->getText();
            // float floatv = (float)atof(v.c_str());
            data.push_back(v);
            types.push_back(AttrType::FLOAT);
        }
        else if(ctx->String()){
            string v = ctx->String()->getText();
            /*
            char *d = new char[v.size() + 1];
            memset(d, 0, sizeof(char) * (v.size() + 1));
            memcpy(d, v.c_str(), v.size());
            */
            data.push_back(v);
            types.push_back(AttrType::STRING);
        }
        else{
            return NULL;
        }
        return 0;
    }

    std::any visitDelete_from_table(SQLParser::Delete_from_tableContext *ctx) override{
        string tableName = ctx->Identifier()->getText();
        if(curr_db == ""){
            cout << "Please select a database with 'USE DATABASENAME'" << endl;
            return -1;
        }
        if(sm->hasTable(tableName.c_str()) == false){
            cout << "There are no table " << tableName << " in this database" << endl;
            return -1;
        }
        curr_tb = tableName;
        attrName = "";
        if(ctx->where_and_clause()){
            ctx->where_and_clause()->accept(this);
        }
        clock_t start, stop;
        start = clock();
        int fileID, rc = 0;
        rm->OpenFile(curr_tb.c_str(), fileID);
        RM_FileHandle rfh(fm, bpm, fileID);
        int res = sm->DeleteRecs(tableName.c_str(), &rfh, attrName, comop, expr);
        rm->CloseFile(fileID);
        stop = clock();
        double deltTime = (double)(stop - start) / CLOCKS_PER_SEC;
        if(res >= 0){
            cout << "Delete " << res << " records Sec(" << deltTime << ")" << endl;
        }
        else{
            cout << "Delete fault" << endl;
        }
        return 0;
    }

    std::any visitUpdate_table(SQLParser::Update_tableContext *ctx) override{
        string tableName = ctx->Identifier()->getText();
        if(curr_db == ""){
            cout << "Please select a database with 'USE DATABASENAME'" << endl;
            return -1;
        }
        if(sm->hasTable(tableName.c_str()) == false){
            cout << "There are no table " << tableName << " in this database" << endl;
            return -1;
        }
        curr_tb = tableName;
        ctx->set_clause()->accept(this);
        attrName = "";
        if(ctx->where_and_clause()){
            ctx->where_and_clause()->accept(this);
        }
        clock_t start, stop;
        start = clock();
        int fileID, rc = 0;
        rm->OpenFile(curr_tb.c_str(), fileID);
        RM_FileHandle rfh(fm, bpm, fileID);
        int res = sm->UpdateRecs(tableName.c_str(), &rfh, attrName1, svalue, attrName, comop, expr);
        rm->CloseFile(fileID);
        stop = clock();
        double deltTime = (double)(stop - start) / CLOCKS_PER_SEC;
        if(res >= 0){
            cout << "Update " << res << " records Sec(" << deltTime << ")" << endl; 
        }
        else{
            cout << "Updata fault" << endl;
        }
        return 0;
    }

    std::any visitWhere_and_clause(SQLParser::Where_and_clauseContext *ctx) override{
        return visitChildren(ctx);
    }

    std::any visitWhere_operator_expression(SQLParser::Where_operator_expressionContext *ctx) override{
        attrName = ctx->column()->Identifier(0)->getText();
        ctx->operator_()->accept(this);
        where = true;
        ctx->expression()->value()->accept(this);
        where = false;
        return 0;
    }

    std::any visitOperator_(SQLParser::Operator_Context *ctx) override{
        if(ctx->EqualOrAssign()){
            comop = ComOp::EQ;
        }
        else if(ctx->Less()){
            comop = ComOp::LT;
        }
        else if(ctx->LessEqual()){
            comop = ComOp::LE;
        }
        else if(ctx->Greater()){
            comop = ComOp::GT;
        }
        else if(ctx->GreaterEqual()){
            comop = ComOp::GE;
        }
        else if(ctx->NotEqual()){
            comop = ComOp::NE;
        }
        return 0;
    }

    std::any visitSet_clause(SQLParser::Set_clauseContext *ctx) override{
        attrName1 = ctx->Identifier(0)->getText();
        set = true;
        ctx->value(0)->accept(this);
        set = false;
        return 0;
    }

    std::any visitSelect_table(SQLParser::Select_tableContext* ctx) override{
        ctx->selectors()->accept(this);
        string tableName = ctx->identifiers()->Identifier(0)->getText();
        if(curr_db == ""){
            cout << "Please select a database with 'USE DATABASENAME'" << endl;
            return -1;
        }
        if(sm->hasTable(tableName.c_str()) == false){
            cout << "There are no table " << tableName << " in this database" << endl;
            return -1;
        }
        curr_tb = tableName;
        attrName = "";
        if(ctx->where_and_clause()){
            ctx->where_and_clause()->accept(this);
        }
        clock_t start, stop;
        start = clock();
        int fileID, rc = 0;
        rm->OpenFile(curr_tb.c_str(), fileID);
        RM_FileHandle rfh(fm, bpm, fileID);
        int res = sm->SelectRecs(tableName.c_str(), &rfh, selects, attrName, comop, expr);
        rm->CloseFile(fileID);
        stop = clock();
        double deltTime = (double)(stop - start) / CLOCKS_PER_SEC;
        if(res >= 0){
            cout << res << " rows in set (" << deltTime << ")" << endl;
        }
        else{
            cout << "Select fault" << endl;
        }
        return 0;
    }

    std::any visitSelectors(SQLParser::SelectorsContext *ctx) override{
        selects.clear();
        if(ctx->getText() == "*"){
            selects.push_back("*");
        }
        else{
            for(auto s : ctx->selector()){
                s->accept(this);
            }
        }
        return 0;
    }

    std::any visitSelector(SQLParser::SelectorContext *ctx) override{
        string attrName = ctx->column()->Identifier(0)->getText();
        selects.push_back(attrName);
        return 0;
    }

    std::any visitAlter_table_drop_pk(SQLParser::Alter_table_drop_pkContext *ctx) override{
        string tableName = ctx->Identifier(0)->getText();
        if(curr_db == ""){
            cout << "Please select a database with 'USE DATABASENAME'" << endl;
            return -1;
        }
        if(sm->hasTable(tableName.c_str()) == false){
            cout << "There are no table " << tableName << " in this database" << endl;
            return -1;
        }
        curr_tb = tableName;
        string keyName = "";
        if(ctx->Identifier(1)){
            keyName = ctx->Identifier(1)->getText();
        }
        // cout << tableName << keyName << endl;
        int res = sm->DropPrimaryKey(curr_tb.c_str(), keyName.c_str());
        if(res == 0){
            cout << "Drop PRIMARY KEY " << keyName << " from " << curr_tb << " successfully!" << endl;
        }
        return 0;
    }

    std::any visitAlter_table_add_pk(SQLParser::Alter_table_add_pkContext *ctx) override{
        string tableName = ctx->Identifier(0)->getText();
        if(curr_db == ""){
            cout << "Please select a database with 'USE DATABASENAME'" << endl;
            return -1;
        }
        if(sm->hasTable(tableName.c_str()) == false){
            cout << "There are no table " << tableName << " in this database" << endl;
            return -1;
        }
        curr_tb = tableName;
        string keyName = ctx->identifiers()->Identifier(0)->getText();
        int res = sm->AddPrimaryKey(curr_tb.c_str(), keyName.c_str());
        if(res == 0){
            cout << "ADD PRIMARY KEY " << keyName << " to " << curr_tb << " successfully!" << endl;
        }
        return 0;
    }
};