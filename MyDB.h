#ifndef MY_DB_H
#define MY_DB_H

enum AttrType{
    INTEGER,
    FLOAT,
    STRING
};

enum ComOp{
    NO,
    EQ,
    NE,
    LT,
    GT,
    LE,
    GE,
    IS_NULL,
    IS_NOT_NULL
};

#endif