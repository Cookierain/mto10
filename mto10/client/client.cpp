#include "public.h"
#include "MD5.h"

#define DEFAULT_IP      "10.60.102.252"
#define DEFAULT_PORT    "21345"
#define DEFAULT_STUNO   "1750979" 
#define DEFAULT_PASSWD  ",KLJsv8|4,2b65Bg"
#define DEFAULT_MAPID   "-1"
#define DEFAULT_ROW     "-1"
#define DEFAULT_COL     "-1"
#define DEFAULT_DELAY   "5000"


// 根据随机码,学号,密码生成认证串
void gen_authenticate_str(char * hex_str, const char* stuno, const char * passwd, ser_info *si){
    string s = passwd;
    MD5 md5 = MD5(s);
    // const char* p = md5.hexdigest().c_str();
    char  str[BUF_LEN];
    sprintf(str,"%s*%s",stuno,md5.hexdigest().c_str());
    // printf("here: %s\n",str);
    encryption(str,si->Content,MD5_LEN);
    str2hex_str(str,hex_str,MD5_LEN);
}

// 包解析函数
void unpack(char *ser_pack, ser_info *si){
    char *name;
    char *value;
    char delim[] = " =\r\n"; 
    while(1){
        
        if(!strcmp(ser_pack,""))break;
        name = strsep(&ser_pack,delim);
        while(!strcmp(name,"")){
            if(!strcmp(ser_pack,""))break;
            name = strsep(&ser_pack,delim);
        }
        
        if(!strcmp(ser_pack,""))break;
        value = strsep(&ser_pack,delim);
        while(!strcmp(value,"")){
            if(!strcmp(ser_pack,""))break;
            value = strsep(&ser_pack,delim);
        }
        // printf("name:%s  ",name);
        // printf("value:%s\n",value);
        if(!strcmp(name,"Type"))strcpy(si->Type,value);
        else if(!strcmp(name,"Content"))strcpy(si->Content       ,value);
        else if(!strcmp(name,"Row"))strcpy(si->Row       ,value);
        else if(!strcmp(name,"Col"))strcpy(si->Col       ,value);
        else if(!strcmp(name,"GameID"))strcpy(si->GameID        ,value);
        else if(!strcmp(name,"Step"))strcpy(si->Step          ,value);
        else if(!strcmp(name,"Score"))strcpy(si->Score         ,value);
        else if(!strcmp(name,"MaxValue"))strcpy(si->MaxValue      ,value);
        else if(!strcmp(name,"OldMap"))strcpy(si->OldMap        ,value);
        else if(!strcmp(name,"NewMap"))strcpy(si->NewMap ,value);
        else if(!strcmp(name,"Map"))strcpy(si->Map,value);
        else if(!strcmp(name,"Delay"))strcpy(si->Delay      ,value);
        else if(!strcmp(name,"Length"))strcpy(si->Length,value);
        else if(!strcmp(name,"FinalStep"    ))strcpy(si->FinalStep    ,value);
        else if(!strcmp(name,"FinalScore"   ))strcpy(si->FinalScore   ,value);
        else if(!strcmp(name,"FinalMaxValue"))strcpy(si->FinalMaxValue,value);
        else if(!strcmp(name,"FinalMap"     ))strcpy(si->FinalMap     ,value);
        else ;
    }
}

// 计算处理函数由ser_info计算并填充->cli_info
void deal(ser_info *si, cli_info *ci){
    // memset(ci,0,sizeof(cli_info));
    if(!strcmp(si->Type,"SecurityString")){ // 收到验证请求
        strcpy(ci->Type,"ParameterAuthenticate");

        
        gen_authenticate_str(ci->MD5,ci->Stuno,ci->Passwd,si);
        // 暂时默认
        // strcpy(ci->Row,"-1");
        // strcpy(ci->Col,"-1");
        // strcpy(ci->GameID,"-1");
        // strcpy(ci->Delay,"5000");
        // strcpy(ci->Length,"179");
    }
    else if(!strcmp(si->Type,"GameProgress")){
        strcpy(ci->Type,"Coordinate");
        if(!strcmp(si->Content,"StartGame") || !strcmp(si->Content,"MergeSucceeded")){ 
            const int col = atoi(si->Col);
            const int row = atoi(si->Row);
            int col_p = rand()%col;  // 0~9
            int row_p = row-1; // Map底部

            int mergable = 0;
            int i = 0;
            int j = 0;
            for(i=0;i!=row;i++){
                for(j=0;j!=col;j++){
                    col_p = (col_p + 1) % col;
                    if(0!=strlen(si->NewMap)){  // NewMap优先
                        if(is_mergeable(row_p,col_p,row,col,si->NewMap))break;
                    }
                    else{
                        if(is_mergeable(row_p,col_p,row,col,si->Map))break;
                    }
                }
                if(0!=strlen(si->NewMap)){  // NewMap优先
                     if(is_mergeable(row_p,col_p,row,col,si->NewMap))break;
                }
                else{
                    if(is_mergeable(row_p,col_p,row,col,si->Map))break;
                }
                row_p = row_p - 1;
            }
            if(i==row){ // 没有可合成的
                disp_err("找不到可合成的位置");
            }
            else{
                sprintf(ci->Row,"%c",(65+row_p));
                sprintf(ci->Col,"%c",(48+col_p));
            }
        }
        
    }
    

}

// 包封装函数
void pack(char *cli_pack, cli_info * ci){
    memset(cli_pack,0,BUF_LEN);
    if(!strcmp(ci->Type,"ParameterAuthenticate")){
        sprintf(cli_pack,"Type = %s\r\nMD5 = %s\r\nRow = %s\r\nCol = %s\r\nGameID = %s\r\nDelay = %s\r\nLength = ",ci->Type,ci->MD5,ci->Row,ci->Col,ci->GameID,ci->Delay);
    }
    else if(!strcmp(ci->Type,"Coordinate")){
        sprintf(cli_pack,"Type = %s\r\nRow = %s\r\nCol = %s\r\nLength = ",ci->Type,ci->Row,ci->Col);
        // sprintf(cli_pack,"Type = %s\r\nRow = %s\r\nCol = %s\r\nLength = ",ci->Type,"A","9");

    }

    // 计算并填充ci->length
    int len = strlen(cli_pack) + 2; // 2: 最后的/r/n
    char str_length[BUF_LEN];
    sprintf(str_length,"%d",len);
    len += strlen(str_length);
    
    
    if(9<=len && len < 10) len += 1;
    if(98 <= len && len < 100) len += 1;
    if(997 <= len && len < 1000)len +=1 ;
    else ;    // 这里省略包长 > 9996的情况 
    sprintf(ci->Length,"%d",len);
    sprintf(cli_pack + strlen(cli_pack),"%d\r\n",len);

}

// 显示命令行参数使用方式
void disp_usage(){
    printf("usage:\n");
    printf("    ./mto10_client --base                                             \n");
    printf("    ./mto10_client --base --ipaddr 10.60.102.252 --port 21345         \n");
    printf("    ./mto10_client --base --stuno 1750979 --passwd ,KLJsv8|4,2b65Bg   \n");
    printf("    ./mto10_client --base --mapid 12345 --row 5 --col 7 --delay 5000  \n");
}


int main(int argc, char *argv[]){

    if(argc <= 1){
        disp_usage();
        return 0;
    }
    else if(1 < argc && (!strcmp(argv[1],"--help") || strcmp(argv[1],"--base"))){
        disp_usage();
        return 0;
    }
    else ;

    param_info pi;
    analy_param(&pi,argv,argc);
    disp_param(&pi);


    ser_info ser_info;
    cli_info cli_info;
    memset(&ser_info,0,sizeof(ser_info));
    memset(&cli_info,0,sizeof(cli_info));
    char ip [PARAM_LEN], port[PARAM_LEN];
    memset(ip,0,PARAM_LEN);
    memset(port,0,PARAM_LEN);


    // --ipaddr --port --stuno --passwd --mapid --row --col
    // "1750979"  ",KLJsv8|4,2b65Bg"
    // 使用参数结构体设置
    for(int i=1;i<pi.param_num;i++){ // 忽略base
        if(!strcmp(pi.name[i],"ipaddr")){
            strcpy(ip,pi.value[i]);
        }
        else if(!strcmp(pi.name[i],"port")){
            strcpy(port,pi.value[i]);
        }
        else if(!strcmp(pi.name[i],"stuno")){
            strcpy(cli_info.Stuno,pi.value[i]);
        }
        else if(!strcmp(pi.name[i],"passwd")){
            strcpy(cli_info.Passwd,pi.value[i]);
        }
        else if(!strcmp(pi.name[i],"mapid")){
            strcpy(cli_info.GameID,pi.value[i]);
        }
        else if(!strcmp(pi.name[i],"row")){
            strcpy(cli_info.Row,pi.value[i]);
        }
        else if(!strcmp(pi.name[i],"col")){
            strcpy(cli_info.Col,pi.value[i]);
        }
        else if(!strcmp(pi.name[i],"delay")){
            strcpy(cli_info.Delay,pi.value[i]);
        }
        else ;
    }

    // 使用缺省设置
    if(0==strlen(ip))strcpy(ip,DEFAULT_IP);
    if(0==strlen(port))strcpy(port,DEFAULT_PORT);
    if(0==strlen(cli_info.Stuno )) strcpy(cli_info.Stuno ,DEFAULT_STUNO );
    if(0==strlen(cli_info.Passwd)) strcpy(cli_info.Passwd,DEFAULT_PASSWD);
    if(0==strlen(cli_info.GameID)) strcpy(cli_info.GameID,DEFAULT_MAPID );
    if(0==strlen(cli_info.Row   )) strcpy(cli_info.Row   ,DEFAULT_ROW   );
    if(0==strlen(cli_info.Col   )) strcpy(cli_info.Col   ,DEFAULT_COL   );
    if(0==strlen(cli_info.Delay )) strcpy(cli_info.Delay ,DEFAULT_DELAY );


    struct sockaddr_in servaddr,cliaddr;
    int connect_fd;
    char rcv_buf[BUF_LEN],snd_buf[BUF_LEN];
    memset(snd_buf,0,BUF_LEN);
    memset(rcv_buf,0,BUF_LEN);
    fd_set wr_fdset, rd_fdset;

    // 确定连接的IP地址和端口
    servaddr.sin_family=AF_INET;                   
    inet_pton(AF_INET,ip,&servaddr.sin_addr); 
    servaddr.sin_port=htons(atoi(port)); 

    // 打开连接描述符并设置为非阻塞
    connect_fd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int flags = fcntl(connect_fd,F_GETFL,0);      
    fcntl(connect_fd,F_SETFL,flags | O_NONBLOCK);

    // 启用REUSEADDR，使端口释放后可再次使用
    int sock_opt;
	setsockopt(connect_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&sock_opt, sizeof(sock_opt));

    // 非阻塞connect
    if(-1 == connect(connect_fd,(struct sockaddr*)&servaddr,sizeof(servaddr))){
        if(errno != EINPROGRESS){ // 直接连接失败
            printf("errno: %d\n",errno);
            disp_err("非阻塞connect失败1");
        }
        FD_ZERO(&wr_fdset);
        FD_SET(connect_fd,&wr_fdset);
        rd_fdset = wr_fdset;
        // 通过可写+select+getsockopt判断连接成功
        if(0 < select(connect_fd + 1, &rd_fdset, &wr_fdset, NULL, NULL)){
            int getsockopt_error = -1, slen = sizeof(int);//getsockopt函数状态码
            getsockopt(connect_fd, SOL_SOCKET, SO_ERROR, &getsockopt_error, (socklen_t *)&slen);
            if(getsockopt_error != 0){
                disp_err("非阻塞connect失败2");
            }
        }
        else{
            disp_err("非阻塞connect失败3");
        }
    }

    
    

    while(1){
        FD_ZERO(&rd_fdset);
        FD_ZERO(&wr_fdset);
        FD_SET(connect_fd,&rd_fdset);
        
        if(0!=strlen(snd_buf)){ // snd_buf不为空置写
            FD_ZERO(&wr_fdset);
            FD_SET(connect_fd,&wr_fdset);
        }

        int ret = select(connect_fd + 1, &rd_fdset, &wr_fdset,NULL,NULL);
        if(ret <= 0){
            disp_err("断开连接");
        }
        if(FD_ISSET(connect_fd,&rd_fdset)){
            memset(rcv_buf,0,BUF_LEN);
            int n = read(connect_fd,rcv_buf,BUF_LEN);
            if(n <= 0){
                disp_err("断开连接");
            }
            printf("read byte num: %d\ncontent: \n%s\n",n,rcv_buf);
            unpack(rcv_buf,&ser_info);
            if(!strcmp(ser_info.Content,"GameOver") || !strcmp(ser_info.Content,"GameFinished") || !strcmp(ser_info.Content,"GameTimeout")){
                printf("游戏结束\n");
                exit(0);
            }
            // disp_ser_info(&ser_info);
            deal(&ser_info,&cli_info);
            disp_ser_info(&ser_info);
            pack(snd_buf,&cli_info);
        }
        else if(FD_ISSET(connect_fd,&wr_fdset)){
            int m = write(connect_fd,snd_buf,atoi(cli_info.Length));
            printf("write byte num: %d\ncontent: \n%s\n",m,snd_buf);
            memset(snd_buf,0,BUF_LEN); // 防止再次触发可写
        }

        // sleep(1);
    }

}