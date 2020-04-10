#include <mysql.h>
#include "public.h"

// 本地
// #define DB_USER_NAME   "root" 
// #define DB_USER_PASSWD "123"  
// #define DB_NAME        "mto10"

// 服务器
#define DB_USER_NAME   "u1750979" 
#define DB_USER_PASSWD "u1750979"  
#define DB_NAME        "hw_mto10_u1750979"

class MysqlDB{

private:
	MYSQL *mysql;
	MYSQL_ROW row;
	MYSQL_RES *res;
	string sql;

public:
    // 构造函数
    MysqlDB();
    // 析构函数
	~MysqlDB();
    // 初始化数据库中所有表(删除表中原有数据)
    void fresh_all_tables(char *db_user_name,char* db_passwd,char *db_name);
    // 用set_student.sql填充数据库
    void init_all_tables(char *db_user_name,char* db_passwd,char *db_name);
    // 连接数据库
    void connect(const char *username, const char *password, const char *database);
    
    // 查询对应学号,密码能否登录游戏
    int is_logable(const char* stuno,const char *passwd);
    // 写入一局游戏的结束记录
    void record_one_game(ser_info * si);

    // 查询接口
    void seek_res(param_info * pi);
};

