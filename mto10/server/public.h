#include <iostream>     //cin, cout
#include <list>         //list<>
#include <cstdio>       //perror(), getchar(), snprintf()
#include <cstdlib>      //atoi(), exit()
#include <cstring>      //strncat(), strncpy(), strncmp(), strerror()
#include <ctime>        //time()��ϵͳʱ�亯����
#include <unistd.h>     //open(), close(), alarm(), sleep()��linux������
#include <fcntl.h>      //fcntl()
#include <signal.h>     //signal()
#include <syslog.h>     //дϵͳ��־syslog()����
#include <errno.h>      //errno���Լ�EINPROGRESS�ȴ�����궨��
#include <stdarg.h>     //��������
#include <getopt.h>     //����������в���
#include <arpa/inet.h>  //inet_addr(), inet_ntoa(), htonl(), htons(), ntohl(), ntohs()
#include <netinet/in.h> //sockaddr_in�ṹ��
#include <sys/socket.h> //socket(), bind(), listen(), accept(), connect(), recv(), send()
#include <sys/types.h>  //�궨��
#include <sys/signal.h> //
#include <sys/stat.h>   //STATUS�궨��
#include <sys/select.h> //select(), fd_set
#include <sys/file.h>   //flock, lockf
#include <sys/prctl.h>  //���޸Ľ�������
#include <sys/time.h>   //struct itimerval, 
#include <sys/sem.h>    //�ź�����������
#include <sys/shm.h>    //�����ڴ��������
#include <fstream>     //cin, cout
#include <algorithm>   //list 
#include <type_traits> // �Զ���DataTimeʹ�� ��Ҫ����ѡ�� -std=c++11

using namespace std;

#define BUF_LEN  512
#define MD5_LEN  40  // ���ܴ�����
#define MAX_LINK 50 // ������������

#define MAX_PARAM_NUM 20 // ���20�������в���
#define PARAM_LEN 50     // ��������(���ƺ�ֵ�50)



// ��ӡ������Ϣ���˳�
void disp_err(const char* info);

struct param_info{
    char name[MAX_PARAM_NUM][PARAM_LEN];
    char value[MAX_PARAM_NUM][PARAM_LEN];
    int param_num = 0;
};

struct cli_info{
    char Type   [BUF_LEN];
    char Row    [BUF_LEN];
    char Col    [BUF_LEN];
    char MD5    [BUF_LEN];
    char Delay  [BUF_LEN];
    char GameID [BUF_LEN];
    char Length [BUF_LEN];

    // client ʹ��, server��ʹ��
    char Stuno  [BUF_LEN];
    char Passwd [BUF_LEN];
};

struct ser_info{
    char Type           [BUF_LEN];
    char Content        [BUF_LEN];
    char GameID         [BUF_LEN];
    char Row            [BUF_LEN];
    char Col            [BUF_LEN];
    char Step           [BUF_LEN];
    char Score          [BUF_LEN];
    char MaxValue       [BUF_LEN];
    char Map            [BUF_LEN];
    char OldMap         [BUF_LEN];
    char NewMap         [BUF_LEN]; // StartGame��Map
    char Delay          [BUF_LEN];
    char FinalStep      [BUF_LEN];   
    char FinalScore     [BUF_LEN];   
    char FinalMaxValue  [BUF_LEN];   
    char FinalMap       [BUF_LEN];   
    char Length         [BUF_LEN];

    // server�ڲ�, clientʹ��
    char Stuno          [BUF_LEN];
    char LastTalkTime   [BUF_LEN];  // �ϴη��ͱ��ĵ�ʱ��
    char LoginTime      [BUF_LEN];  // ��¼��ʱ��  
};



// ʱ��ת����(time_t��DATETIME�ַ���:2020-03-15 22:00:00����ת��)
// string timeStr = DateTime::convert<string>(time(nullptr));
// cout << timeStr << endl;
// cout << DateTime::convert<time_t>(timeStr) << endl;
// cout << DateTime::currentTime() << endl << endl;
class DateTime
{
public:
    template<typename T>
    static typename std::enable_if<std::is_same<std::string, T>::value, std::string>::type convert(time_t t)
    {
        return time2string(t);
    }

    template<typename T>
    static typename std::enable_if<std::is_same<time_t, T>::value, time_t>::type convert(const std::string& timeStr)
    {
        return string2time(timeStr);
    }

    static std::string currentTime()
    {
        return time2string(time(nullptr));
    }

private:
    static std::string time2string(time_t t)
    {
        struct tm* tmNow = localtime(&t);
        char timeStr[sizeof("yyyy-mm-dd hh:mm:ss")] = {'\0'};
        std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tmNow);
        return timeStr;
    }
    
    static time_t string2time(const std::string& timeStr)
    {
        struct tm stTm;
        sscanf(timeStr.c_str(), "%d-%d-%d %d:%d:%d",
               &(stTm.tm_year),
               &(stTm.tm_mon),
               &(stTm.tm_mday),
               &(stTm.tm_hour),
               &(stTm.tm_min),
               &(stTm.tm_sec));

        stTm.tm_year -= 1900;
        stTm.tm_mon--;
        stTm.tm_isdst = -1;

        return mktime(&stTm);
    }
};


void disp_map(ser_info * si);
void disp_ser_info(ser_info * si);
void disp_cli_info(cli_info * ci);

void str2hex_str(char * str, char * hex_str, int str_len);
void hex_str2str(char *hex_str, char *str, int str_len);
void gen_hex_str(char *hex_str, int len);
void encryption(char *str, char *key, int len);

int is_mergeable(int row_p, int col_p, int row, int col, const char * map);

void analy_param(param_info* pi, char *argv[], int argc);
void disp_param(param_info* pi);