#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>


int main(int argc, char *argv[])
{
	int file,i,n;
	char buffer[10];

	if(argc == 1)
		while((n=read(0,buffer,10))>0)
		{
			if(write(1,buffer,n) != n)
			{
				write(2, "An error occurred in the write\n", 40);
        		return -1;
        	}
		}
	for(i = 1; i < argc; i++)
	{
		file = open(argv[i],O_RDONLY);
		if(file == -1)
		{
			perror("Error");
			return -1;
		}
		while((n=read(file,buffer,10))>0)
		{
			if(write(1,buffer,n) != n)
			{
				write(2, "An error occurred in the write\n", 40);
        		return -1;
        	}
		}
		
		close(file);
	}
	return 0;
}