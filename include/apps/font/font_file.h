#ifndef		__FONT_FILE_H__
#define		__FONT_FILE_H__

int font_file_open(const char* file);
int font_file_seek(long offset);
int font_file_read(void *pdata, long size);
void font_file_close();

#endif