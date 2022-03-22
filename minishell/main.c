#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>
#include<stdlib.h>

int main()
{
    while(1){
        printf("[minishell@localhost]$ ");
        fflush(stdout); //刷新缓冲区

        char buf[1024]={0};
        fgets(buf,sizeof(buf)-1,stdin);
        if(strlen(buf)==0 || buf[0]=='\n'){
            continue;
        }

        buf[strlen(buf)-1]='\0';    //将输入的一行数据最后一个‘\n'换为’\0'

        pid_t pid=fork();
        if(pid<0){
            perror("fork");
            continue;
        }else if(pid==0){   //child
            //区分命令程序和命令行参数
            printf("%s\n",buf);
            //avgs存有各参数地址的指针数组的地址
            char* argv[1024]={0};
            int pos=0;
            char* begin=buf;
            char* end=buf;

            while(*end!='\0'){
                while(*end!=' ' && *end!='\0'){
                    ++end;
                }
                
                //记录参数字符串地址
                argv[pos]=begin;
                pos++;

                if(*end=='\0'){ //读到了结尾
                    break;
                }else{
                    *end='\0';  //为每个参数封上‘\0'
                    end++;
                    begin=end;
                }
            }

            printf("pos: %d\n",pos);
            argv[pos]=NULL; //NULL结尾
            
            for(int i=0;i<pos;++i){
                printf("argv[%d];%s\n",i,argv[i]);
            }
            
            //进程程序替换
            execvp(argv[0],argv);
            //替换失败
            printf("test...\n");

            exit(1);
        }else{  //parent
            wait(NULL);
            
        }

    }
    return 0;
}
