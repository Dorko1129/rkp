int main(int argc, char* argv[])
{
  int in;
  int out;
  int tmp;
  struct stat inode;

  // 1. ELŐSZÖR ellenőrizzük az argumentumok számát
  if(argc < 3){
    fprintf(stderr," File or directory name is necessary.\n");
    return 1;
  }

  // 2. CSAK UTÁNA nyúlunk az argv elemeihez
  in  = open(argv[1], O_RDONLY);
  out = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

  if(in < 0 || out < 0){
    write(2, " Openning error!\n", 17);
    return 2;
  }

  tmp = stat(argv[1], &inode);
  if(tmp < 0){
    fprintf(stderr," Bad name!\n");
    return 2;
  }
   
  close(in);
  close(out);

  return 0;
}
  if(tmp<0){
    fprintf(stderr," Bad name!\n");
    return 2;
    }
   
   
   
 close(in);
 close(out);


    return 0;
}
