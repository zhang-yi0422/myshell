#include<stdio.h>
#include<string.h>
#include<signal.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<fcntl.h>
#include<sys/utsname.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<sys/ptrace.h>
#include<sys/types.h>
#include<pwd.h>
#define MAX 1024
#define MAX_COMM 100

// ȫ������
void print_prompt();
void sig_handle(int sig);
char cwd[MAX];  //���浱ǰ·��
char *cmd[MAX]; //cmd ���������������ַ���
char PATH[MAX];
int current_out = 4;
int current_in = 5;
int fd[4];

/* ���������ʾ�� */
void print_prompt()
{
    // ����uname��ȡϵͳ��Ϣ
    struct utsname uname_ptr;
	struct passwd *pid_to_name;
    uname(&uname_ptr);
	pid_to_name = getpwuid(getuid());
    //���� getcwd ��ȡ��ǰ·���������洢�� cwd ָ����ַ���
    getcwd(cwd, sizeof(cwd));
    setbuf(stdout, NULL);       //���� buffer�� ֱ�ӽ� printf Ҫ������������
    printf("$%s@%s:%s$> ",pid_to_name->pw_name,uname_ptr.sysname, cwd);
}

/* ɨ���û������������ */
void scan_cmd(char *command)
{
    int bytes_read;
    size_t nbytes = MAX;
    bytes_read = getline(&command, &nbytes, stdin); /*�ӱ�׼�����ж������
                                                    ���������������ַ������������� command������ */
    bytes_read -= 1;
    command[bytes_read] = '\0';
}

/* �Կո�Ϊ�ֽ��������� */
void *parse(char *command, int time)
{
    char *comm;   // ���ڴ������������Ĳ���
    if(time ==0)
        comm = strtok(command, " ");
    else
        comm = strtok(NULL, " ");
    return comm;
}
/* �ָ��û��Էֺŷָ��Ķ�����硰ls;cd��*/
void parse_semicolon(char *command)
{
    int i ;
    for (i=0; i < MAX; i++)
        cmd[i] = (char *) malloc(MAX_COMM * sizeof(char));
    i = 0;
    cmd[i] = strtok(command, ";"); //ע�� strtok ���÷��������ָ�������
    while(1)
    {
        i += 1;
        cmd[i] = strtok(NULL, ";");
        if(cmd[i] == NULL)
            break;
    }
}

//cd
int do_cd(char *args)
{
    if(args == NULL)
    {
        printf("insufficient arguments\n");
    }
    else
    {
        int cond;
        cond = chdir(args);
        if(cond == -1)
        {
            printf("wrong path,please check your path!\n");
        }

    }
    return 1;
}
//help
void do_help()
{
	printf("Welcome to myshell!\n");
	printf("Made by zhaohaofeng, zhangyi and jiangwen.\n");
	printf("Commands list:\n");
	printf("cat\t\tcd\t\tcp\ndate\t\tls\t\tmkdir\nmv\t\tpwd\t\trm\nrmdir\t\ttouch\t\twhoami\n");
}\
/* �����ӽ��̣����� execvp ����ִ���������*/
void bf_exec(char *arg[])
{
    pid_t pid;

    if((pid = fork()) < 0)
    {
        printf("*** ERROR: forking child process failed\n");
        return ;
    }
    // ���ӽ���ִ�д���ķ���
    else if(pid == 0)    //�ӽ���
    {
        char cmd_dir[MAX];
        strcpy(cmd_dir, PATH);
        strcat(cmd_dir, arg[0]);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGINT,SIG_DFL);//����ɱ���ӽ���
        signal(SIGQUIT, SIG_IGN);
        execvp(cmd_dir, arg);  // execvp �������ӽ������滻Ϊ��һ�γ���
    }
    else     //������
    {
        signal(SIGINT, sig_handle); // SIGINT = 2�� �û�����Ctrl-C
        signal(SIGQUIT, sig_handle); /* SIGQUIT = 3 �û�����Ctr+\ */
        signal(SIGTSTP, sig_handle);  // SIGTSTP =20��һ����Ctrl-Z����
        pid_t c;
        c = wait(&pid);  //�ȴ��ӽ��̽���
        dup2(current_out, 1); //��ԭĬ�ϵı�׼���
        dup2(current_in, 0); //��ԭĬ�ϵı�׼����
        return;
    }

}

/* ִ�е������������ */
void execute(char *command)
{
    char *arg[MAX_COMM];
    char *try;
    arg[0] = parse(command, 0);        //����������Ƶ��ַ���ָ�룬�硰ls��
    int t = 1;
    arg[t] = NULL;
	if (strcmp(arg[0], "exit") == 0) // Ϊ�˷����û��˳�shell
	{
		puts("exited!");
		exit(0);
	}
    if (strcmp(arg[0], "cd") == 0)     // ������Ƕ���cd�������
    {
        try = parse(command, 1);
        do_cd(try);
        return ;
    }
    if (strcmp(arg[0], "help") == 0)     // ������Ƕ���cd�������
    {
        do_help();
        return ;
    }    

    while (1)
    {
        try = parse(command, 1);
        if (try == NULL)
            break;
        else       //try��һ���������
        {
            arg[t] = try;
            t += 1;
            arg[t] = NULL;
        }
    }

    bf_exec(arg);     // ���� 0 ��ʾǰ̨����
    return;
}

/* �źŴ����� */
void sig_handle(int sig)
{
    if(sig == SIGINT) // SIGINT = 2
    {
        printf("\nInstead of Ctrl-C type quit\n");
        print_prompt();
    }
    else if(sig == SIGQUIT)  //SIGQUIT = 3
    {
        printf("\nType quit to exit\n");
        print_prompt();
    }
	else if(sig == SIGTSTP)
	{
	 printf("\nInstead of Ctrl-Z type pause\n");
	 print_prompt();
	}
}
void signal_set(){
	     /* ��׽�źţ����źŷ���ʱ�����źŴ�����sig_handle��������sigaction��д */
        signal(SIGINT, sig_handle); // SIGINT = 2�� �û�����Ctrl-C
        signal(SIGQUIT, sig_handle); /* SIGQUIT = 3 �û�����Ctr+\ */
        signal(SIGTSTP, sig_handle);  // SIGTSTP =20��һ����Ctrl-Z����
		return;
}
/* ��������� */
int main()
{
    getcwd(cwd, sizeof(cwd));
    strcpy(PATH, cwd);
    strcat(PATH, "/bin/");
    char *command;
    int iter = 0;
    command = (char *)malloc(MAX+1); //���ڴ洢�������

    chdir("/home/");  /* ͨ���ڵ�½ shell ��ʱ��ͨ�����������ļ���/etc/passwd �ļ���
                               �е���ʼĿ¼����ʼ������Ŀ¼������Ĭ��Ϊ��Ŀ¼ */
    while(1)
    {
        iter = 0;
		//�����źŴ���ʽ
		signal_set();
        //���������ʾ��
        print_prompt();

        //ɨ������������
        scan_cmd(command);

        // ���ڷֺŽ���������������䣬�����ַ�������ʽ���ȫ�ֱ��� cmd ��
        parse_semicolon(command);

        // ����ִ�е����������
        while(cmd[iter] != NULL)
        {
            execute(cmd[iter]); //���� shell �������ĺ��ĺ���
            iter += 1;
        }
    }
}
