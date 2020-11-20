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

// 全局声明
void print_prompt();
void sig_handle(int sig);
char cwd[MAX];  //保存当前路径
char *cmd[MAX]; //cmd 用来保存命令行字符串
char PATH[MAX];
int current_out = 4;
int current_in = 5;
int fd[4];

/* 用于输出提示符 */
void print_prompt()
{
    // 调用uname获取系统信息
    struct utsname uname_ptr;
	struct passwd *pid_to_name;
    uname(&uname_ptr);
	pid_to_name = getpwuid(getuid());
    //调用 getcwd 获取当前路径名，并存储在 cwd 指向的字符串
    getcwd(cwd, sizeof(cwd));
    setbuf(stdout, NULL);       //禁用 buffer， 直接将 printf 要输出的内容输出
    printf("$%s@%s:%s$> ",pid_to_name->pw_name,uname_ptr.sysname, cwd);
}

/* 扫描用户输入的命令行 */
void scan_cmd(char *command)
{
    int bytes_read;
    size_t nbytes = MAX;
    bytes_read = getline(&command, &nbytes, stdin); /*从标准输入中读入包含
                                                    多条命令行语句的字符串，并保存在 command变量中 */
    bytes_read -= 1;
    command[bytes_read] = '\0';
}

/* 以空格为分解命令及其参数 */
void *parse(char *command, int time)
{
    char *comm;   // 用于储存命令或命令的参数
    if(time ==0)
        comm = strtok(command, " ");
    else
        comm = strtok(NULL, " ");
    return comm;
}
/* 分割用户以分号分隔的多命令，如“ls;cd”*/
void parse_semicolon(char *command)
{
    int i ;
    for (i=0; i < MAX; i++)
        cmd[i] = (char *) malloc(MAX_COMM * sizeof(char));
    i = 0;
    cmd[i] = strtok(command, ";"); //注意 strtok 的用法，用来分割命令行
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
/* 创建子进程，调用 execvp 函数执行命令程序*/
void bf_exec(char *arg[])
{
    pid_t pid;

    if((pid = fork()) < 0)
    {
        printf("*** ERROR: forking child process failed\n");
        return ;
    }
    // 父子进程执行代码的分离
    else if(pid == 0)    //子进程
    {
        char cmd_dir[MAX];
        strcpy(cmd_dir, PATH);
        strcat(cmd_dir, arg[0]);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGINT,SIG_DFL);//用于杀死子进程
        signal(SIGQUIT, SIG_IGN);
        execvp(cmd_dir, arg);  // execvp 用于在子进程中替换为另一段程序
    }
    else     //父进程
    {
        signal(SIGINT, sig_handle); // SIGINT = 2， 用户键入Ctrl-C
        signal(SIGQUIT, sig_handle); /* SIGQUIT = 3 用户键入Ctr+\ */
        signal(SIGTSTP, sig_handle);  // SIGTSTP =20，一般有Ctrl-Z产生
        pid_t c;
        c = wait(&pid);  //等待子进程结束
        dup2(current_out, 1); //还原默认的标准输出
        dup2(current_in, 0); //还原默认的标准输入
        return;
    }

}

/* 执行单条命令行语句 */
void execute(char *command)
{
    char *arg[MAX_COMM];
    char *try;
    arg[0] = parse(command, 0);        //获得命令名称的字符串指针，如“ls”
    int t = 1;
    arg[t] = NULL;
	if (strcmp(arg[0], "exit") == 0) // 为了方便用户退出shell
	{
		puts("exited!");
		exit(0);
	}
    if (strcmp(arg[0], "cd") == 0)     // 处理内嵌命令“cd”的情况
    {
        try = parse(command, 1);
        do_cd(try);
        return ;
    }
    if (strcmp(arg[0], "help") == 0)     // 处理内嵌命令“cd”的情况
    {
        do_help();
        return ;
    }    

    while (1)
    {
        try = parse(command, 1);
        if (try == NULL)
            break;
        else       //try是一个命令参数
        {
            arg[t] = try;
            t += 1;
            arg[t] = NULL;
        }
    }

    bf_exec(arg);     // 参数 0 表示前台运行
    return;
}

/* 信号处理函数 */
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
	     /* 捕捉信号，当信号发生时调用信号处理函数sig_handle，建议用sigaction改写 */
        signal(SIGINT, sig_handle); // SIGINT = 2， 用户键入Ctrl-C
        signal(SIGQUIT, sig_handle); /* SIGQUIT = 3 用户键入Ctr+\ */
        signal(SIGTSTP, sig_handle);  // SIGTSTP =20，一般有Ctrl-Z产生
		return;
}
/* 主函数入口 */
int main()
{
    getcwd(cwd, sizeof(cwd));
    strcpy(PATH, cwd);
    strcat(PATH, "/bin/");
    char *command;
    int iter = 0;
    command = (char *)malloc(MAX+1); //用于存储命令语句

    chdir("/home/");  /* 通常在登陆 shell 的时候通过查找配置文件（/etc/passwd 文件）
                               中的起始目录，初始化工作目录，这里默认为家目录 */
    while(1)
    {
        iter = 0;
		//定义信号处理方式
		signal_set();
        //用于输出提示符
        print_prompt();

        //扫描多条命令语句
        scan_cmd(command);

        // 基于分号解析出单条命令语句，并以字符串的形式存进全局变量 cmd 中
        parse_semicolon(command);

        // 迭代执行单条命令语句
        while(cmd[iter] != NULL)
        {
            execute(cmd[iter]); //这是 shell 解释器的核心函数
            iter += 1;
        }
    }
}
