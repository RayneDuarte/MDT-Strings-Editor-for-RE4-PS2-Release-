//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "main.h"
#include "read_mdt_file.h"
#include "import_to_mdt_file.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------


void __fastcall TForm1::open_mdt_fileClick(TObject *Sender)
{
	selected_file_path->Caption = "";
	OpenFile->FileName = "";
	OpenFile->Execute();
	if (OpenFile->FileName != "")
	{
		if (OpenFile->FileName.Length() > 45)
		{
			wchar_t *str = OpenFile->FileName.c_str();
			selected_file_path->Caption = *str++;
			int i = 0;
			while (i < 45)
			{
				selected_file_path->Caption += *str++;
				i++;
			}
			selected_file_path->Caption += "...";
		}
		else
			selected_file_path->Caption = OpenFile->FileName;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::unpack_mdtClick(TObject *Sender)
{
	CheckFile(OpenFile, 'r');
}
//---------------------------------------------------------------------------

void CheckFile(TOpenDialog *OpenFile, char mode)
{
	if (OpenFile->FileName != "")
	{
		if (FileExists(OpenFile->FileName))
		{
			String name, fullpath, path;
			fullpath = OpenFile->FileName;
			GetFileName(fullpath, name);
			GetPath(fullpath, path);
			bool ret;

			TFileStream *mdt;
			mdt = new TFileStream(fullpath, fmOpenRead);
			if (mode == 'r')
				ret = Dump_Multi_MDT(mdt, name, path);
			else
				ret = Insert_Multi_MDT(mdt, name, path);
			if (ret) ShowMessage("Finished");
			else
				delete mdt;
		}
	}
}

void GetFileName(String &path, String &filename)
{
	wchar_t *str = path.c_str();
	while (*str != 0)
		str++;
	while (*str != '\\')
		str--;
	str++;
	filename = *str++;
	while (*str != 0)
		filename += *str++;
}

void GetPath(String &fullpath, String &path)
{
	wchar_t *str = fullpath.c_str();
	int len = fullpath.Length();

	while (fullpath[len] != L'\\')
		len--;
	path = *str++;
	int i = 1;

	while (i < len)
	{
		path += *str++;
		i++;
	}
}

void __fastcall TForm1::repack_mdtClick(TObject *Sender)
{
	CheckFile(OpenFile, 'w');
}
//---------------------------------------------------------------------------

