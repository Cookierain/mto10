#include "db.h"

MysqlDB gbl_db;

int main(int argc, char *argv[]){
    

    if(argc <= 1 || strcmp(argv[1],"--stest")){
        printf("usage:\n");
        printf("    ./mto10_read --stest                                             : ��ʾ���µ�10����Ϸ�Ľ��(�Ǿ���)\n");
        printf("    ./mto10_read --stest --limit 64                                  : ��ʾ���µ�64����Ϸ�Ľ��(�Ǿ���)\n");
        printf("    ./mto10_read --stest --mapid 12345                               : ��ʾ���µ�10��[mapid=12345/����ֵ����]��Ϸ�Ľ��(�Ǿ���)\n");
        printf("    ./mto10_read --stest --mapid 12345 --limit 32                    : ��ʾ���µ�32��[mapid=12345/����ֵ����]��Ϸ�Ľ��(�Ǿ���)\n");
        printf("    ./mto10_read --stest --mapid 12345 --row 8 --col 10              : ��ʾ���µ�10��[mapid=12345/��=8/��=10]��Ϸ�Ľ��(�Ǿ���)\n");
        printf("    ./mto10_read --stest --mapid 12345 --row 8 --col 10 --limit 16   : ��ʾ���µ�16��[mapid=12345/��=8/��=10]��Ϸ�Ľ��(�Ǿ���)\n");
        return 0;
    }

    param_info param_info;
    analy_param(&param_info,argv,argc);
    // disp_param(&param_info);

    char db_name		[BUF_LEN];
	char db_user_name	[BUF_LEN];
	char db_passwd		[BUF_LEN];
    strcpy(db_user_name ,   DB_USER_NAME   );
    strcpy(db_passwd ,      DB_USER_PASSWD );
    strcpy(db_name ,        DB_NAME        );
    gbl_db.connect(db_user_name,db_passwd,db_name);
    gbl_db.seek_res(&param_info);

}

