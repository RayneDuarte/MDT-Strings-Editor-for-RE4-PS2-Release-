//---------------------------------------------------------------------------

#pragma hdrstop

#include "import_to_mdt_file.h"
#include "read_mdt_file.h"
#include "aux_functions.h"
#include "CHARMAP/import_map.h"
#include <vector>
//---------------------------------------------------------------------------
#pragma package(smart_init)

byte MDT_START_STR[2]={0};
byte MDT_END_STR[4]={0x08, 0x00, 0x01, 0x00};
byte NEXT_PAGE[4]={0x08, 0x00, 0x04, 0x00};
byte WHITE_COLOR[4]={0x06, 0x00, 0x00, 0x00};
byte WHITE_COLOR1[4]={0x06, 0x00, 0x01, 0x00};
byte WHITE_COLOR2[4]={0x06, 0x00, 0x02, 0x00};
byte WHITE_COLOR3[4]={0x06, 0x00, 0x03, 0x00};
byte WHITE_COLOR4[4]={0x06, 0x00, 0x04, 0x00};
byte GREY_COLOR[4]={0x06, 0x00, 0x05, 0x00};
byte GREEN_COLOR[4]={0x06, 0x00, 0x07, 0x00};
byte YELLOW_COLOR[4]={0x06, 0x00, 0x0A, 0x00};
byte RED_COLOR[4]={0x06, 0x00, 0x0B, 0x00};
byte TXT_SHADOW[4]={0x06, 0x00, 0x0C, 0x00};
byte CROSS_BTN[2]={0x30, 0x01};
byte SQUARE_BTN[2]={0x31, 0x01};
byte CIRCLE_BTN[2]={0x32, 0x01};
byte TRIANGLE_BTN[2]={0x33, 0x01};

import_table *table;

bool Insert_Multi_MDT(TFileStream *mdt, String &filename, String &path)
{
	InitialiseImportTable(&table);
	if (table == nullptr) return false;

	if (Check_MDT_Version(mdt) != 6)
	{
		return (Build_Single_MDT(mdt, filename, path));
	}

	unsigned char *buff, data[4];
	int len, txt_count = 0, blocks_table = 8;
	int k, block_start, str_count;
	int pointers_table_len, next_str;
	int off, pointer;

	mdt->Seek(8, soFromBeginning);
	mdt->Read(&len, 4);
	buff = new unsigned char [len];
	if (buff == nullptr)
		return ErrorMsg(nullptr, 0);

	mdt->Seek(0, soFromBeginning);
	mdt->ReadBuffer(buff, len);

	TFileStream *new_file;
	String new_mdt = path + "NEW_";
	new_file = new TFileStream(new_mdt, fmCreate);
	new_file->WriteBuffer(buff, len);
	delete[] buff;

	TXT_FILES TXT;
	Generate_TXT_Files(&TXT, filename, path, 'r');
	bool missing_txt = false;

	while (txt_count < TEXTS_BLOCK)
	{ //verifica os 5 arquivos de idiomas extraídos pela função no arquivo "read_mdt_file.c"
		if (TXT.TXT[txt_count] == nullptr)
		{
			missing_txt = true;
			break;
		}
		txt_count++;
	}
	if (missing_txt)
	{
		txt_count = 0;
		while (txt_count < TEXTS_BLOCK)
		{
			if (TXT.TXT[txt_count] != nullptr)
				delete TXT.TXT[txt_count];
			txt_count++;
		}
		return ErrorMsg(new_file, 1);
	}

	txt_count = 0;
	std::vector <pointers_table> group;
	mdt->Seek(8, soFromBeginning);

	while (txt_count < TEXTS_BLOCK)
	{
		block_start = new_file->Position;
		mdt->Read(&pointer, 4); //lê o ponteiro que aponta para o cabeçalho do bloco do mdt original
		mdt->Seek(pointer, soFromBeginning);
		mdt->ReadBuffer(data, 4); //lê o cabeçalho do bloco do mdt original
		new_file->WriteBuffer(data, 4); //salva o cabeçalho lido no novo mdt
		mdt->ReadBuffer(data, 4); //lê o número de strings do bloco no mdt original
		new_file->WriteBuffer(data, 4);
		str_count = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | (data[0]);

		pointers_table_len = str_count * 4; //calcula o tamanho da tabela de ponteiros do bloco no mdt original
		buff = new unsigned char [pointers_table_len];
		if (buff == nullptr)
			return ErrorMsg(new_file, 0);

		//lê a tabela de ponteiros do bloco no mdt original
		mdt->ReadBuffer(buff, pointers_table_len);
		new_file->WriteBuffer(buff, pointers_table_len);
		//grava a tabela lida no novo mdt
		delete[] buff; //desaloca o buffer

		pointers_table table;

		table.str_count = str_count; //número de strings do bloco
		table.block_start = block_start;
		TFileStream *txt = TXT.TXT[txt_count]; //ponteiro do arquivo de texto que vai ser lido e inserido no bloco

		k = 0;
		while (k < str_count)
		{ //faz a inserção das strings no bloco selecionado apartir de um arquivo .txt

			String str;
			GetText(txt, str);

			next_str = txt->Position + 2; //início da próxima string
			off = new_file->Position - block_start;
			table.pointers.push_back(off);

			//cada string no jogo inicia com 0x0000
			new_file->WriteBuffer(MDT_START_STR, 2);
			Convert_To_MDT(new_file, str);

			//adiciona a endstring após a string que foi inserida
			new_file->WriteBuffer(MDT_END_STR, 4);

			if (txt->Position != txt->Size)
				txt->Seek(next_str, soFromBeginning); //posiciona o cursor no início da próxima string
			k++; //conta o número de strings inseridas
		}
		group.push_back(table);
		txt_count++; //um bloco de texto foi inserido
		delete txt; //fecha o arquivo txt que foi lido para inserção
		/*adiciona bytes nulos de preenchimento até
		chegar em um offset múltiplo de 16 para em
		seguida inserir mais 12 bytes de preenchimento*/
		Padding(new_file);

		if (txt_count != TEXTS_BLOCK)
		{
			off = 0;
			unsigned char zero = 0;
			while (off < 12)
			{ //adiciona 12 bytes nulos antes do próximo bloco que será inserido
				new_file->Write(&zero, 1);
				off++;
			}
		}
		blocks_table += 4;
		mdt->Seek(blocks_table, soFromBeginning);
	}
	delete mdt;
	delete new_file;
	delete[] table;

	new_file = new TFileStream(new_mdt, fmOpenReadWrite);
	txt_count = 0;
	k = 0;
	blocks_table = 8;
	new_file->Seek(blocks_table, soFromBeginning);

	while (txt_count < TEXTS_BLOCK)
	{//atualiza os ponteiros dos blocos e de cada string dentro deles

		SwapTo32LE(data, group[txt_count].block_start);
		new_file->WriteBuffer(data, 4);
		off = group[txt_count].block_start + 8;
		new_file->Seek(off, soFromBeginning);

		while (k < group[txt_count].str_count)
		{
			SwapTo32LE(data, group[txt_count].pointers[k]);
			new_file->WriteBuffer(data, 4);
			k++;
		}
		k = 0;
		blocks_table += 4;
		new_file->Seek(blocks_table, soFromBeginning);
		txt_count++;
	}
	delete new_file;
	String old_mdt = path + filename;
	DeleteFile(old_mdt);
	RenameFile(new_mdt, old_mdt);
	return true;
}

bool Build_Single_MDT(TFileStream *mdt, String &filename, String &path)
{
	String text_file;
	int pos = filename.Pos(".");
	if (pos > 0)
	{
		text_file = path;
		wchar_t *name = filename.c_str();
		text_file += *name++;
		for (int i = 1; i < pos; i++)
			text_file += *name++;

		text_file += "MDT.TXT";
	}
	else
	{
		delete[] table;
		return false;
	}

	TFileStream *txt = new TFileStream(text_file, fmOpenRead);
	if (txt == nullptr)
	{
		ShowMessage("O arquivo " + text_file + " com os textos não foi encontrado!");
		delete[] table;
		return false;
	}

	String new_mdt = path;
	new_mdt += "NEW_";
	TFileStream *new_file;
	new_file = new TFileStream(new_mdt, fmCreate);
	mdt->Seek(4, soFromBeginning);
	int str_count;
	mdt->Read(&str_count, 4);
	int table_len = (str_count * 4) + 8;
	mdt->Seek(0, soFromBeginning);

	unsigned char *buff;
	buff = new unsigned char [table_len];
	if (buff == nullptr)
		return ErrorMsg(new_file, 0);

	mdt->ReadBuffer(buff, table_len);
	new_file->WriteBuffer(buff, table_len);
	delete[] buff;
	delete mdt;

	std::vector <int> pointers;
	int k = 0;

	while (k < str_count)
	{
		String str;
		GetText(txt, str);
		pointers.push_back((new_file->Position));
		int next_str = txt->Position + 2; //início da próxima string

		//cada string no jogo inicia com 0x0000
		new_file->WriteBuffer(MDT_START_STR, 2);
		Convert_To_MDT(new_file, str);
		new_file->WriteBuffer(MDT_END_STR, 4);

		if (txt->Position != txt->Size)
			txt->Seek(next_str, soFromBeginning);
		k++;
	}
	Padding(new_file);
	delete txt;
	delete new_file;
	delete[] table;

	new_file = new TFileStream(new_mdt, fmOpenReadWrite);
	new_file->Seek(8, soFromBeginning);
	k = 0;
	unsigned char data[4];

	while (k < str_count)
	{//atualiza os ponteiros das strings
		SwapTo32LE(data, pointers[k]);
		new_file->WriteBuffer(data, 4);
		k++;
	}
	delete new_file;
	String old_mdt = path + filename;
	DeleteFile(old_mdt);
	RenameFile(new_mdt, old_mdt);
	return true;
}

void Convert_To_MDT(TFileStream *new_file, String &string)
{
	wchar_t *str = string.c_str();

	while (1)
	{ //faz a inserção de uma string completa

		if (*str == '$') return;

		if (*str++ == '0' && *str == 'x')
		{  //faz a conversão de bytes raw que foram salvos em formato hex durante a extração
			str++;
			String hex = *str++;
			for (int i = 0; i <= 2; i++)
				hex += *str++;

			int n = HexToInt(hex);
			unsigned char data[2];
			data[0] = n;
			data[1] = n >> 8;
			new_file->Write(data, 2);
		}
		else
		{
			str--;
			if (*str == '[')
			{ //verifica strings que representam códigos de controle
				wchar_t s[10];
				memcpy(s, str + 1, 20);

				if (s[0] == 'N' && s[1] == 'E' && s[2] == 'X' && s[3] == 'T')
				{
					new_file->Write(NEXT_PAGE, 4);
					str += 6;
				}
				else if (s[0] == 'W' && s[1] == 'H' && s[2] == 'I' && s[3] == 'T' && s[4] == 'E' && s[5] == '_' && s[6] == 'C' && s[7] == '1')
				{
					new_file->Write(WHITE_COLOR1, 4);
					str += 10;
				}
				else if (s[0] == 'W' && s[1] == 'H' && s[2] == 'I' && s[3] == 'T' && s[4] == 'E' && s[5] == '_' && s[6] == 'C' && s[7] == '2')
				{
					new_file->Write(WHITE_COLOR2, 4);
					str += 10;
				}
				else if (s[0] == 'W' && s[1] == 'H' && s[2] == 'I' && s[3] == 'T' && s[4] == 'E' && s[5] == '_' && s[6] == 'C' && s[7] == '3')
				{
					new_file->Write(WHITE_COLOR3, 4);
					str += 10;
				}
				else if (s[0] == 'W' && s[1] == 'H' && s[2] == 'I' && s[3] == 'T' && s[4] == 'E' && s[5] == '_' && s[6] == 'C' && s[7] == '4')
				{
					new_file->Write(WHITE_COLOR4, 4);
					str += 10;
				}
				else if (s[0] == 'W' && s[1] == 'H' && s[2] == 'I' && s[3] == 'T')
				{
					new_file->Write(WHITE_COLOR, 4);
					str += 9;
				}
				else if (s[0] == 'G' && s[1] == 'R' && s[2] == 'E' && s[3] == 'Y')
				{
					new_file->Write(GREY_COLOR, 4);
					str += 8;
				}
				else if (s[0] == 'G' && s[1] == 'R' && s[2] == 'E' && s[3] == 'E')
				{
					new_file->Write(GREEN_COLOR, 4);
					str += 9;
				}
				else if (s[0] == 'Y' && s[1] == 'E' && s[2] == 'L' && s[3] == 'L')
				{
					new_file->Write(YELLOW_COLOR, 4);
					str += 10;
				}
				else if (s[0] == 'R' && s[1] == 'E' && s[2] == 'D' && s[3] == '_')
				{
					new_file->Write(RED_COLOR, 4);
					str += 7;
				}
				else if (s[0] == 'T' && s[1] == 'X' && s[2] == 'T' && s[3] == '_')
				{
					new_file->Write(TXT_SHADOW, 4);
					str += 12;
				}
				else if (s[0] == 'X' && s[1] == '_' && s[2] == 'B' && s[3] == 'U')
				{
					new_file->Write(CROSS_BTN, 2);
					str += 10;
				}
				else if (s[0] == 'S' && s[1] == 'Q' && s[2] == 'U' && s[3] == 'A')
				{
					new_file->Write(SQUARE_BTN, 2);
					str += 8;
				}
				else if (s[0] == 'C' && s[1] == 'I' && s[2] == 'R' && s[3] == 'C')
				{
					new_file->Write(CIRCLE_BTN, 2);
					str += 8;
				}
				else if (s[0] == 'T' && s[1] == 'R' && s[2] == 'I' && s[3] == 'A')
				{
					new_file->Write(TRIANGLE_BTN, 2);
					str += 10;
				}
				else
				{
					new_file->Write(new_file, table[*str].game_ch);
					str++;
				}
			}
			else
			{
				SaveToMDT(new_file, table[*str].game_ch);
				str++;
			}
		}
	}
}

void GetText(TFileStream *f, String &str)
{
	wchar_t ch;
	f->Read(&ch, 2);
	str = ch;
	if (ch != '$')
	{
		while (1)
		{
			f->Read(&ch, 2);
			str += ch;
			if (ch == '$') return;
		}
	}
}

bool ErrorMsg(TFileStream *f, int msg_id)
{
	delete[] table;
	if (f != nullptr)
		delete f;

	if (!msg_id)
		ShowMessage("Erro de alocação de memória!");
	else if (msg_id)
		ShowMessage("Não foi possível abrir um ou mais arquivos '.TXT'!");

	return false;
}
