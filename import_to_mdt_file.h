//---------------------------------------------------------------------------

#ifndef import_to_mdt_fileH
#define import_to_mdt_fileH
//---------------------------------------------------------------------------
#endif
#include <System.IOUtils.hpp>
#include <vector>
#include "CHARMAP/import_map.h"

bool InitialiseImportTable();
bool Insert_Multi_MDT(TFileStream *mdt, String &filename, String &path);
bool Build_Single_MDT(TFileStream *mdt, String &filename, String &path);
void Convert_To_MDT(TFileStream *new_file, String &string);
void GetText(TFileStream *f, String &str);
bool ErrorMsg(TFileStream *f, int msg_id);

#ifndef pointers_tableH
#define pointers_tableH

typedef struct
{
	std::vector <int> pointers;
	int str_count;
	int block_start;
}
pointers_table;

#endif
