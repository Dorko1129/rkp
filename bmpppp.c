#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <fcntl.h>


int main(int argc, char* argv[])
{
  int in;
  int out;
  int nread;
  int tmp;
  struct stat inode;
  struct passwd *pwd;

  in  = open(argv[1],O_RDONLY);
  out = open(argv[2],O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
  if(in<0 || out<0){
    write(2," Openning error!\n",17);
    return 2;
    }
   if(argc < 3)
   {

    return 1;
   }

  if(argc==1){
    fprintf(stderr," File or directory name is necessary.\n");
    return 1;
    }

  tmp = stat(argv[1],&inode);
  if(tmp<0){
    fprintf(stderr," Bad name!\n");
    return 2;
    }
   
   
   
 close(in);
 close(out);


    return 0;
}
