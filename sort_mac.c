/*yinjun 2019.1.4*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>


static char * vendor_company_code[] = {
    "INTELBRAS",  // 0
    "GOOGLE",    // 1
    "ASUS",   // 2
    "AMAZON",   // 3
    "LENOVO",     // 4
    "MOTOROLA",    // 5
    "LG",     // 6
    "SAMSUNG",        // 7
    "APPLE",        // 8
    "BLACKBERTY",        // 9
    "HTC",       // 10
    "XIAOMI",        // 11
    "HUAWEI",      // 12
    "NOKIA",         // 13
    "PANASONIC",        // 14
    "TOSHIBA",        // 15
    "SONY",         // 16
    "MICROSOFT",      // 17
    "PHILIPS",     // 18
    "ACER",         // 19
    "DELL",      // 20
    "HP",       // 21
    "EPSON",      // 22
    "MEIZU",   // 23
    "ONEPLUS", //24 
};

int get_file_line(char *file)
{
  if(NULL == file)
  {
	  printf("file is empty\n");
	  return 0;
  }

  int file_line = 0 ;
  char buf[16] = {0};
  FILE* fd;
  fd = fopen(file,"r");

  while(NULL != fgets(buf,sizeof(buf),fd))
  {
	  file_line++;
  }

  fclose(fd);
  return file_line;
}

void sort_mac(char *file ,int file_line)
{
   if(NULL == file || 0 == file_line)
   {
	   printf("Invaild input in function %s\n",__FUNCTION__);
	   return ;
   }
   int i = 0;
   int j = 0;
   char temp_buf[16] = {0};
   char file_buf[file_line][16]; 

   FILE *fd;
   fd = fopen(file,"rt+");

   for(i;i<file_line;i++)
   {
	if(NULL == fgets(file_buf[i],sizeof(temp_buf),fd))
	{
		printf("fgets file is end\n");
		break;
	}
   }

   for(i=0;i<file_line-1;i++)
   {
	   for(j=0;j<file_line-i-1;j++)
	   {	   
		   if(strncmp(file_buf[j],file_buf[j+1],8) > 0)//只比较8位mac地址
		   {
			  strcpy(temp_buf,file_buf[j+1]);
			  strcpy(file_buf[j+1],file_buf[j]);
			  strcpy(file_buf[j],temp_buf);
		   }
	   }
   }

	for(i=0;i<file_line;i++)
	{
		printf("file_buf[%d]=%s\n",i,file_buf[i]);
		fprintf(fd,"%s",file_buf[i]);
	}

	printf("mac sort is success\n");
	fclose(fd);

}



void write_value_to_mac(char *mac_file,char *new_file)
{
	if(NULL == mac_file)
	{
		printf("input file size equal null\n");
		return ;
	}
	FILE *fd_file;
	FILE *fd_new_file;
	int ret = 0;
	char result_buf[64] = {0};
	char new_file_buf[64] = {0};
	char temp_buf[64] = {0};

	fd_file = fopen(mac_file,"r+");
	fd_new_file = fopen(new_file,"wr+");

	while(fgets(result_buf,64,fd_file) != NULL)
	{
		static int i = 0;
		if(!strncmp(result_buf,vendor_company_code[i],sizeof(vendor_company_code[i])))
		{
			//printf("result_buf = %s\n",vendor_company_code[i]);
			continue;
		}

		snprintf(temp_buf,strlen(result_buf)-1,"%s",result_buf);
		sprintf(new_file_buf,"%s=%d",temp_buf,i);
		//printf("YJtest:%s\n",new_file_buf);

		if(NULL != strstr(result_buf,"---"))
		{
			if (24 == i)
			{
				break;
			}

			i++;
			continue;
		}
		
		if(strlen(temp_buf) > 3)
		{
			strcat(new_file_buf,"\r\n");
			fprintf(fd_new_file,"%s",new_file_buf);
		}
	}
	printf("write vendor value to mac address is success\n");
	fclose(fd_file);
	fclose(fd_new_file);
}

int main(int argc,char *argv[])
{
	int file_line = 0;
	char *new_file = "sort_mac.txt";
    /*step1:给所有的mac根据vendor赋值，并去除vendor信息，---以及空行*/
	write_value_to_mac(argv[1],new_file);
	file_line = get_file_line(new_file);

    /*step2:给所有的mac排序*/
	sort_mac(new_file,file_line);

}