#ifndef _SETTING_H_
#define _SETTING_H_

#ifdef __cplusplus
extern "C"
{
#endif



typedef struct
{
  int  verbose;
  int  quiet;
  int  help;
  int  version;
  int  error;
  int  length;
  int  simple;
  int  index;
  int  amount;
  int  append;
  char* appendarg;
  char filename[ 128 ];
} tSetting;

void Setting_Init(tSetting* as);
int Setting_Parse( tSetting* as ,int argc, char** argv);

#ifdef __cplusplus
}
#endif

#endif