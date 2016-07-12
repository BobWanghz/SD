#ifndef __SD_H__
#define	__SD_H__

#include "mbed.h"
#include "SDFileSystem.h"
#include "sys.h"

#define PIN_MOSI P1_24
#define PIN_MISO P1_23
#define PIN_SCK P1_20
#define PIN_CS P1_21

#define SD_FILE_SIZE 2097152	//2M
/*Those variables is define in main.cpp*/
extern int file_size;
extern int count;
//extern bool flag_backdata;
extern char data_path[20];
extern u16 length;
extern u8 submit[512];
extern Serial pc;
extern char sd_buff[512];
extern bool tcp;
extern bool protocol_test;
extern bool little_endian;
extern void delayMS(int x);
//extern int back_count;
extern char workarea[512];


template<typename T> T change_endian(T p);


void set_path(int txt_num);
void write_to_sd();
int data_return();
void print_buf(char* buf,int buf_len);
int read_location(int* location);
int write_location(int* location);
int create_file(char * file_path);

#endif //__SD_H__
