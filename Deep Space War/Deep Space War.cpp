#include "framework.h"
#include "Deep Space War.h"
#include <mmsystem.h>
#include <d2d1.h>
#include <dwrite.h>
#include "D2BMPLOADER.h"
#include "ErrH.h"
#include "FCheck.h"
#include "gifresizer.h"
#include "spacefight.h"
#include <chrono>
#include <clocale>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d2bmploader.lib")
#pragma comment(lib, "errh.lib")
#pragma comment(lib, "fcheck.lib")
#pragma comment(lib, "gifresizer.lib")
#pragma comment(lib, "spacefight.lib")

constexpr int mNew{ 1001 };
constexpr int mLvl{ 1002 };
constexpr int mExit{ 1003 };
constexpr int mSave{ 1004 };
constexpr int mLoad{ 1005 };
constexpr int mHoF{ 1006 };

constexpr int no_record{ 2001 };
constexpr int first_record{ 2002 };
constexpr int record{ 2003 };

constexpr wchar_t bWinClassName[]{ L"SpaceFighter" };

constexpr wchar_t Ltmp_file[]{ L".\\res\\data\\temp.dat" };
constexpr char tmp_file[]{ ".\\res\\data\\temp.dat" };
constexpr wchar_t sound_file[]{ L".\\res\\snd\\main.wav" };
constexpr wchar_t save_file[]{ L".\\res\\data\\save.dat" };
constexpr wchar_t record_file[]{ L".\\res\\data\\record.dat" };
constexpr wchar_t help_file[]{ L".\\res\\data\\help.dat" };

WNDCLASS bWinClass{};
HINSTANCE bIns{ nullptr };
HWND bHwnd{ nullptr };
HMENU bBar{ nullptr };
HMENU bMain{ nullptr };
HMENU bStore{ nullptr };
HDC PaintDc{ nullptr };
HICON bIcon{ nullptr };
HCURSOR bCursor{ nullptr };
HCURSOR outCursor{ nullptr };
POINT cur_pos{};
PAINTSTRUCT bPaint{};
MSG bMsg{};
BOOL bRet{ 0 };

UINT bTimer{ 0 };

D2D1_RECT_F b1Rect{ 50.0f, 10.0f, scr_width / 3.0f - 50.0f, 40.0f };
D2D1_RECT_F b2Rect{ scr_width / 3.0f, 10.0f, scr_width * 2.0f / 3.0f - 50.0f, 40.0f };
D2D1_RECT_F b3Rect{ scr_width * 2.0f / 3.0f, 10.0f, scr_width - 50.0f, 40.0f };

D2D1_RECT_F b1TxtRect{ 80.0f, 15.0f, scr_width / 3.0f - 50.0f, 40.0f };
D2D1_RECT_F b2TxtRect{ scr_width / 3.0f + 30.0f, 15.0f, scr_width * 2.0f / 3.0f - 50.0f, 40.0f };
D2D1_RECT_F b3TxtRect{ scr_width * 2.0f / 3.0f + 30.0f, 15.0f, scr_width - 50.0f, 40.0f };

bool pause{ false };
bool sound{ true };
bool in_client{ true };
bool show_help{ false };
bool b1Hglt{ false };
bool b2Hglt{ false };
bool b3Hglt{ false };

bool name_set{ false };

wchar_t current_player[16]{ L"TARLYO" };

float scale_x{ 0 };
float scale_y{ 0 };

float level{ 1.0f };
int score{ 0 };
int mins{ 0 };
int secs{ 0 };

ID2D1Factory* iFactory{ nullptr };
ID2D1HwndRenderTarget* Draw{ nullptr };

ID2D1SolidColorBrush* statBrush{ nullptr };
ID2D1SolidColorBrush* txtBrush{ nullptr };
ID2D1SolidColorBrush* inactBrush{ nullptr };
ID2D1SolidColorBrush* hgltBrush{ nullptr };

ID2D1SolidColorBrush* backBrush{ nullptr };
ID2D1SolidColorBrush* lifeBrush{ nullptr };

ID2D1RadialGradientBrush* b1BckgBrush{ nullptr };
ID2D1RadialGradientBrush* b2BckgBrush{ nullptr };
ID2D1RadialGradientBrush* b3BckgBrush{ nullptr };

IDWriteFactory* iWriteFactory{ nullptr };
IDWriteTextFormat* nrmText{ nullptr };
IDWriteTextFormat* midText{ nullptr };
IDWriteTextFormat* bigText{ nullptr };

ID2D1Bitmap* bmpAssetArmor{ nullptr };
ID2D1Bitmap* bmpAssetLife{ nullptr };
ID2D1Bitmap* bmpAssetShot{ nullptr };

ID2D1Bitmap* bmpEvil1{ nullptr };
ID2D1Bitmap* bmpEvil2{ nullptr };
ID2D1Bitmap* bmpEvil3{ nullptr };
ID2D1Bitmap* bmpEvil4{ nullptr };

ID2D1Bitmap* bmpLevel{ nullptr };
ID2D1Bitmap* bmpLogo{ nullptr };
ID2D1Bitmap* bmpWin{ nullptr };
ID2D1Bitmap* bmpLoose{ nullptr };
ID2D1Bitmap* bmpWorldRecord{ nullptr };

ID2D1Bitmap* bmpIntro[48]{ nullptr };
ID2D1Bitmap* bmpSpace[49]{ nullptr };

ID2D1Bitmap* bmpMeteor1[10]{ nullptr };
ID2D1Bitmap* bmpMeteor2[5]{ nullptr };
ID2D1Bitmap* bmpMeteor3[20]{ nullptr };

ID2D1Bitmap* bmpHero[6]{ nullptr };
ID2D1Bitmap* bmpShot[12]{ nullptr };

////////////////////////////////////////////////////////




///////////////////////////////////////////////////////

template<typename T>concept HasRelease = requires(T check)
{
	check.Release();
};
template<HasRelease T>bool FreeMem(T** var)
{
	if ((*var))
	{
		(*var)->Release();
		(*var) = nullptr;
		return true;
	}

	return false;
}

void LogErr(const wchar_t* what)
{
	std::wofstream err(L".\\res\\data\\error.log", std::ios::app);

	err << what << L" time stamp: " << std::chrono::system_clock::now() << std::endl;

	err.close();
}
void ReleaseResources()
{
	if (!FreeMem(&iFactory))LogErr(L"Error releasing D2D1 main Factory !");
	if (!FreeMem(&Draw))LogErr(L"Error releasing main D2D1 HwndRenderTarget !");
	if (!FreeMem(&statBrush))LogErr(L"Error releasing solid D2D1 statBrush !");
	if (!FreeMem(&txtBrush))LogErr(L"Error releasing solid D2D1 txtBrush !");
	if (!FreeMem(&inactBrush))LogErr(L"Error releasing solid D2D1 inactBrush !");
	if (!FreeMem(&hgltBrush))LogErr(L"Error releasing solid D2D1 hgltBrush !");
	if (!FreeMem(&backBrush))LogErr(L"Error releasing solid D2D1 backBrush !");
	if (!FreeMem(&lifeBrush))LogErr(L"Error releasing solid D2D1 lifeBrush !");

	if (!FreeMem(&b1BckgBrush))LogErr(L"Error releasing radial D2D1 b1BckgBrush !");
	if (!FreeMem(&b2BckgBrush))LogErr(L"Error releasing radial D2D1 b2BckgBrush !");
	if (!FreeMem(&b3BckgBrush))LogErr(L"Error releasing radial D2D1 b3BckgBrush !");

	if (!FreeMem(&iWriteFactory))LogErr(L"Error releasing D2D1 main WriteFactory !");
	if (!FreeMem(&nrmText))LogErr(L"Error releasing D2D1 nrmText !");
	if (!FreeMem(&midText))LogErr(L"Error releasing D2D1 midText !");
	if (!FreeMem(&bigText))LogErr(L"Error releasing D2D1 bigText !");

	if (!FreeMem(&bmpAssetArmor))LogErr(L"Error releasing D2D1 bmpAssetArmor !");
	if (!FreeMem(&bmpAssetLife))LogErr(L"Error releasing D2D1 bmpAssetLife !");
	if (!FreeMem(&bmpAssetShot))LogErr(L"Error releasing D2D1 bmpAssetShot !");

	if (!FreeMem(&bmpEvil1))LogErr(L"Error releasing D2D1 bmpEvil1 !");
	if (!FreeMem(&bmpEvil2))LogErr(L"Error releasing D2D1 bmpEvil2 !");
	if (!FreeMem(&bmpEvil3))LogErr(L"Error releasing D2D1 bmpEvil3 !");
	if (!FreeMem(&bmpEvil4))LogErr(L"Error releasing D2D1 bmpEvil4 !");

	if (!FreeMem(&bmpLogo))LogErr(L"Error releasing D2D1 bmpLogo !");
	if (!FreeMem(&bmpLevel))LogErr(L"Error releasing D2D1 bmpLevel !");
	if (!FreeMem(&bmpLoose))LogErr(L"Error releasing D2D1 bmpLoose !");
	if (!FreeMem(&bmpWin))LogErr(L"Error releasing D2D1 bmpWin !");
	if (!FreeMem(&bmpWorldRecord))LogErr(L"Error releasing D2D1 bmpWorldRecord !");

	for (int i = 0; i < 48; ++i)if (!FreeMem(&bmpIntro[i]))LogErr(L"Error releasing D2D1 bmpIntro !");
	for (int i = 0; i < 49; ++i)if (!FreeMem(&bmpSpace[i]))LogErr(L"Error releasing D2D1 bmpSpace !");

	for (int i = 0; i < 10; ++i)if (!FreeMem(&bmpMeteor1[i]))LogErr(L"Error releasing D2D1 bmpMeteor1 !");
	for (int i = 0; i < 5; ++i)if (!FreeMem(&bmpMeteor2[i]))LogErr(L"Error releasing D2D1 bmpMeteor2 !");
	for (int i = 0; i < 20; ++i)if (!FreeMem(&bmpMeteor3[i]))LogErr(L"Error releasing D2D1 bmpMeteor3 !");

	for (int i = 0; i < 6; ++i)if (!FreeMem(&bmpHero[i]))LogErr(L"Error releasing D2D1 bmpHero !");
	for (int i = 0; i < 12; ++i)if (!FreeMem(&bmpShot[i]))LogErr(L"Error releasing D2D1 bmpShot !");
}
void ErrExit(int what)
{
	MessageBeep(MB_ICONERROR);
	MessageBox(NULL, ErrHandle(what), L"Критична грешка !", MB_OK | MB_APPLMODAL | MB_ICONERROR);

	std::remove(tmp_file);
	ReleaseResources();
	exit(1);
}













int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	_wsetlocale(LC_ALL, L"");

	bIns = hInstance;
	if (!bIns)
	{
		LogErr(L"Windows hInstance is NULL !");
		ErrExit(eClass);
	}










	std::remove(tmp_file);
	ReleaseResources();

    return (int) bMsg.wParam;
}