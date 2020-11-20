#include<stdio.h>
#include<unistd.h>//����getoptͷ�ļ�
#include<stdlib.h>
int bflag, nflag, num;//�����b����b  �� �к�
void cat_file(const char *);
int main(int argc, char *argv[]) {
    char c;
    int i;
    while((c = getopt(argc, argv, "bn")) != -1) {//�ж������в��� �� -b ���� -n
        switch(c) {
            case 'b':
                bflag = 1;
                break;
            case 'n':
                nflag = 1;
                break;
            default :
                fprintf(stdout, "Use : %s [-b|-n] file!\n", argv[0]);
                exit(1);
        }
    }
    for(i = optind; i < argc; i++) {//�����ж���ļ���ӡ��optind�ٴε�����һ��argv������
        cat_file(argv[i]);
    }
    return 0;
}
void cat_file(const char *file) {
    FILE *fp;
    if((fp = fopen(file, "r")) == NULL) {//�ж��ǲ��ǿ��ļ�
        //��ӡ������Ϣ
        perror(file);
        exit(1);
    }
    char buff[1024] = {0};
    while(fgets(buff, sizeof(buff), fp)) {//���ж�ȡ
        //�������� ֱ�Ӵ�ӡ
        if(!nflag && !bflag) {
            printf("%s", buff);
            continue;
        }
        if(buff[0] != '\n'){
            num++;
            printf("%d\t%s", num, buff);
        }else {
            //nflag���� ��ӡ�к� ������ ����ӡ�к�
            if(nflag && !bflag) {
                num++;
                printf("%d\t\n", num);
            }else {
                printf("\n");
            }
        }
    }
    fclose(fp);//�ر��ļ�
    return ;
}
