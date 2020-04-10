#include "public.h"

// ��ӡ������Ϣ���˳�
void disp_err(const char* info){
    printf("error: %s\n",info);
    exit(1);
}


void disp_map(ser_info * si){
    if(strlen(si->NewMap)!=0 || strlen(si->Map)!=0 || 0!=strlen(si->FinalMap)){
        printf("  |  ");
        for(int i=0;i<atoi(si->Col);i++){
            printf("%d  ",i);
        }
        printf("\n");

        printf("--+--");
        for(int i=0;i<atoi(si->Col);i++){
            printf("---");
        }
        printf("\n");

        for(int i=0;i<atoi(si->Row);i++){
            printf("%c |  ", 65 + i);
            for(int j=0;j<atoi(si->Col);j++){
                if(0!=strlen(si->FinalMap))printf("%c  ",si->FinalMap[i*atoi(si->Col)+j]);
                else if(0!=strlen(si->NewMap))printf("%c  ",si->NewMap[i*atoi(si->Col)+j]);
                else if(0!=strlen(si->Map))printf("%c  ",si->Map[i*atoi(si->Col)+j]);
            }
            printf("\n");
        }
    }
    
}

void disp_ser_info(ser_info * si){
    printf("\n------------------ser_info display---------------\n");
    // printf("Type          :%s\n",si->Type          ); // ;��ʶ����
    // printf("Content       :%s\n",si->Content       );
    // printf("Row       :%s\n",si->Row       );
    // printf("Col       :%s\n",si->Col       );
    // printf("Delay         :%s\n",si->Delay         );
    // printf("GameID        :%s\n",si->GameID        );
    // printf("Step          :%s\n",si->Step          );
    // printf("Score         :%s\n",si->Score         );
    // printf("MaxValue      :%s\n",si->MaxValue      );
    // printf("Map       :%s\n",si->Map       );
    // printf("OldMap        :%s\n",si->OldMap        );
    // printf("NewMap        :%s\n",si->NewMap        );
    // printf("Length        :%s\n",si->Length        );
    disp_map(si);
    printf("------------------end of ser_info display---------------\n\n");
}

void disp_cli_info(cli_info * ci){
    printf("\n------------------cli_info display---------------\n");
    printf("Type       :%s\n",ci->Type        ); // ;��ʶ����
    printf("Row        :%s\n",ci->Row         );
    printf("Col        :%s\n",ci->Col         );
    printf("MD5        :%s\n",ci->MD5         );
    printf("Delay      :%s\n",ci->Delay       );
    printf("GameID     :%s\n",ci->GameID      );
    printf("Length     :%s\n",ci->Length      );
    printf("------------------end of cli_info display---------------\n");
}

// ���ַ���ת����16���ƴ�(����*2)
void str2hex_str(char * str, char * hex_str, int str_len){
    int i;
    for(i=0;i<str_len;i++){
        sprintf(hex_str+(i*2),"%02x",str[i]);
    }
}

// ��16���ƴ�ת��Ϊ�ַ���(����/2)
void hex_str2str(char *hex_str, char *str, int str_len){
    char temp[3];
    char ** endptr; // strtol����ʹ��
    memset(temp,0,3);
    for(int i = 0;i < str_len;i++){
        memcpy(temp,hex_str+2*i,2);
        sprintf(str+i,"%c",(char)strtol(temp,endptr,16));
    }
}

// ��������ض����ȵ�hex�ַ��� (< BUF_LEN / 2)
void gen_hex_str(char *hex_str, int len){
    srand(time(0)); // ��ǰ������ʱ��
    if(len >= BUF_LEN/2){
        disp_err("�޷����ɸó��ȵ�hex��");
    }
    char long_hex_str[BUF_LEN];
    char ascii_str[BUF_LEN]; 
    for(int i=0;i<len;i++){
        ascii_str[i] = rand()%128; // char��Χ0~127
    }
    str2hex_str(ascii_str,long_hex_str,len);
    memcpy(hex_str,long_hex_str,len);
    hex_str[len] = 0;
    return ;
}

// �ַ���������(���ڼӽ���)
void encryption(char *str, char *key, int len){
    int i;
    for(i=0;i<len;i++){
        str[i] = str[i] ^ key[i];
    }
}

// �����ѡλ���ܷ�ϳ�(����=1,����=0)
int is_mergeable(int row_p, int col_p, int row, int col, const char * map){
    int index = row_p*col + col_p;
    if(row_p!=0 && map[index - col] == map[index] ){ // �����
        return 1;
    }
    else if(col_p!=0 && map[index - 1] == map[index]){ // �����
        return 1;
    }
    else if(col_p!=col-1 && map[index + 1] == map[index]){ // �����
        return 1;
    }
    else if(row_p!=row-1 && map[index + col] == map[index]){
        return 1;
    }
    else return 0;
}

// �����в�������
void analy_param(param_info* pi, char *argv[], int argc){
    
    memset(pi,0,sizeof(param_info));
    int i,j;
    for(i = 1, j = 0; i < argc; i++, j++){
        if(argv[i][0]=='-' && argv[i][1]=='-'){
            pi->param_num++;
            
            strcpy(pi->name[j],argv[i]);
            strcpy(pi->name[j],&pi->name[j][2]);
            if(argc <= i+1){
                continue;
            }
            else if(argv[i+1][0]=='-' && argv[i+1][1]=='-'){ 
                continue;
            }
            else{
                strcpy(pi->value[j],argv[i+1]);
                i++;
            }
        }
    }

    return;
}

// ��ʾ�����������
void disp_param(param_info* pi){
    printf("\n----------------�����в���------------------\n");
    printf("��������: %d\n",pi->param_num);
    for(int i=0;i<pi->param_num;i++){
        printf("%s = %s\n",pi->name[i],pi->value[i]);
    }
    printf("----------------end of �����в���------------------\n");
}