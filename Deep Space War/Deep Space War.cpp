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

D2D1_RECT_F b1TxtRect{ 90.0f, 12.0f, scr_width / 3.0f - 50.0f, 40.0f };
D2D1_RECT_F b2TxtRect{ scr_width / 3.0f + 45.0f, 12.0f, scr_width * 2.0f / 3.0f - 50.0f, 40.0f };
D2D1_RECT_F b3TxtRect{ scr_width * 2.0f / 3.0f + 55.0f, 12.0f, scr_width - 50.0f, 40.0f };

bool pause{ false };
bool sound{ true };
bool in_client{ true };
bool show_help{ false };
bool b1Hglt{ false };
bool b2Hglt{ false };
bool b3Hglt{ false };

bool level_skipped{ false };

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

struct EXPLOSION
{
	D2D1_POINT_2F center{};

	int frame = 0;
	int max_frames = 23;
	int frame_delay = 4;

	int get_frame()
	{
		frame_delay--;
		if (frame_delay <= 0)
		{
			frame_delay = 4;
			++frame;
		}
	}
};

dll::RANDIT RandIt{};

dll::BACKGROUND Intro(background::intro);
dll::BACKGROUND Field(background::field);

dll::CREATURES* Hero{ nullptr };

std::vector<EXPLOSION>vExplosions;
std::vector<dll::FADING>vAssets;

std::vector<dll::CREATURES*> vHeroShots{ nullptr };
std::vector<dll::CREATURES*> vEvilShots{ nullptr };

std::vector<dll::CREATURES*> vEvils{ nullptr };

std::vector<dll::METEORS*> vMeteors{ nullptr };


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

void GameOver()
{
	KillTimer(bHwnd, bTimer);

	PlaySound(NULL, NULL, NULL);



	bMsg.message = WM_QUIT;
	bMsg.wParam = 0;
}
void InitGame()
{
	wcscpy_s(current_player, L"TARLYO");
	name_set = false;

	level = 1.0f;
	score = 0;
	mins = 0;
	secs = 0;

	level_skipped = false;

	vExplosions.clear();
	vAssets.clear();

	FreeMem(&Hero);
	Hero = dll::CREATURES::create(creatures::hero, 100.0f, RandIt(60.0f, ground - 100.0f));

	if (!vHeroShots.empty())
		for (int i = 0; i < vHeroShots.size(); ++i)FreeMem(&vHeroShots[i]);
	vHeroShots.clear();

	if (!vEvilShots.empty())
		for (int i = 0; i < vEvilShots.size(); ++i)FreeMem(&vEvilShots[i]);
	vEvilShots.clear();

	if (!vEvils.empty())
		for (int i = 0; i < vEvils.size(); ++i)FreeMem(&vEvils[i]);
	vEvils.clear();

	if (!vMeteors.empty())
		for (int i = 0; i < vMeteors.size(); ++i)FreeMem(&vMeteors[i]);
	vMeteors.clear();
}

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
	switch (ReceivedMsg)
	{
	case WM_INITDIALOG:
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)(bIcon));
		return true;

	case WM_CLOSE:
		EndDialog(hwnd, IDCANCEL);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;

		case IDOK:
			if (GetDlgItemText(hwnd, IDC_NAME, current_player, 16) < 1)
			{
				wcscpy_s(current_player, L"TARLYO");

				if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
				MessageBox(bHwnd, L"Ха, ха, хааа ! Забрави си името !", L"Забраватор !", MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);

				EndDialog(hwnd, IDCANCEL);
				break;
			}
			EndDialog(hwnd, IDOK);
			break;
		}
		break;
	}

	return (INT_PTR)(FALSE);
}
LRESULT CALLBACK WinProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
	switch (ReceivedMsg)
	{
	case WM_CREATE:
		if (bIns)
		{
			SetTimer(hwnd, bTimer, 1000, nullptr);

			bBar = CreateMenu();
			bMain = CreateMenu();
			bStore = CreateMenu();

			AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bMain), L"Основно меню");
			AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bStore), L"Меню за данни");

			AppendMenu(bMain, MF_STRING, mNew, L"Нова игра");
			AppendMenu(bMain, MF_STRING, mLvl, L"Следващо ниво");
			AppendMenu(bMain, MF_SEPARATOR, NULL, NULL);
			AppendMenu(bMain, MF_STRING, mExit, L"Изход");

			AppendMenu(bStore, MF_STRING, mSave, L"Запази игра");
			AppendMenu(bStore, MF_STRING, mLoad, L"Зареди игра");
			AppendMenu(bStore, MF_SEPARATOR, NULL, NULL);
			AppendMenu(bStore, MF_STRING, mHoF, L"Зьль на славата");

			SetMenu(hwnd, bBar);

			InitGame();
		}
		break;

	case WM_CLOSE:
		if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
		pause = true;
		if (MessageBox(hwnd, L"Ако излезеш, губиш тази игра !\n\nНаистина ли излизаш ?",
			L"Изход", MB_YESNO | MB_APPLMODAL | MB_ICONINFORMATION) == IDNO)
		{
			pause = false;
			break;
		}
		GameOver();
		break;
		
	case WM_TIMER:
		if (pause)break;
		--secs;
		mins = secs / 60;
		break;

	case WM_PAINT:
		PaintDc = BeginPaint(hwnd, &bPaint);
		FillRect(PaintDc, &bPaint.rcPaint, CreateSolidBrush(RGB(10, 10, 10)));
		EndPaint(hwnd,&bPaint);
		break;

	case WM_SETCURSOR:
		GetCursorPos(&cur_pos);
		ScreenToClient(hwnd, &cur_pos);
		if (LOWORD(lParam) == HTCLIENT)
		{
			if (!in_client)
			{
				in_client = true;
				pause = false;
			}

			if (cur_pos.y * scale_y <= 50)
			{
				if (cur_pos.x * scale_x >= b1Rect.left && cur_pos.x * scale_x <= b1Rect.right)
				{
					if (!b1Hglt)
					{
						if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
						b1Hglt = true;
						b2Hglt = false;
						b3Hglt = false;
					}
				}
				else if (cur_pos.x * scale_x >= b2Rect.left && cur_pos.x * scale_x <= b2Rect.right)
				{
					if (!b2Hglt)
					{
						if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
						b1Hglt = false;
						b2Hglt = true;
						b3Hglt = false;
					}
				}
				else if (cur_pos.x * scale_x >= b3Rect.left && cur_pos.x * scale_x <= b3Rect.right)
				{
					if (!b3Hglt)
					{
						if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
						b1Hglt = false;
						b2Hglt = false;
						b3Hglt = true;
					}
				}
				else if (b1Hglt || b2Hglt || b3Hglt)
				{
					if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
					b1Hglt = false;
					b2Hglt = false;
					b3Hglt = false;
				}

				SetCursor(outCursor);
				return true;
			}
			else if (b1Hglt || b2Hglt || b3Hglt)
			{
				if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
				b1Hglt = false;
				b2Hglt = false;
				b3Hglt = false;
			}

			SetCursor(bCursor);
			return true;
		}
		else
		{
			if (in_client)
			{
				in_client = false;
				pause = true;
			}

			if (b1Hglt || b2Hglt || b3Hglt)
			{
				if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
				b1Hglt = false;
				b2Hglt = false;
				b3Hglt = false;
			}

			SetCursor(LoadCursor(NULL, IDC_ARROW));
			return true;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case mNew:
			if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
			pause = true;
			if (MessageBox(hwnd, L"Ако рестартираш, губиш тази игра !\n\nНаистина ли рестартираш ?",
				L"Рестарт", MB_YESNO | MB_APPLMODAL | MB_ICONINFORMATION) == IDNO)
			{
				pause = false;
				break;
			}
			GameOver();
			break;

		case mLvl:
			if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
			pause = true;
			if (MessageBox(hwnd, L"Ако прескочиш нивото, губиш бонусите за него !\n\nНаистина ли прескачаш нивото ?",
				L"Следващо ниво", MB_YESNO | MB_APPLMODAL | MB_ICONINFORMATION) == IDNO)
			{
				pause = false;
				break;
			}
			level_skipped = true;
			InitGame();
			break;

		case mExit:
			SendMessage(hwnd, WM_CLOSE, NULL, NULL);
			break;

		}
		break;

	case WM_LBUTTONDOWN:
		if (HIWORD(lParam) <= 50)
		{

		}
		else
		{
			if (!Hero) break;
			else
			{
				float targ_x = (float)(cur_pos.x * scale_x);
				float targ_y = (float)(cur_pos.y * scale_y);

				Hero->set_path(targ_x, targ_y);
				Hero->angle = Hero->rotate_angle(abs(Hero->center.x - targ_x), abs(Hero->center.y - targ_y));
				Hero->hero_moving = true;
				if (sound)mciSendString(L"play .\\res\\snd\\engine.wav", NULL, NULL, NULL);
			}
		}
		break;

	case WM_RBUTTONDOWN:
		if (!Hero)break;
		else
		{
			float targ_x = (float)(cur_pos.x * scale_x);
			float targ_y = (float)(cur_pos.y * scale_y);

			Hero->set_path(targ_x, targ_y);
			Hero->angle = Hero->rotate_angle(abs(Hero->center.x - targ_x), abs(Hero->center.y - targ_y));

			vHeroShots.push_back(dll::CREATURES::create(creatures::shot, Hero->center.x, Hero->center.y));
			vHeroShots.back()->angle = Hero->angle;
			vHeroShots.back()->set_path(targ_x, targ_y);
			if (sound)mciSendString(L"play .\\res\\snd\\laser.wav", NULL, NULL, NULL);
		}
		break;

	default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
	}

	return (LRESULT)(FALSE);
}

void CreateResources()
{
	int result{ 0 };
	CheckFile(Ltmp_file, &result);
	if (result == FILE_EXIST)ErrExit(eStarted);
	else
	{
		std::wofstream start{ Ltmp_file };
		start << L"Game started at: " << std::chrono::system_clock::now();
		start.close();
	}

	int winx{ GetSystemMetrics(SM_CXSCREEN) / 2 - (int)(scr_width / 2.0f) };
	int winy = 20;
	if (GetSystemMetrics(SM_CXSCREEN) < winx + (int)(scr_width) || GetSystemMetrics(SM_CYSCREEN) < winy + (int)(scr_height))
		ErrExit(eScreen);

	bIcon = (HICON)(LoadImage(NULL, L".\\res\\main.ico", IMAGE_ICON, 255, 255, LR_LOADFROMFILE));
	if (!bIcon)ErrExit(eIcon);

	bCursor = LoadCursorFromFileW(L".\\res\\main.ani");
	outCursor = LoadCursorFromFileW(L".\\res\\out.ani");

	if (!bCursor || !outCursor)ErrExit(eCursor);

	bWinClass.lpszClassName = bWinClassName;
	bWinClass.hInstance = bIns;
	bWinClass.hbrBackground = CreateSolidBrush(RGB(10, 10, 10));
	bWinClass.hIcon = bIcon;
	bWinClass.hCursor = bCursor;
	bWinClass.lpfnWndProc = &WinProc;
	bWinClass.style = CS_DROPSHADOW;

	if (!RegisterClass(&bWinClass))ErrExit(eClass);

	bHwnd = CreateWindow(bWinClassName, L"SPACE INVADERS", WS_CAPTION | WS_SYSMENU, winx, winy, (int)(scr_width),
		(int)(scr_height), NULL, NULL, bIns, NULL);
	if (!bHwnd)ErrExit(eWindow);
	else
	{
		ShowWindow(bHwnd, SW_SHOWDEFAULT);

		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &iFactory);
		if (hr != S_OK)
		{
			LogErr(L"Error creating main D2D1 Factory !");
			ErrExit(eD2D);
		}

		if (iFactory)
		{
			hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(bHwnd,
				D2D1::SizeU((UINT32)(scr_width), (UINT32)(scr_height))), &Draw);
			if (hr != S_OK)
			{
				LogErr(L"Error creating main D2D1 HwndRenderTarget !");
				ErrExit(eD2D);
			}
		}

		if (Draw)
		{
			RECT scrR{};
			D2D1_SIZE_F hwndR{ Draw->GetSize() };

			GetClientRect(bHwnd, &scrR);

			scale_x = hwndR.width / (scrR.right - scrR.left);
			scale_y = hwndR.height / (scrR.bottom - scrR.top);

			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Maroon), &statBrush);
			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGreen), &txtBrush);
			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::OrangeRed), &hgltBrush);
			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::MidnightBlue), &inactBrush);

			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::SlateGray), &backBrush);
			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Lime), &lifeBrush);

			if (hr != S_OK)
			{
				LogErr(L"Error creating main D2D1 SolidColorBrushes !");
				ErrExit(eD2D);
			}

			D2D1_GRADIENT_STOP gStops[2]{};
			ID2D1GradientStopCollection* gColl{ nullptr };

			gStops[0].position = 0;
			gStops[0].color = D2D1::ColorF(D2D1::ColorF::LightSkyBlue);
			gStops[1].position = 1.0f;
			gStops[1].color = D2D1::ColorF(D2D1::ColorF::LightSeaGreen);

			hr = Draw->CreateGradientStopCollection(gStops, 2, &gColl);
			if (hr != S_OK)
			{
				LogErr(L"Error creating D2D1 GradientStopCollection for RadialGradientBrushes !");
				ErrExit(eD2D);
			}

			if (gColl)
			{
				hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b1Rect.left +
					(b1Rect.right - b1Rect.left) / 2.0f, 25.0f), D2D1::Point2F(0, 0), (b1Rect.right - b1Rect.left) / 2.0f, 25.0f),
					gColl, &b1BckgBrush);
				hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b2Rect.left +
					(b2Rect.right - b2Rect.left) / 2.0f, 25.0f), D2D1::Point2F(0, 0), (b2Rect.right - b2Rect.left) / 2.0f, 25.0f),
					gColl, &b2BckgBrush);
				hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b3Rect.left +
					(b3Rect.right - b3Rect.left) / 2.0f, 25.0f), D2D1::Point2F(0, 0), (b3Rect.right - b3Rect.left) / 2.0f, 25.0f),
					gColl, &b3BckgBrush);
				if (hr != S_OK)
				{
					LogErr(L"Error creating D2D1 RadialGradientBrushes !");
					ErrExit(eD2D);
				}
				
				FreeMem(&gColl);
			}

			bmpAssetArmor = Load(L".\\res\\img\\assets\\armor.png", Draw);
			if (!bmpAssetArmor)
			{
				LogErr(L"Error loading bmpAssetArmor !");
				ErrExit(eD2D);
			}
			bmpAssetLife = Load(L".\\res\\img\\assets\\life.png", Draw);
			if (!bmpAssetLife)
			{
				LogErr(L"Error loading bmpAssetlife !");
				ErrExit(eD2D);
			}
			bmpAssetShot = Load(L".\\res\\img\\assets\\shot.png", Draw);
			if (!bmpAssetShot)
			{
				LogErr(L"Error loading bmpAssetShot !");
				ErrExit(eD2D);
			}

			bmpEvil1 = Load(L".\\res\\img\\evils\\evil1.png", Draw);
			if (!bmpEvil1)
			{
				LogErr(L"Error loading bmpEvil1 !");
				ErrExit(eD2D);
			}
			bmpEvil2 = Load(L".\\res\\img\\evils\\evil2.png", Draw);
			if (!bmpEvil2)
			{
				LogErr(L"Error loading bmpEvil2 !");
				ErrExit(eD2D);
			}
			bmpEvil3 = Load(L".\\res\\img\\evils\\evil3.png", Draw);
			if (!bmpEvil3)
			{
				LogErr(L"Error loading bmpEvil3 !");
				ErrExit(eD2D);
			}
			bmpEvil4 = Load(L".\\res\\img\\evils\\evil4.png", Draw);
			if (!bmpEvil4)
			{
				LogErr(L"Error loading bmpEvil4 !");
				ErrExit(eD2D);
			}

			bmpLogo = Load(L".\\res\\img\\logos\\logo.png", Draw);
			if (!bmpLogo)
			{
				LogErr(L"Error loading bmpLogo !");
				ErrExit(eD2D);
			}
			bmpLevel = Load(L".\\res\\img\\logos\\level.png", Draw);
			if (!bmpLevel)
			{
				LogErr(L"Error loading bmpLevel !");
				ErrExit(eD2D);
			}
			bmpWin = Load(L".\\res\\img\\logos\\win.png", Draw);
			if (!bmpWin)
			{
				LogErr(L"Error loading bmpWin !");
				ErrExit(eD2D);
			}
			bmpLoose = Load(L".\\res\\img\\logos\\loose.png", Draw);
			if (!bmpLoose)
			{
				LogErr(L"Error loading bmpLoose !");
				ErrExit(eD2D);
			}
			bmpWorldRecord = Load(L".\\res\\img\\logos\\Record.png", Draw);
			if (!bmpWorldRecord)
			{
				LogErr(L"Error loading bmpWorldRecord !");
				ErrExit(eD2D);
			}

			for (int i = 0; i < 48; ++i)
			{
				wchar_t name[100]{ L".\\res\\img\\field\\intro\\" };
				wchar_t add[4]{ L"\0" };
				wsprintf(add, L"%d", i);
				wcscat_s(name, add);
				wcscat_s(name, L".png");
				bmpIntro[i] = Load(name, Draw);
				if (!bmpIntro[i])
				{
					LogErr(L"Error loading bmpIntro !");
					ErrExit(eD2D);
				}
			}
			for (int i = 0; i < 49; ++i)
			{
				wchar_t name[100]{ L".\\res\\img\\field\\space\\" };
				wchar_t add[4]{ L"\0" };
				wsprintf(add, L"%d", i);
				wcscat_s(name, add);
				wcscat_s(name, L".png");
				bmpSpace[i] = Load(name, Draw);
				if (!bmpSpace[i])
				{
					LogErr(L"Error loading bmpSpace !");
					ErrExit(eD2D);
				}
			}
			for (int i = 0; i < 10; ++i)
			{
				wchar_t name[100]{ L".\\res\\img\\field\\meteor1\\" };
				wchar_t add[4]{ L"\0" };
				wsprintf(add, L"%d", i);
				wcscat_s(name, add);
				wcscat_s(name, L".png");
				bmpMeteor1[i] = Load(name, Draw);
				if (!bmpMeteor1[i])
				{
					LogErr(L"Error loading bmpMeteor1 !");
					ErrExit(eD2D);
				}
			}
			for (int i = 0; i < 5; ++i)
			{
				wchar_t name[100]{ L".\\res\\img\\field\\meteor2\\" };
				wchar_t add[4]{ L"\0" };
				wsprintf(add, L"%d", i);
				wcscat_s(name, add);
				wcscat_s(name, L".png");
				bmpMeteor2[i] = Load(name, Draw);
				if (!bmpMeteor2[i])
				{
					LogErr(L"Error loading bmpMeteor2 !");
					ErrExit(eD2D);
				}
			}
			for (int i = 0; i < 20; ++i)
			{
				wchar_t name[100]{ L".\\res\\img\\field\\meteor3\\" };
				wchar_t add[4]{ L"\0" };
				wsprintf(add, L"%d", i);
				wcscat_s(name, add);
				wcscat_s(name, L".png");
				bmpMeteor3[i] = Load(name, Draw);
				if (!bmpMeteor3[i])
				{
					LogErr(L"Error loading bmpMeteor3 !");
					ErrExit(eD2D);
				}
			}

			for (int i = 0; i < 6; ++i)
			{
				wchar_t name[100]{ L".\\res\\img\\hero\\" };
				wchar_t add[4]{ L"\0" };
				wsprintf(add, L"%d", i);
				wcscat_s(name, add);
				wcscat_s(name, L".png");
				bmpHero[i] = Load(name, Draw);
				if (!bmpHero[i])
				{
					LogErr(L"Error loading bmpHero !");
					ErrExit(eD2D);
				}
			}
			for (int i = 0; i < 12; ++i)
			{
				wchar_t name[100]{ L".\\res\\img\\shot\\" };
				wchar_t add[4]{ L"\0" };
				wsprintf(add, L"%d", i);
				wcscat_s(name, add);
				wcscat_s(name, L".png");
				bmpShot[i] = Load(name, Draw);
				if (!bmpShot[i])
				{
					LogErr(L"Error loading bmpShot !");
					ErrExit(eD2D);
				}
			}
		}

		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&iWriteFactory));
		if (hr != S_OK)
		{
			LogErr(L"Error creating main D2D1 WriteFactory !");
			ErrExit(eD2D);
		}

		if (iWriteFactory)
		{
			hr = iWriteFactory->CreateTextFormat(L"Copperplate Gothic", nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_OBLIQUE,
				DWRITE_FONT_STRETCH_NORMAL, 18.0f, L"", &nrmText);
			hr = iWriteFactory->CreateTextFormat(L"Copperplate Gothic", nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_OBLIQUE,
				DWRITE_FONT_STRETCH_NORMAL, 32.0f, L"", &midText);
			hr = iWriteFactory->CreateTextFormat(L"Copperplate Gothic", nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_OBLIQUE,
				DWRITE_FONT_STRETCH_NORMAL, 72.0f, L"", &bigText);
			if (hr != S_OK)
			{
				LogErr(L"Error creating D2D1 WriteTextFormats !");
				ErrExit(eD2D);
			}
		}
	}

	PlaySound(L".\\res\\snd\\intro.wav", NULL, SND_ASYNC);

	for (int i = 0; i < 220; ++i)
	{
		Draw->BeginDraw();
		Draw->DrawBitmap(bmpIntro[Intro.frame()], D2D1::RectF(0, 0, scr_width, scr_height));
		Draw->DrawBitmap(bmpLogo, D2D1::RectF(0, 0, scr_width, scr_height));
		Draw->EndDraw();

		if (i == 219)PlaySound(L".\\res\\snd\\boom.wav", NULL, SND_SYNC);
	}
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

	CreateResources();

	while (bMsg.message != WM_QUIT)
	{
		if ((bRet = PeekMessage(&bMsg, NULL, NULL, NULL, PM_REMOVE)) != 0)
		{
			if (bRet == -1)ErrExit(eMsg);

			TranslateMessage(&bMsg);
			DispatchMessage(&bMsg);
		}

		if (pause)
		{
			if (show_help)continue;

			if (bigText && txtBrush)
			{
				Draw->BeginDraw();
				Draw->DrawBitmap(bmpIntro[Intro.frame()], D2D1::RectF(0, 0, scr_width, scr_height));
				Draw->DrawTextW(L"ПАУЗА", 6, bigText, D2D1::RectF(scr_width / 2.0f - 100.0f, scr_height / 2.0f - 50.0f,
					scr_width, scr_height), txtBrush);
				Draw->EndDraw();
				continue;
			}
		}

	// ACTION //////////////////////////////////////////////////////////////////

		if (Hero)
		{
			if (Hero->hero_moving)Hero->move(level);
			
		}
		if (!vHeroShots.empty())
		{
			for (std::vector<dll::CREATURES*>::iterator shot = vHeroShots.begin(); shot < vHeroShots.end(); ++shot)
			{
				if (!(*shot)->shot_move(level))
				{
					(*shot)->Release();
					vHeroShots.erase(shot);
					break;
				}
			}
		}

		if (vMeteors.size() < (int)(level)+3 && RandIt(0, 700) == 66)
		{
			float start_x{ RandIt(0.0f, 900.0f) };
			float start_y{ sky - 100.0f };

			float end_x{ 0 };
			float end_y{ ground + 200.0f };

			float opposite{ 0 };
			float adjanced{ 0 };

			if (start_x < scr_width / 2.0f)end_x = scr_width / 2.0f + RandIt(50.0f, scr_width);
			else end_x = scr_width / 2.0f - RandIt(50.0f, 400.0f);

			if (RandIt(0, 2) == 1)start_y = ground + 100.0f;

			opposite = abs(start_x - end_x);
			adjanced = abs(start_y - end_y);

			vMeteors.push_back(dll::METEORS::create(static_cast<meteors>(RandIt(0, 2)), start_x, start_y, end_x, end_y,
				opposite, adjanced));
		}
		if (!vMeteors.empty())
		{
			for (std::vector<dll::METEORS*>::iterator met = vMeteors.begin(); met < vMeteors.end(); ++met)
			{
				if (!(*met)->move(level))
				{
					(*met)->Release();
					vMeteors.erase(met);
					break;
				}
			}
		}


		if (vEvils.size() < 3 + (int)(level) && RandIt(0, 300) == 66)
		{
			float sx{ scr_width - 100.0f };
			float sy{ RandIt(sky, ground - 100.0f) };

			if (RandIt(0, 2) == 2)sx = 100.0f;
	
			vEvils.push_back(dll::CREATURES::create(static_cast<creatures>(RandIt(0, 3)), sx, sy));
			if (vEvils.back()->center.x > scr_width / 2.0f)vEvils.back()->set_path(0, vEvils.back()->center.y);
			else vEvils.back()->set_path(scr_width, vEvils.back()->center.y);
			vEvils.back()->action = actions::patrol;
		}

		if (!vEvils.empty() && Hero)
		{
			dll::BAG<D2D1_POINT_2F>MeteorsBag(vMeteors.size());
			dll::BAG<D2D1_POINT_2F>AssetsBag(vAssets.size());

			if (!vMeteors.empty())
				for (int i = 0; i < vMeteors.size(); ++i)MeteorsBag.push_back(vMeteors[i]->center);
			if (!vAssets.empty())
				for (int i = 0; i < vAssets.size(); ++i)AssetsBag.push_back(D2D1::Point2F(vAssets[i].rect.left + 11.0f,
					vAssets[i].rect.top + 15.5f));

			for (std::vector<dll::CREATURES*>::iterator evil = vEvils.begin(); evil < vEvils.end(); ++evil)
			{
				if (!MeteorsBag.empty())dll::sort(MeteorsBag, (*evil)->center);
				if (!AssetsBag.empty())dll::sort(AssetsBag, (*evil)->center);

				dll::AIMove((*evil), AssetsBag, MeteorsBag, Hero->center);
			}
		}

		if (!vEvils.empty())
		{
			for (int i = 0; i < vEvils.size(); ++i)
			{
				if (vEvils[i]->action == actions::patrol || vEvils[i]->action == actions::move)vEvils[i]->move(level);
				else if (vEvils[i]->action == actions::attack)
				{
					int damage = vEvils[i]->attack();

					if (damage > 0)
					{
						vEvilShots.push_back(dll::CREATURES::create(creatures::shot, vEvils[i]->center.x, vEvils[i]->center.y));
						vEvilShots.back()->set_path(Hero->center.x, Hero->center.y);
						vEvilShots.back()->angle = vEvils[i]->angle;
					}
				}
			}
		}




	// DRAW THINGS *****************************************************************

		Draw->BeginDraw();
		
		Draw->DrawBitmap(bmpSpace[Field.frame()], Field.my_rect);

		// DRAW METEORS *********************************

		if (!vMeteors.empty())
		{
			for (std::vector<dll::METEORS*>::iterator met = vMeteors.begin(); met < vMeteors.end(); ++met)
			{
				Draw->SetTransform(D2D1::Matrix3x2F::Rotation((*met)->angle, (*met)->center));
				switch ((*met)->type)
				{
				case meteors::meteor1:
					Draw->DrawBitmap(bmpMeteor1[(*met)->get_frame()], (*met)->my_rect);
					break;

				case meteors::meteor2:
					Draw->DrawBitmap(bmpMeteor2[(*met)->get_frame()], (*met)->my_rect);
					break;

				case meteors::meteor3:
					Draw->DrawBitmap(bmpMeteor3[(*met)->get_frame()], (*met)->my_rect);
					break;
				}
				Draw->SetTransform(D2D1::Matrix3x2F::Rotation(0, (*met)->center));
			}
		}

		////////////////////////////////////////////////////////////////////////////////
		
		
		if (statBrush && inactBrush && txtBrush && hgltBrush && nrmText && b1BckgBrush && b2BckgBrush && b3BckgBrush)
		{
			Draw->FillRectangle(D2D1::RectF(0, 0, scr_width, 50.0f), statBrush);
			Draw->FillRoundedRectangle(D2D1::RoundedRect(b1Rect, 25.0f, 30.0f), b1BckgBrush);
			Draw->FillRoundedRectangle(D2D1::RoundedRect(b2Rect, 25.0f, 30.0f), b2BckgBrush);
			Draw->FillRoundedRectangle(D2D1::RoundedRect(b3Rect, 25.0f, 30.0f), b3BckgBrush);

			if (name_set)Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmText, b1TxtRect, inactBrush);
			else
			{
				if (!b1Hglt)Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmText, b1TxtRect, txtBrush);
				else Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmText, b1TxtRect, hgltBrush);
			}
			if (!b2Hglt)Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmText, b2TxtRect, txtBrush);
			else Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmText, b2TxtRect, hgltBrush);
			if (!b3Hglt)Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmText, b3TxtRect, txtBrush);
			else Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmText, b3TxtRect, hgltBrush);
		}

		

		// DRAW HERO ************************************

		if (Hero)
		{
			Draw->SetTransform(D2D1::Matrix3x2F::Rotation(Hero->angle, Hero->center));
			Draw->DrawBitmap(bmpHero[Hero->get_frame()], Hero->my_rect);
			Draw->SetTransform(D2D1::Matrix3x2F::Rotation(0, Hero->center));
		}

		if (!vHeroShots.empty())
		{
			for (std::vector<dll::CREATURES*>::iterator shot = vHeroShots.begin(); shot < vHeroShots.end(); ++shot)
			{
				Draw->SetTransform(D2D1::Matrix3x2F::Rotation((*shot)->angle, (*shot)->center));
				Draw->DrawBitmap(bmpShot[(*shot)->get_frame()], (*shot)->my_rect);
				Draw->SetTransform(D2D1::Matrix3x2F::Rotation(0, (*shot)->center));
			}
		}
		
		// DRAW EVILS **********************************

		if (!vEvils.empty())
		{
			for (std::vector<dll::CREATURES*>::iterator evil = vEvils.begin(); evil < vEvils.end(); ++evil)
			{
				Draw->SetTransform(D2D1::Matrix3x2F::Rotation((*evil)->angle, (*evil)->center));
			
				switch ((*evil)->type)
				{
				case creatures::fighter:
					Draw->DrawBitmap(bmpEvil1, (*evil)->my_rect);
					break;

				case creatures::cruiser:
					Draw->DrawBitmap(bmpEvil2, (*evil)->my_rect);
					break;

				case creatures::shuttle:
					Draw->DrawBitmap(bmpEvil3, (*evil)->my_rect);
					break;

				case creatures::ship:
					Draw->DrawBitmap(bmpEvil4, (*evil)->my_rect);
					break;
				}

				Draw->SetTransform(D2D1::Matrix3x2F::Rotation(0, (*evil)->center));
			}
		}

		if (!vEvilShots.empty())
		{
			for (std::vector<dll::CREATURES*>::iterator shot = vEvilShots.begin(); shot < vEvilShots.end(); ++shot)
			{
				Draw->SetTransform(D2D1::Matrix3x2F::Rotation((*shot)->angle, (*shot)->center));
				Draw->DrawBitmap(bmpShot[(*shot)->get_frame()], (*shot)->my_rect);
				Draw->SetTransform(D2D1::Matrix3x2F::Rotation(0, (*shot)->center));
			}
		}



		////////////////////////////////////////////////////////////////////////////////
		
		Draw->EndDraw();
	
	}

	std::remove(tmp_file);
	ReleaseResources();

    return (int) bMsg.wParam;
}