//---------------------------------------------------------------------------

#pragma hdrstop

#include "aux_functions.h"
#include <System.IOUtils.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)

void SwapTo32LE(unsigned char *data, int n)
{
	data[0] = n;
	data[1] = n >> 8;
	data[2] = n >> 16;
	data[3] = n >> 24;
}

short int Check_MDT_Version(TFileStream *mdt)
{
	short int hdr;
	mdt->Read(&hdr, 2);
	if (hdr != 6) return 2;
	return hdr;
}

void SaveToStr(TFileStream *txt, wchar_t *str)
{
	unsigned char c[2];
	while (*str != 0)
	{ //salva cada caractere em UTF-8 LE
		if (*str <= 0xff)
		{
			c[0] = *str;
			c[1] = *str++ >> 8;
		}
		else
		{
			c[0] = *str & 0xff;
			c[1] = *str++ >> 8;
		}
		txt->Write(c, 2);
	}
}

void SaveToMDT(TFileStream *mdt, wchar_t sym)
{
	unsigned char c[2];
	c[0] = sym;
	c[1] = sym >> 8;
	mdt->Write(c, 2);
}

void Padding(TFileStream *new_file)
{
	int off = new_file->Position;
	int r = off / 16;
	if (r * 16 != off)
	{
		unsigned char zero = 0;
		while (1)
		{
			off++;
			r = off / 16;
			new_file->Write(&zero, 1);
			if (r * 16 == off) return;
		}
	}
}

int HexToInt(String &hex_str)
{
	int decimal = 0, base = 1;
	int i, value, length;
	wchar_t *str = hex_str.c_str();
	length = hex_str.Length();

	for(i = length--; i >= 0; i--)
	{
		if(str[i] >= '0' && str[i] <= '9')
		{
			decimal += (str[i] - 48) * base;
			base *= 16;
		}
		else if(str[i] >= 'A' && str[i] <= 'F')
		{
			decimal += (str[i] - 55) * base;
			base *= 16;
		}
		else if(str[i] >= 'a' && str[i] <= 'f')
		{
			decimal += (str[i] - 87) * base;
			base *= 16;
		}
	}
	return decimal;
}

