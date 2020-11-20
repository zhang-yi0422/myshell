/*
* �ļ�����mypwd.c
* ������ʵ�ּ򵥵�pwd����
*/
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

/*�����ļ�����ȡ�ļ�inode-number*/
ino_t get_ino_byname(char *filename)
{
    struct stat file_stat;
    if (0 != stat(filename, &file_stat)) {
        perror("stat");
        exit(-1);
    }
    return file_stat.st_ino;
}

/*����inode-number ���ڵ�ǰĿ¼�в��Ҷ�Ӧ���ļ���*/
char* find_name_byino(ino_t ino)
{
    DIR *dp = NULL;
    struct dirent *dptr = NULL;
    char *filename = NULL;
    if (NULL == (dp = opendir("."))) {
        fprintf(stderr, "Can not open Current Directory\n");
        exit(-1);
    } else {
        while (NULL != (dptr = readdir(dp))) {
            if (dptr->d_ino == ino) {
                filename = strdup(dptr->d_name);
                break;
            }
        }
        closedir(dp);
    }
    return filename;
}

/*��������Ŀ¼���*/
#define MAX_DIR_DEPTH (256)

int main(int argc, char *argv[])
{
    /*��¼Ŀ¼����ջ*/
    char *dir_stack[MAX_DIR_DEPTH];
    unsigned current_depth = 0;

    for(;;) {
        /*1.ͨ��������ļ�����.����ȡ��ǰĿ¼��inode-number*/
        ino_t current_ino = get_ino_byname(".");
        /*2.ͨ��������ļ�����..����ȡ��ǰĿ¼�ĸ���Ŀ¼��inode-number*/
        ino_t parent_ino = get_ino_byname("..");

        /*3.�жϵ�ǰĿ¼���ϼ�Ŀ¼��inode-number�Ƿ�һ��*/
        if (current_ino == parent_ino)
            break; /*4.�������inode-numberһ��˵�������Ŀ¼*/

        /*5.�������inode-number��һ��*/
        /*�л�������Ŀ¼�����ݲ���1��ȡ��inode-number���ڸ���Ŀ¼��������Ӧ���ļ�������¼����, ���»ص�����1*/
        chdir("..");
        dir_stack[current_depth++] = find_name_byino(current_ino);
        if (current_depth>=MAX_DIR_DEPTH) { /*·����̫��*/
             fprintf(stderr, "Directory tree is too deep.\n");
             exit(-1);
        }
    }

    /*�������·����*/
    int i = current_depth-1;
    for (i = current_depth-1; i>=0; i--) {
       fprintf(stdout, "/%s", dir_stack[i]);
    }
    fprintf(stdout, "%s\n", current_depth==0?"/":"");

    return 0;
}
