#include "MD5.h"
#include "db.h"

MysqlDB gbl_db;



// select每秒至少返回一次
void handler(int signo){
    alarm(1); 
}

// 总的信息结构体,包含各个玩家的收发缓冲区,游戏信息,socket
struct player{
    char rcv_buf[BUF_LEN];
    char snd_buf[BUF_LEN];
    ser_info si;
    cli_info ci;
    int  connect_fd;
};


// 寻找并清理未被使用的player结构体, 未找到-1
int find_not_used_player(player * player, char * is_player_online){
    for(int i=0;i<MAX_LINK;i++){
        if(!is_player_online[i]){
            memset(&player[i],0,sizeof(struct player));
            return i;
        }
    }
    return -1;
}

// 每次select前设置socket到fd_set中
void set_fdset(player * player, int listen_fd, char * is_player_online, fd_set & rd_fdset, fd_set & wr_fdset, int &max_fd){
    FD_ZERO(&rd_fdset);
    FD_ZERO(&wr_fdset);
    FD_SET(listen_fd,&rd_fdset);
    max_fd = listen_fd;
    for(int i=0;i<MAX_LINK;i++){
        if(is_player_online[i]){
            if(player[i].connect_fd > max_fd){
                max_fd = player[i].connect_fd;
            }
            FD_SET(player[i].connect_fd,&rd_fdset);
            if(0!=strlen(player[i].snd_buf)){
                FD_SET(player[i].connect_fd,&wr_fdset);
            }
        }
    }
}


// 生成新地图, row, col, mapid一致则地图一致
void gen_map(char * map, int row, int col, int mapid){
    srand(mapid);
    int i;
    for(i=0;i<row*col;i++){
        map[i] = rand()%3 + 1 + 48;
    }
    map[i]=0;
}


// 进行合成, 返回新增分数,新旧地图,最大值
int one_merge(int row_p, int col_p, int row, int col, char * oldmap, char * newmap, char *maxvalue, int mapid){
    
    list<int> checked;
    list<int> checking;
    int index = row_p*col + col_p;
    checking.push_back(index);
    
    while(!checking.empty()){
        int pos = checking.front();
        checking.pop_front();
        if(0==count(checked.begin(),checked.end(),pos)){
            checked.push_back(pos);
        }
        else{
            continue;
        }
        int new_pos = 0;
        if(pos/col!=0 && oldmap[pos - col] == oldmap[pos] ){ // 检查上
            new_pos = pos - col;
            checking.push_back(new_pos);
        }
        if(pos%col!=0 && oldmap[pos - 1] == oldmap[pos]){ // 检查左
            new_pos = pos - 1;
            checking.push_back(new_pos);
        }
        if(pos%col!=col-1 && oldmap[pos + 1] == oldmap[pos]){ // 检查右
            new_pos = pos + 1;
            checking.push_back(new_pos);
        }
        if(pos/col!=row-1 && oldmap[pos + col] == oldmap[pos]){
            new_pos = pos + col;
            checking.push_back(new_pos);
        }
        else ;
        
    }
    // 分数: 消除值*消除个数*3
    int score = (oldmap[index]<65?oldmap[index]-48:oldmap[index]-65+10)*checked.size()*3;
    
    // 消除, 位置用0填充
    strcpy(newmap,oldmap);
    list<int> :: iterator ii;
    int first = checked.front();
    newmap[first] = (newmap[first]=='9')?'A':newmap[first]+1;
    checked.pop_front();
    for(ii=checked.begin();ii!=checked.end();ii++){
        newmap[*ii] = '0'; 
    }
    
    // 下落,同时找最大值
    int new_maxvalue = '0';
    for(int i=row*col-1;i!=-1;i--){
        if(newmap[i]== '0'){ // 想上寻找寻找不为0的位置
            for(int up = i - col;up>=0;up-=col){
                if(newmap[up]!='0'){ // 下落
                    newmap[i] = newmap[up];
                    newmap[up] = '0';
                    break;
                }
            }
        }
        if(newmap[i]>new_maxvalue)new_maxvalue = newmap[i];
    }
    new_maxvalue = (new_maxvalue<'A'?new_maxvalue-'0':new_maxvalue-'A'+10);
    sprintf(maxvalue,"%d",new_maxvalue); // 十进制显示新的最值
    
    // 填充
    srand(mapid);
    for(int i=row*col-1;i!=-1;i--){
        if(newmap[i]=='0'){
            char t = rand()%new_maxvalue + 1;
            newmap[i] = t<10?t+'0':t+'A';
        }
    }
    return score;
}


// 判断游戏结束(分数<0,无可合成项,已到16,超时)
int is_game_over(ser_info* si){
    if(atoi(si->Score)<=0){
        return 1;
    }

    if(atoi(si->MaxValue)==16){
        return 1;
    }

    int mapsize = atoi(si->Row)*atoi(si->Col);
    for(int i=0;i<mapsize;i++){
        if(is_mergeable(i/atoi(si->Col),i%atoi(si->Col),atoi(si->Row),atoi(si->Col),si->NewMap)){
            return 0;
        }
    }
    return 1;
}

// 包解析函数
void unpack(player * player){
    char *name;
    char *value;
    char delim[] = " =\r\n"; 

    char rcv_t[BUF_LEN];
    strcpy(rcv_t,player->rcv_buf);
    char* rcv_p = rcv_t; 

    while(1){
        
        if(!strcmp(player->rcv_buf,""))break;
        name = strsep(&rcv_p,delim);
        while(!strcmp(name,"")){
            if(!strcmp(rcv_p,""))break;
            name = strsep(&rcv_p,delim);
        }

        if(!strcmp(rcv_p,""))break;
        value = strsep(&rcv_p,delim);
        while(!strcmp(value,"")){
            if(!strcmp(rcv_p,""))break;
            value = strsep(&rcv_p,delim);
        }

        if(!strcmp(name,"Type"))strcpy(player->ci.Type,value);
        else if(!strcmp(name,"Row"))strcpy(player->ci.Row   ,value);
        else if(!strcmp(name,"Col"))strcpy(player->ci.Col   ,value);
        else if(!strcmp(name,"MD5"))strcpy(player->ci.MD5   ,value);
        else if(!strcmp(name,"Delay"))strcpy(player->ci.Delay ,value);
        else if(!strcmp(name,"GameID"))strcpy(player->ci.GameID,value);
        else if(!strcmp(name,"Length"))strcpy(player->ci.Length,value);
        else ;
    }
}

// 处理某个玩家的信息
void deal(player * player, char * is_player_online){
    if(0==strlen(player->ci.Type)){ // 开始登录
        strcpy(player->si.Type,"SecurityString");
        gen_hex_str(player->si.Content,MD5_LEN);
    }
    else if(!strcmp(player->ci.Type,"ParameterAuthenticate")){
        if(0!=strlen(player->si.Stuno)){ // 重复登陆
            return;
        }
        char decoded_str[BUF_LEN];
        hex_str2str(player->ci.MD5,decoded_str,MD5_LEN);
        encryption(decoded_str,player->si.Content,MD5_LEN); 
        
        // string s = ",KLJsv8|4,2b65Bg";
        // MD5 md5 = MD5(s);
        // printf("md5: %s\n",md5.hexdigest().c_str());

        // 此时decoded_str为学号 + '*' + 密码md5的形式(在数据库中查询)
        memcpy(player->si.Stuno,decoded_str,7);
        player->si.Stuno[7] = 0; 
        // if(!strcmp(&decoded_str[8],md5.hexdigest().c_str())){   // 密码匹配, 登陆成功初始化
        if(gbl_db.is_logable(player->si.Stuno,&decoded_str[8])){
            memcpy(player->si.Stuno,decoded_str,7);
            player->si.Stuno[7] = 0; 
            printf("登陆成功\n");
            strcpy(player->si.Type,"GameProgress");
            strcpy(player->si.Content,"StartGame");
            if(!strcmp(player->ci.Row,"-1")){
                sprintf(player->si.Row,"%d",5 + rand()%4);
            }
            else{ 
                sprintf(player->si.Row,"%u",(atoi(player->ci.Row)<5 || 8<atoi(player->ci.Row))?atoi(player->ci.Row)%4+5:atoi(player->ci.Row));
            }
            if(!strcmp(player->ci.Col,"-1")){
                sprintf(player->si.Col,"%d",5 + rand()%6);
            }
            else{ 
                sprintf(player->si.Col,"%u",(atoi(player->ci.Col)<5 || 10<atoi(player->ci.Col))?atoi(player->ci.Col)%6+5:atoi(player->ci.Col));
            }
            if(!strcmp(player->ci.GameID,"-1")){
                sprintf(player->si.GameID,"%u",(unsigned)rand());
            }
            else{
                strcpy(player->si.GameID,player->ci.GameID);
            }
            if(!strcmp(player->ci.Delay,"-1")){
                strcpy(player->si.Delay,"5000");
            }
            else{
                sprintf(player->si.Delay,"%u",(atoi(player->ci.Delay)<2000 || atoi(player->ci.Delay)>60000)?atoi(player->ci.Delay)%59+2:atoi(player->ci.Delay));
            }
            gen_map(player->si.Map,atoi(player->si.Row),atoi(player->si.Col),atoi(player->si.GameID));
            strcpy(player->si.OldMap,player->si.Map);
            strcpy(player->si.NewMap,player->si.Map);
            strcpy(player->si.Step,"0");
            strcpy(player->si.Score,"100");
            strcpy(player->si.MaxValue,"3");
            sprintf(player->si.LoginTime,"%s",DateTime::currentTime().c_str());
        }
        else{ // 登陆失败
            *is_player_online = 0;
        }
    }
    else if(!strcmp(player->ci.Type,"Coordinate")){
    
        sprintf(player->si.Step,"%d",atoi(player->si.Step) + 1);
        if(
            (player->ci.Row[0]-'A')<0 || atoi(player->si.Row)<=(player->ci.Row[0]-'A') || 1!=strlen(player->ci.Row) ||
            (player->ci.Col[0]-'0')<0 || atoi(player->si.Col)<=(player->ci.Col[0]-'0') || 1!=strlen(player->ci.Col)
        ){    // 不存在的位置 
            strcpy(player->si.Content,"InvalidCoordinate");
            sprintf(player->si.Score,"%d",atoi(player->si.Score) - 50);
        }
              // 可合成位置
        else if(is_mergeable(player->ci.Row[0]-'A',player->ci.Col[0]-'0',atoi(player->si.Row),atoi(player->si.Col),player->si.NewMap)){
            
            strcpy(player->si.Content,"MergeSucceeded");
            disp_ser_info(&player->si);
            strcpy(player->si.OldMap,player->si.NewMap);
            int score_add = one_merge(player->ci.Row[0]-'A',
                player->ci.Col[0]-'0',
                atoi(player->si.Row),
                atoi(player->si.Col),
                player->si.OldMap,
                player->si.NewMap,
                player->si.MaxValue,
                atoi(player->si.GameID)
            );
            disp_map(&player->si);
            sprintf(player->si.Score,"%d",atoi(player->si.Score) + score_add);

        }
        else{ // 周围无可消除的位置
            strcpy(player->si.Content,"MergeFailed");
            sprintf(player->si.Score,"%d",atoi(player->si.Score) - 10);
        }

        // 判断是否game_over
        if(is_game_over(&player->si)){
            if(16 == atoi(player->si.MaxValue)){
                strcpy(player->si.Content,"GameFinished");
            }
            else{
                strcpy(player->si.Content,"GameOver");
            }
            strcpy(player->si.FinalStep,player->si.Step);
            strcpy(player->si.FinalScore,player->si.Score);
            strcpy(player->si.FinalMaxValue,player->si.MaxValue);
            strcpy(player->si.FinalMap,player->si.NewMap);
        }

    }
    else ;
}

// 包封装函数
void pack(player * player){
    if(!strcmp(player->si.Type,"SecurityString")){
        sprintf(player->snd_buf,"Type = %s\r\nContent = %s\r\nLength = ",player->si.Type,player->si.Content);
    }
    if(!strcmp(player->si.Type,"GameProgress")){
        if(!strcmp(player->si.Content,"StartGame")){
            sprintf(player->snd_buf,"Type = %s\r\nContent = %s\r\nRow = %s\r\nCol = %s\r\nGameID = %s\r\nDelay = %s\r\nMap = %s\r\nLength = ",player->si.Type,player->si.Content,player->si.Row,player->si.Col,player->si.GameID,player->si.Delay,player->si.Map);
        }
        else if(!strcmp(player->si.Content,"MergeSucceeded")){
            sprintf(player->snd_buf,"Type = %s\r\nContent = %s\r\nGameID = %s\r\nStep = %s\r\nScore = %s\r\nMaxValue = %s\r\nOldMap = %s\r\nNewMap = %s\r\nLength = ",player->si.Type,player->si.Content,player->si.GameID,player->si.Step,player->si.Score,player->si.MaxValue,player->si.OldMap,player->si.NewMap);
        }
        else if(!strcmp(player->si.Content,"MergeFailed")){
            sprintf(player->snd_buf,"Type = %s\r\nContent = %s\r\nGameID = %s\r\nStep = %s\r\nScore = %s\r\nMaxValue = %s\r\nLength = ",player->si.Type,player->si.Content,player->si.GameID,player->si.Step,player->si.Score,player->si.MaxValue);
        }
        else if(!strcmp(player->si.Content,"InvalidCoordinate")){
            sprintf(player->snd_buf,"Type = %s\r\nContent = %s\r\nGameID = %s\r\nStep = %s\r\nScore = %s\r\nMaxValue = %s\r\nLength = ",player->si.Type,player->si.Content,player->si.GameID,player->si.Step,player->si.Score,player->si.MaxValue);
        }
        else if(!strcmp(player->si.Content,"GameOver") || !strcmp(player->si.Content,"GameFinished")){
            sprintf(player->snd_buf,"Type = %s\r\nContent = %s\r\nGameID = %s\r\nFinalStep = %s\r\nFinalScore = %s\r\nFinalMaxValue = %s\r\nFinalMap = %s\r\nLength = ",player->si.Type,player->si.Content,player->si.GameID,player->si.FinalStep,player->si.FinalScore,player->si.FinalMaxValue,player->si.FinalMap);
        }
        else if(!strcmp(player->si.Content,"GameTimeout")){
            sprintf(player->snd_buf,"Type = %s\r\nContent = %s\r\nGameID = %s\r\nFinalStep = %s\r\nFinalScore = %s\r\nFinalMaxValue = %s\r\nFinalMap = %s\r\nLength = ",player->si.Type,player->si.Content,player->si.GameID,player->si.FinalStep,player->si.FinalScore,player->si.FinalMaxValue,player->si.FinalMap);
        }
        else;
    }
    

    // 计算并填充length
    int len = strlen(player->snd_buf) + 2; // 2: 最后的/r/n
    char str_length[BUF_LEN];
    sprintf(str_length,"%d",len);
    len += strlen(str_length);
    
    
    if(9<=len && len < 10) len += 1;
    if(98 <= len && len < 100) len += 1;
    if(997 <= len && len < 1000)len += 1 ;
    else ;    // 这里省略包长 > 9996的情况 
    sprintf(player->si.Length,"%d",len);
    sprintf(player->snd_buf + strlen(player->snd_buf),"%d\r\n",len);
}

// 显示命令行参数使用方式
void disp_usage(){
    printf("server usage:\n");
    printf("    ./mto10_server_d --port 12345  \n");
    printf("mto10_read usage:\n");
    printf("    ./mto10_read --stest                                             : 显示最新的10局游戏的结果(非竞赛)\n");
    printf("    ./mto10_read --stest --limit 64                                  : 显示最新的64局游戏的结果(非竞赛)\n");
    printf("    ./mto10_read --stest --mapid 12345                               : 显示最新的10局[mapid=12345/行列值任意]游戏的结果(非竞赛)\n");
    printf("    ./mto10_read --stest --mapid 12345 --limit 32                    : 显示最新的32局[mapid=12345/行列值任意]游戏的结果(非竞赛)\n");
    printf("    ./mto10_read --stest --mapid 12345 --row 8 --col 10              : 显示最新的10局[mapid=12345/行=8/列=10]游戏的结果(非竞赛)\n");
    printf("    ./mto10_read --stest --mapid 12345 --row 8 --col 10 --limit 16   : 显示最新的16局[mapid=12345/行=8/列=10]游戏的结果(非竞赛)\n");
} 


void turn_to_daemon(){
	if(-1 == daemon(0, 0))
		disp_err("daemon失败");//变守护进程失败，报错退出
}


int main(int argc, char *argv[]){

    if(argc <= 2 || strcmp(argv[1],"--port")){
        disp_usage();
        return 0;
    }
    else ;


    // 初始化数据库
    char db_name		[BUF_LEN];
	char db_user_name	[BUF_LEN];
	char db_passwd		[BUF_LEN];
    strcpy(db_user_name ,DB_USER_NAME      );
    strcpy(db_passwd ,   DB_USER_PASSWD    );
    strcpy(db_name ,     DB_NAME           );

    gbl_db.connect(db_user_name,db_passwd,db_name);
    // gbl_db.fresh_all_tables(db_user_name,db_passwd,db_name);
    // gbl_db.init_all_tables(db_user_name,db_passwd,db_name);
    
    turn_to_daemon();

    // 网络参数
    struct sockaddr_in servaddr, cliaddr; 
    socklen_t cliaddr_len=sizeof(cliaddr); 
    int listen_fd;
    player player[MAX_LINK];
    char is_player_online[MAX_LINK]; // 表示用户在线的标志位(0未使用, 1使用中)
    memset(player,0,MAX_LINK*sizeof(struct player));
    memset(is_player_online,0,MAX_LINK*sizeof(char));
    fd_set wr_fdset, rd_fdset;

    // 设置地址和端口
    servaddr.sin_family=AF_INET;                
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY); 
    servaddr.sin_port=htons(atoi(argv[2]));    

    // 设置监听socket设置成非阻塞
    listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
    int flags = fcntl(listen_fd,F_GETFL,0);
    fcntl(listen_fd,F_SETFL,flags | O_NONBLOCK);

    // socket绑定地址和端口
    bind(listen_fd,(struct sockaddr *)&servaddr, sizeof(servaddr));

    // 开始监听
    listen(listen_fd,MAX_LINK);

    // 每1s至少从select返回1次
    signal(SIGALRM,handler);
    alarm(1); 


    while(1){
        
        int max_fd;
        set_fdset(player,listen_fd,is_player_online,rd_fdset,wr_fdset,max_fd);

        int ret = select(max_fd+1,&rd_fdset,&wr_fdset,NULL,NULL);
        // if(ret<=0){
        //     if(ret == -1)continue;
        //     printf("ret = %d\n",ret);
        //     disp_err("select返回值<=0, 服务端异常关闭");
        // }

        if(ret > 0 && FD_ISSET(listen_fd,&rd_fdset)){
            int index;
            if(-1!=(index = find_not_used_player(player,is_player_online))){ 
                player[index].connect_fd = accept(listen_fd,(struct sockaddr *)&cliaddr, &cliaddr_len);
                is_player_online[index] = 1;

                // 设置非阻塞
                flags = fcntl(player[index].connect_fd,F_GETFL,0);      
                fcntl(player[index].connect_fd,F_SETFL,flags | O_NONBLOCK);

                 // 启用REUSEADDR，使端口释放后可再次使用
                int sock_opt;
	            setsockopt(player[index].connect_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&sock_opt, sizeof(sock_opt));

                // 生成该玩家初始ser_info并写入缓冲区
                deal(&player[index],&is_player_online[index]);
                pack(&player[index]);

            }
            else ; // 达到MAX_LINK没有可分配的结构体
        }
        for(int i=0;i<MAX_LINK;i++){
            if(!is_player_online[i]){
                continue;
            }
            else{
                if(ret > 0 && FD_ISSET(player[i].connect_fd,&rd_fdset)){
                    memset(player[i].rcv_buf,0,BUF_LEN);
                    int n = read(player[i].connect_fd,player[i].rcv_buf,BUF_LEN);
                    if(n <= 0){ // 多用户服务端不能关
                        disp_err("玩家主动断开连接\n");
                        close(player[i].connect_fd);
                    }
                    printf("read byte num: %d\ncontent: \n%s\n",n,player[i].rcv_buf);
                    unpack(&player[i]);
                    deal(&player[i],&is_player_online[i]);
                    if(!is_player_online[i]){
                        printf("玩家登陆失败,断开连接\n");
                        close(player[i].connect_fd);
                    }
                    pack(&player[i]);
                }
                else if(ret > 0 && FD_ISSET(player[i].connect_fd,&wr_fdset)){
                    int m = write(player[i].connect_fd,player[i].snd_buf,atoi(player[i].si.Length));
                    printf("write byte num: %d\ncontent: \n%s\n",m,player[i].snd_buf);
                    memset(player[i].snd_buf,0,BUF_LEN); 
                    // player[i].si.LastTalkTime = time(NULL);
                    sprintf(player[i].si.LastTalkTime,"%s",DateTime::currentTime().c_str());

                    // 游戏结束 关闭socket 结果写入数据库 释放player结构体
                    if(!strcmp(player[i].si.Content,"GameOver") || 
                       !strcmp(player[i].si.Content,"GameFinished") || 
                       !strcmp(player[i].si.Content,"GameTimeout")
                    ){
                        gbl_db.record_one_game(&player[i].si);
                        close(player[i].connect_fd);
                        is_player_online[i] = 0;
                    }
                }
                else if(0 != strlen(player[i].si.Delay) && 
                        atoi(player[i].si.Delay)/1000 < time(NULL)-DateTime::convert<time_t>(player[i].si.LastTalkTime) 
                ){ // 超时
                    strcpy(player[i].si.Content,"GameTimeout");
                    strcpy(player->si.FinalStep,player->si.Step);
                    strcpy(player->si.FinalScore,player->si.Score);
                    strcpy(player->si.FinalMaxValue,player->si.MaxValue);
                    strcpy(player->si.FinalMap,player->si.NewMap);
                    pack(&player[i]);
                }
                else continue;
            }
        }

        // sleep(1);

    }

    return 0;

}
