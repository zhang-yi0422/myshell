#include<stdio.h>
#include<string.h>
#include <errno.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define BUFSIZE 1024

int main(int argc,char*argv[])
{
	int sourfd,dstfd,numRead,numWrite;
	int totalByte=0;
	char buf[BUFSIZE];

	if(argv[1]==NULL||argv[2]==NULL)
	{
		printf("Usage:%s sourfile  dstfile\n",argv[0]);
		exit(0);
	}

	if((sourfd=open(argv[1],O_RDONLY))==-1)
	{
		perror("open");
	}
    printf("open %s succeed\n",argv[1]);

	if((dstfd=open(argv[2],O_CREAT|O_WRONLY))==-1)
	{
		perror("open");
	}

	printf("open %s succeed\n",argv[2]);
	while((numRead=read(sourfd,buf,BUFSIZE))>0)
	{
		printf("read:%d\n",numRead);
		if(((numWrite=write(dstfd,buf,numRead)))!=numRead)
		{
			perror("write");
		}
		printf("write total %d byte to %s\n",totalByte+=numWrite,argv[2]);
	}
	close(sourfd);
	printf("close sourfd\n");
	close(dstfd);
	printf("close dstfd\n");

	return 0;
}
