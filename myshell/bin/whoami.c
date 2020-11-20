
#include<stdio.h>
#include<pwd.h>
#include<sys/types.h>
#include<unistd.h>
int main()
{
	uid_t id;
	struct passwd * pa;
	id=geteuid();
	pa=getpwuid(id);
	printf("id:%d\n",id);
	printf("name:%s\n",pa->pw_name);
	return 0;
}
