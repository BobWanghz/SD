#include "SD.h"

//static char buf_back[512];
//static char buf_data[512];

template<typename T>
T change_endian(T p)
{
    if(little_endian)
    {
        if(sizeof(T) == 2) return (p>>8) | (p<<8 & 0xFF00);
        if(sizeof(T) == 4) return (p>>24 & 0x000000FF) | (p>>8 & 0x0000FF00) | (p<<8 & 0x00FF0000) | (p<<24 & 0xFF000000);
    }
    return p;
}


/*****************************************************************************
** Function name:	set_path
** Description:		Set the path to save file.
** Parameters:		txt_num - Number of txt,such as data0.txt,here txt_num is 0.
** Returned value:	None
******************************************************************************/
void set_path(int txt_num)
{
	char N[2];
	sprintf(data_path,"/sd/data");
	sprintf(N,"%d",txt_num);
	strcat(data_path,N);
	strcat(data_path,".txt");
}
/**
*	Judge whether send the backup data.
*/
bool back_flag()
{
	FILE* fp = fopen("/sd/back.pac","r");
	if(fp)
	{
		fseek(fp,0L,SEEK_END);
		int size = ftell(fp);
		if(size < 10)
			return false;
		else
			return true;
	}
	else 
	{
		pc.printf("Open /sd/back.pac error!\r\n");
		return false;
	}
}
int create_file(char * file_path)
{
	FILE* fp = fopen(file_path,"ab");
	if(fp)
	{
		pc.printf("Create %s with append ok!\r\n",file_path);
		fclose(fp);
		return 0;
	}
	else
	{
		pc.printf("Open %s with append error!\r\n",file_path);
		return -1;
	}
}

int write_location(int* location)
{
	if(*location < 0)
	{
		pc.printf("error location\r\n");
		return -1;
	}
	int position = change_endian(*location);
	FILE* fp_len = fopen("/sd/length.txt","rb+");
	if(fp_len)
	{
		fseek(fp_len,0L,SEEK_SET);
		fwrite(&position,1,4,fp_len);
		pc.printf("write location:%d\r\n",*location);
		fclose(fp_len);
		return 0;
	}else
	{
		pc.printf("Open /sd/length.txt error!\r\n");
		return -1;
	}
}
int read_location(int* location)
{
	*location = 0;
	FILE* fp_len = fopen("/sd/length.txt","rb");
	if(fp_len)
	{
		fseek(fp_len,0L,SEEK_SET);
		u8 Loc[4];
		memset(Loc,0,4);
		fread(Loc,1,4,fp_len);
		//int i = 0;
		//for(i = 0;i < 4;i++)
			//pc.printf("%x ",Loc[i]);
		//pc.printf("\r\n");
		*location = Loc[0] * (2 << 23) + Loc[1] * (2 << 15) + Loc[2] * (2 << 7) + Loc[3];
		if(*location < 0)
			location = 0;
		pc.printf("read location is:%d\r\n",*location);
		fclose(fp_len);
		return  0;
	}else
	{
		pc.printf("Open /sd/length.txt error!\r\n");
		return -1;
	}
}
void print_buf(u8* buf,int buf_len)
{
	int i = 0;
	for(i = 0;i < buf_len;i++)
	{
		if(buf[i] < 0x10)	pc.printf("0");
		pc.printf("%X ",buf[i]);
	}
	pc.printf("\r\n");

}

/*****************************************************************************
** Function name:	write_to_sd
** Description:		Write message to SD card.
** Parameters:		None
** Returned value:	None
******************************************************************************/
void write_to_sd()
{
	int file_write = 0;
	int file_size;
	int location = 0;
	int rel = 0;
	pc.printf("Enter write_to_sd func!\r\n");
	if(!tcp && !protocol_test)	//Network is unreachable.
	{
		pc.printf("Trying to backup data.\r\n");
		FILE* fp1 = fopen("/sd/back.pac","rb+");
		//if((rel = setvbuf(fp1,NULL,_IONBF,0)) != 0)
			//pc.printf("setvbuf back error!\r\n");
		if(fp1)
		{
			read_location(&location);
			fseek(fp1,location,SEEK_SET);
			file_write = fwrite(submit, 1, length, fp1);
			//print_buf(submit,length);
			if(file_write == 0) pc.printf("SD write error!code\r\n");
			else fflush(fp1);
			u16 write_len = change_endian(length);
			file_write = fwrite(&write_len, 1, 2, fp1);
			//pc.printf("%x \r\n",length);
			if(file_write == 0) pc.printf("SD write error!code\r\n");
			else fflush(fp1);
			fseek(fp1,0L,SEEK_END);
			location += (length + 2);
			//pc.printf("file: /sd/back.pac size:%d\r\n",location);
			write_location(&location);
			fclose(fp1);
		}else pc.printf("open /sd/back.pac error!\r\n");
	}
	pc.printf("start record\r\n");
	FILE* fp2 = fopen(data_path, "ab");
	pc.printf("fopen ok!\r\n");
	//if((rel = setvbuf(fp2,NULL,_IONBF,0)) !=0)
		//pc.printf("setvbuf data error!\r\n");
	if(fp2) 
	{
		fseek(fp2, 0L, SEEK_END);
		file_size = ftell(fp2);
		while(file_size > SD_FILE_SIZE)
		{
			fclose(fp2);
			set_path(count);
			fp2 = fopen(data_path, "a+");
			if(fp2)
			{
				fseek(fp2, 0L, SEEK_END);
				file_size = ftell(fp2);
				pc.printf("file:%s size:%d\r\n", data_path,file_size);
			}
			else 
				pc.printf("Open file:%s error!",data_path);
			count++;
		}
		file_write = fwrite(submit, 1, length, fp2);
		pc.printf("write data!\r\n");
		if(file_write == 0) pc.printf("SD write error!code\r\n");
		else 
		{
			if((rel = fflush(fp2))!= 0)
				pc.printf("fflush error\r\n");
		}
		rel = fseek(fp2, 0L, SEEK_END);
		if(rel != 0)
			pc.printf("fseek error!\r\n");
		file_size = ftell(fp2);
		pc.printf("file:%s size:%d\r\n", data_path,file_size);
		fclose(fp2);
	}else pc.printf("SD open error!code\r\n");
}

/*****************************************************************************
** Function name:	data_return
** Description:		Send the saved data when network is unreachable to server.
** Parameters:		None
** Returned value:	The length of sended message.
******************************************************************************/
int data_return(int location)
{
	int len = 0;
	char read_buf[512];
	memset(read_buf,0,512);
	pc.printf("Send the data back!\r\n");
	FILE *fp;
	fp = fopen("/sd/back.pac","rb");
	if(fp)
	{
		//read_location(&location);
		fseek(fp,location - 2,SEEK_SET);
		u8 Len[2];
		fread(Len,2,1,fp);  
		len = Len[0]*256 + Len[1];			//Store with little endian
		//pc.printf("Len is %X %X\r\n",Len[0],Len[1]);
		pc.printf("length of data to re-send is %d\r\n",len);
		location = location- len - 2;
		fseek(fp,location,SEEK_SET);
		fread(sd_buff,len,1,fp);			//read the record to sd_buff
		//pc.printf("location is %d\r\n",location);
		write_location(&location);
		pc.printf("The data needed to return is:\r\n");
		print_buf((u8 *)sd_buff,len);
		fclose(fp);
		if(location< 10)
		{
			pc.printf("No backup data remain\r\n");
		}
	}else pc.printf("Open file:/sd/back.pac error!\r\n");
	delayMS(50);
	return len;
}
