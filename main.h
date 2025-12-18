//---------------------------------------------------------------------------

#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Imaging.pngimage.hpp>
#include <Vcl.Dialogs.hpp>
//---------------------------------------------------------------------------

void GetFileName(String &path, String &filename);
void GetPath(String &fullpath, String &path);
void CheckFile(TOpenDialog *OpenFile, char mode);

class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TImage *background;
	TButton *unpack_mdt;
	TButton *repack_mdt;
	TImage *open_mdt_file;
	TLabel *selected_file_path;
	TOpenDialog *OpenFile;
	void __fastcall open_mdt_fileClick(TObject *Sender);
	void __fastcall unpack_mdtClick(TObject *Sender);
	void __fastcall repack_mdtClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
