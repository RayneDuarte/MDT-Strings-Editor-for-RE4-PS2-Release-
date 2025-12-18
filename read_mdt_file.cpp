//---------------------------------------------------------------------------

#pragma hdrstop

#include <System.IOUtils.hpp>
#include "main.h"
#include "read_mdt_file.h"
#include "import_to_mdt_file.h"
#include "aux_functions.h"
#include "CHARMAP/export_map.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

wchar_t LINEBREAK[2] = {0x0A, 0x00};
wchar_t ENDSTR[2] = {0x24, 0x00};
wchar_t QUOTATIONMARK[2] = {0x22, 0x00};

charmap chars[CHARS_NUM];

bool Dump_Multi_MDT(TFileStream *mdt, String &filename, String &path)
{
	InitialiseExportCharmap(chars);
	int off, is_text;
	TFileStream *txt;

	if (Check_MDT_Version(mdt) != 6)
	{
		Dump_Single_MDT(mdt, filename, path);
		return true;
	}

	mdt->Seek(4, soFromBeginning);
	mdt->Read(&off, 4);
	mdt->Seek(off, soFromBeginning);
	mdt->Read(&is_text, 4);
	if (is_text == 0xCDCDCDCD)
	{
		ShowMessage("Não há textos no arquivo MDT " + filename + "!");
		return false;
	}

	TXT_FILES TXT;
	Generate_TXT_Files(&TXT, filename, path, 'w');
	mdt->Seek(8, soFromBeginning);

	int t = 0;
	while (t < TEXTS_BLOCK)
	{
		TFileStream *txt = TXT.TXT[t];

		int main_table_off = mdt->Position; //início do ponteiro de um bloco de texto
		main_table_off += 4; //início do ponteiro do próximo bloco de texto
		mdt->Read(&off, 4); //lê o ponteiro do bloco
		int block_off = off; //o início do bloco é usado para encontrar os textos nele de forma relativa
		off += 4; //ignora os 4 bytes iniciais do bloco
		mdt->Seek(off, soFromBeginning);
		int str_count, str = 0;
		mdt->Read(&str_count, 4);

		while (str < str_count)
		{ //str tem a quantidade de strings que serão extraídas do bloco
			mdt->Read(&off, 4);
			int next_str = mdt->Position; //ponteiro da próxima string

			//abaixo é calculado o início de ambas as strings
			off = (off + block_off) + 2;
			mdt->Seek(off, soFromBeginning);
			Convert_Chars(mdt, txt);

			txt->Write(ENDSTR, 2);
			txt->Write(LINEBREAK, 2);
			mdt->Seek(next_str, soFromBeginning);
			str++;
		}
		delete txt;
		t++;
		mdt->Seek(main_table_off, soFromBeginning);
	}
    delete mdt;
    return true;
}

void Dump_Single_MDT(TFileStream *mdt, String &filename, String &path)
{
	int str_count, str = 0, next_str;
	mdt->Seek(4, soFromBeginning);
	mdt->Read(&str_count, 4);

	String txt_path = path + filename;
	txt_path += ".TXT";
	DeleteFile(txt_path);

	TFileStream *txt = new TFileStream(txt_path, fmCreate);

	while (str < str_count)
	{ //str tem a quantidade de strings que serão extraídas do bloco
		int off;
		mdt->Read(&off, 4);
		off += 2;
		next_str = mdt->Position; //início da próxima string
		mdt->Seek(off, soFromBeginning);
		Convert_Chars(mdt, txt);

		txt->Write(ENDSTR, 2);
		txt->Write(LINEBREAK, 2);
		mdt->Seek(next_str, soFromBeginning);
		str++;
	}
	delete txt;
    delete mdt;
}

void Convert_Chars(TFileStream *mdt, TFileStream *txt)
{
	while (1)
	{
		short int ch;
		mdt->Read(&ch, 2); //os caracteres são em formato UTF-16, mas são mapeados no jogo usando uma sequência de bytes diferentes do UTF-16 padrão

		if (ch >= 167 && ch <= 294)
		{
			SaveToStr(txt, chars[ch].str);
		}
		else if (ch == 156 || ch == 157)
		{
			txt->Write(QUOTATIONMARK, 2);
		}
		else if (ch == 3 || ch == 4)
		{
			SaveToStr(txt, chars[ch].str);
		}
		else if (ch >= 128 && ch <= 150)
		{
			SaveToStr(txt,chars[ch].str);
		}
		else if (ch >= 152 && ch <= 164)
		{
			SaveToStr(txt, chars[ch].str);
		}
		else if (ch >= 296 && ch <= 299)
		{
			SaveToStr(txt, chars[ch].str);
		}
		else if (ch >= 301 && ch <= 309)
		{
			SaveToStr(txt, chars[ch].str);
		}
		else if (ch == 8)
		{ //verifica se chegou no fim do texto
			mdt->Read(&ch, 2);
			if (ch == 1) return;
			else
			{
				ch = 8;
				int off = mdt->Position - 2;
				mdt->Seek(off, soFromBeginning);
			}
		}
		else if (ch == 6)
		{ //códigos de cores
			mdt->Read(&ch, 2);
			ch = ((6 << 8) | ch) - 1531;
			SaveToStr(txt, chars[ch].str);
		//	ShowMessage(ch);
        //    ShowMessage(chars[ch].str);
		}
		else if (ch <= 255)
		{
			String hex = "0x";
			hex += IntToHex(ch);
			SaveToStr(txt, hex.c_str());
		}
		else
		{
			String hex = "0x";
			hex += IntToHex(ch);
			SaveToStr(txt, hex.c_str());

			if ((int)ch == 0xFFFF)
			{
				mdt->Read(&ch, 2);
				if (ch == 4)
					SaveToStr(txt, chars[ch].str);
				else
				{
					hex = "0x";
					hex += IntToHex(ch);
					SaveToStr(txt, hex.c_str());
				}
			}
		}
	}
}

void Generate_TXT_Files(TXT_FILES *TXT, String &filename, String &path, char mode)
{
	int txt_count = 0;
	while (txt_count < 5)
	{
		String txt_path = path;
		txt_path += "0";
		txt_path += IntToStr(txt_count + 1);
		txt_path += "_";
		txt_path += filename;
		txt_path += ".TXT";

		if (mode == 'w')
		{
			DeleteFile(txt_path);
			TFileStream *f = new TFileStream(txt_path, fmCreate);
			TXT->TXT[txt_count] = f;
		}
		else
		{
			TFileStream *f = new TFileStream(txt_path, fmOpenRead);
			TXT->TXT[txt_count] = f;
		}
		txt_count++;
	}
}

void Close_TXT_Files(TXT_FILES *TXT)
{
	int txt_count = 0;
	while (txt_count < TEXTS_BLOCK)
	{
		delete TXT->TXT[txt_count];
		txt_count++;
	}
}

