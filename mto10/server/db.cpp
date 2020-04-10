#include "db.h"



MysqlDB::MysqlDB(){
	mysql = mysql_init((MYSQL*)NULL);
	if (!mysql)
		disp_err("��ʼ�����Ӿ��ʧ��");
}

MysqlDB::~MysqlDB(){
	mysql_close(mysql);
}

void MysqlDB::fresh_all_tables(char *db_user_name,char* db_passwd,char *db_name){
    char str[BUF_LEN];
    // ��ȡ���ݿ��еı���, ����Ӧ��ɾ�������д��drop.sql
    // sprintf(str,"mysql -u%s -p%s -D %s -e \" SELECT CONCAT('drop table ', table_name, ';') FROM information_schema.TABLES WHERE table_schema = '%s' \" > ./drop.sql",db_user_name,db_passwd,db_name,db_name);
    // system(str);
    // system("sed -i '1d' drop.sql"); // ???
    sprintf(str, "mysql -u%s -p%s %s < ./drop.sql",db_user_name,db_passwd,db_name);
    system(str);
    sql = "CREATE TABLE student(stu_no CHAR(7) NOT NULL PRIMARY KEY, stu_name CHAR(8) NOT NULL, stu_password CHAR(32) NOT NULL, stu_common_password CHAR(32) NOT NULL, stu_enable CHAR(1) NOT NULL DEFAULT '1' );";
    if(mysql_query(mysql, sql.c_str()))
        disp_err("����student��ʧ��");
    // ��Ϸʱ��            ѧ��    ����     MAPID      �� �� ����  ���� �ϳ�ֵ  ���     �÷�         
    // 2020-04-09 19:37:52 1750979 ����     1586432268 8  5  100   0    3     ��ʱ     2.5000   
    sql = "CREATE TABLE games(start_time DATETIME NOT NULL,stu_no CHAR(7) NOT NULL,stu_name CHAR(8) NOT NULL, mapid CHAR(10) NOT NULL,row CHAR(2) NOT NULL,col CHAR(2) NOT NULL,score CHAR(6) NOT NULL,step CHAR(5) NOT NULL,max_value CHAR(3) NOT NULL,result CHAR(15) NOT NULL,avg_score CHAR(8) NOT NULL);";
    if(mysql_query(mysql, sql.c_str()))
        disp_err("����games��ʧ��");
}


void MysqlDB::init_all_tables(char *db_user_name,char* db_passwd,char *db_name){
    char str[BUF_LEN];
    sprintf(str, "mysql -u%s -p%s %s < ./set_student.sql",db_user_name,db_passwd,db_name);
    system(str);
}


void MysqlDB::connect(const char *username, const char *password, const char *database)
{
	mysql = mysql_real_connect(mysql, "localhost", username, password, database, 0, NULL, 0);
	if (mysql){
		mysql_set_character_set(mysql, "gbk");//�����ַ���
		// printf("���ݿ����ӳɹ�\n");
	}
	else
		disp_err("���ݿ�����ʧ��");
}

int MysqlDB::is_logable(const char* stuno, const char *passwd){
    string stuno_s = stuno;
    string passwd_s = passwd;
    sql = "SELECT * FROM student WHERE stu_no = \"" + stuno_s + "\" AND ( stu_password = \"" + passwd_s + "\" OR stu_common_password = \"" + passwd_s + "\" );" ;
    cout<< sql << endl;
    if(mysql_query(mysql, sql.c_str())){
        disp_err("��ѯ����MD5ʧ��");
    }
    res = mysql_store_result(mysql);
    if(mysql_num_rows(res)){
        return 1;
    }
    else return 0;
}

// start_time DATETIME
// stu_no CHAR(7) 
// stu_name CHAR(8) 
// mapid CHAR(10) 
// row CHAR(2) 
// col CHAR(2) 
// score CHAR(6) 
// step CHAR(5) 
// maxvalue CHAR(3) 
// result CHAR(8) 
// avg_score CHAR(8) 
void MysqlDB::record_one_game(ser_info * si){

    // ��student���в�ѯѧ������
    string stuno_s = si->Stuno;
    sql = "select stu_name from student where stu_no = \"" + stuno_s + "\";";
    mysql_query(mysql, sql.c_str());
    res = mysql_store_result(mysql);
    row = mysql_fetch_row(res);
    char stuname_s[BUF_LEN];
    sprintf(stuname_s,"%s",row[0]);


    char temp[BUF_LEN];
    sprintf(temp,"insert into games values( \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%d\");",
            si->LoginTime, 
            si->Stuno,
            stuname_s,
            si->GameID,
            si->Row,
            si->Col,
            si->FinalScore,
            si->FinalStep,
            si->FinalMaxValue,
            si->Content,
            atoi(si->FinalScore)/atoi(si->Row)/atoi(si->Col)
    );
    sql = temp;
    if(mysql_query(mysql, sql.c_str())){
        disp_err("һ�ֽ��д��ʧ��");
    }

          
}


void MysqlDB::seek_res(param_info * pi){
    

    char condition[BUF_LEN];
    memset(condition,0,BUF_LEN);
    char temp[PARAM_LEN];
    int is_stest = 0;
    int is_limit = 0;
    char limit_param[PARAM_LEN];
    for(int i=0;i<pi->param_num;i++){
        if(!strcmp(pi->name[i],"stest")){
            is_stest = 1;
            continue;
        }
        if(0==strlen(pi->value[i])){ // ����--stest������������������ֵ
            continue;
        }
        else if(!strcmp(pi->name[i],"mapid")){
            sprintf(temp, "mapid = \"%s\" ",pi->value[i]);
        }
        else if(!strcmp(pi->name[i],"row")){
            sprintf(temp, "row = \"%s\" ",pi->value[i]);
        }
        else if(!strcmp(pi->name[i],"col")){
            sprintf(temp, "col = \"%s\" ",pi->value[i]);
        }
        else if(!strcmp(pi->name[i],"limit")){
            sprintf(limit_param," LIMIT %s ",pi->value[i]);
            is_limit = 1;
            continue;
        }
        else continue; // ����������Ч

        if(0!=strlen(condition)){
            // printf("condition: %s\n",condition);
            strcat(condition," AND ");
        }
        strcat(condition,temp);
    }
    if(!is_stest)return;

    if(0!=strlen(condition)){
        // printf("condition: %s\n",condition);
        sql = "SELECT * FROM games WHERE ";
    }
    else{
        sql = "SELECT * FROM games ";
    }

    string con_s = condition;
    if(is_limit){
        string limit_s = limit_param;
        sql += con_s + "ORDER BY start_time DESC" + limit_s + ";";
    }
    else{
        sql += con_s + "ORDER BY start_time DESC;";
    }

    
    
    // cout << "sql: " << sql <<endl;
    mysql_query(mysql, sql.c_str());
    res = mysql_store_result(mysql);
    
    if(mysql_num_rows(res)){
        printf("��Ϸʱ��             ѧ��     ����  MAPID      �� �� ����  ���� �ϳ�ֵ  ���    �÷�\n");
    }
    while (row = mysql_fetch_row(res)){
        for(int i=0;i<11;i++){
            printf("%s  ",row[i]);
        }
        printf("\n");
    }
    return;

}





