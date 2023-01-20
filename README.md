# MyDB（一个简单的数据库管理系统）
---
## 运行依赖
### Linux操作系统
### g++(需支持c++17的编译)
### ANTLR4 Runtime Library
需提前安装`antlr4`的`c++运行时库`，并且在[Makefile](Makefile)中修改`INCLUDEANDLIB`对应的位置。
## 编译并运行源程序
在`MyDB`文件夹下运行以下命令：
```
make
./main
```
## 使用示例
``` sql
-- 创建数据库
CREATE DATABASE TEST;
-- 查看数据库
SHOW DATABASES;
-- 使用数据库
USE TEST;
-- 创建表格
CREATE TABLE STUDENT (S_ID INT PRIMARY KEY, NAME VARCHAR(50), AGE INT, HEIGHT FLOAT);
-- 查看数据库中的表格
SHOW TABLES;
-- 显示数据表的详细信息
DESC STUDENT;
-- 向表格中插入数据
INSERT INTO STUDENT VALUES (0, 'ZHANG SAN', 18, 170.0), (1, 'LI SI', 19, 165.0), (2, 'XIAO MING', 20, 185.5);
-- 删除表格中的数据
DELETE FROM STUDENT WHERE S_ID = 0;
-- 更新表格中的数据
UPDATE STUDENT SET NAME = 'WANG WU' WHERE NAME = 'LI SI';
-- 查询数据
SELECT * FROM STUDENT;
SELECT * FROM STUDENT WHERE HEIGHT > 170;
-- 删除主键
ALTER TABLE STUDENT DROP PRIMARY KEY (S_ID);
-- 设置主键
ALTER TABLE STUDENT ADD CONSTRAINT PRIMARY KEY (S_ID);
-- 删除表格
DROP TABLE STUDENT;
-- 删除数据库
DROP DATABASE TEST;
-- 退出程序
QUIT;
```

