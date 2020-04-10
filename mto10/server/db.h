#include <mysql.h>
#include "public.h"

// ����
// #define DB_USER_NAME   "root" 
// #define DB_USER_PASSWD "123"  
// #define DB_NAME        "mto10"

// ������
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
    // ���캯��
    MysqlDB();
    // ��������
	~MysqlDB();
    // ��ʼ�����ݿ������б�(ɾ������ԭ������)
    void fresh_all_tables(char *db_user_name,char* db_passwd,char *db_name);
    // ��set_student.sql������ݿ�
    void init_all_tables(char *db_user_name,char* db_passwd,char *db_name);
    // �������ݿ�
    void connect(const char *username, const char *password, const char *database);
    
    // ��ѯ��Ӧѧ��,�����ܷ��¼��Ϸ
    int is_logable(const char* stuno,const char *passwd);
    // д��һ����Ϸ�Ľ�����¼
    void record_one_game(ser_info * si);

    // ��ѯ�ӿ�
    void seek_res(param_info * pi);
};

