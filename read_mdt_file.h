//---------------------------------------------------------------------------

#ifndef read_mdt_fileH
#define read_mdt_fileH
//---------------------------------------------------------------------------
#endif
#include <System.IOUtils.hpp>
#define TEXTS_BLOCK 5

typedef struct
{
	TFileStream *TXT[5];
}
TXT_FILES;

bool Dump_Multi_MDT(TFileStream *mdt, String &filename, String &path);
void Dump_Single_MDT(TFileStream *mdt, String &filename, String &path);
void Generate_TXT_Files(TXT_FILES *TXT, String &filename, String &path, char mode);
void Convert_Chars(TFileStream *mdt, TFileStream *txt);
void Close_TXT_Files(TXT_FILES *TXT);

