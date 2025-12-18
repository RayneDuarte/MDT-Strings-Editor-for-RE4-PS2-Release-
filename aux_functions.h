//---------------------------------------------------------------------------

#ifndef aux_functionsH
#define aux_functionsH
//---------------------------------------------------------------------------
#endif
#include <System.IOUtils.hpp>

void SwapTo32LE(unsigned char *data, int n);
short int Check_MDT_Version(TFileStream *mdt);
void SaveToStr(TFileStream *txt, wchar_t *str);
void SaveToMDT(TFileStream *mdt, wchar_t sym);
void Padding(TFileStream *new_file);
int HexToInt(String &hex_str);

