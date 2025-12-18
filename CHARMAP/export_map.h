//---------------------------------------------------------------------------

#ifndef export_mapH
#define export_mapH
//---------------------------------------------------------------------------
#endif

#define CHARS_NUM 320

typedef struct
{
	wchar_t str[20];
}
charmap;

void InitialiseExportCharmap(charmap *chars);
