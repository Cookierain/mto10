#include "db.h"

MysqlDB gbl_db;

int main(int argc, char *argv[]){
    

    if(argc <= 1 || strcmp(argv[1],"--stest")){
        printf("usage:\n");
        printf("    ./mto10_read --stest                                             : 显示最新的10局游戏的结果(非竞赛)\n");
        printf("    ./mto10_read --stest --limit 64                                  : 显示最新的64局游戏的结果(非竞赛)\n");
        printf("    ./mto10_read --stest --mapid 12345                               : 显示最新的10局[mapid=12345/行列值任意]游戏的结果(非竞赛)\n");
        printf("    ./mto10_read --stest --mapid 12345 --limit 32                    : 显示最新的32局[mapid=12345/行列值任意]游戏的结果(非竞赛)\n");
        printf("    ./mto10_read --stest --mapid 12345 --row 8 --col 10              : 显示最新的10局[mapid=12345/行=8/列=10]游戏的结果(非竞赛)\n");
        printf("    ./mto10_read --stest --mapid 12345 --row 8 --col 10 --limit 16   : 显示最新的16局[mapid=12345/行=8/列=10]游戏的结果(非竞赛)\n");
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

