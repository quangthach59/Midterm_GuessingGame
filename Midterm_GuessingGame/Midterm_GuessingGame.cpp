#include "stdafx.h"
#include <gdiplus.h>
#pragma comment (lib, "gdiplus")
using namespace Gdiplus;
#include "Midterm_GuessingGame.h"
#include <windowsx.h>
#include <vector>
using namespace std;
#include <time.h>
//#include <shlwapi.h>

#define MAX_LOADSTRING 100
// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

//Class pokemon, chứa 2 dữ liệu là tên file ảnh và tên pokemon
class Pokemon
{
private:
	wstring filename;	//Tên file ảnh
	wstring name;	//Tên pokemon
public:
	Pokemon(wstring a, wstring b)	//Hàm dựng 1 đối tượng pokemon với a là tên file, b là tên pokemon
	{
		filename = a;
		name = b;
	}
	wstring GetFileName()	//Lấy tên ngắn gọn của file ảnh không kèm đường dẫn đầy đủ
	{
		return filename;
	}
	wstring GetPokemonName()	//Lấy tên pokemon
	{
		return name;
	}
};

#pragma region Functions
BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
void OnPaint(HWND hwnd);
void OnDestroy(HWND hwnd);
void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
//Hàm load câu hỏi mới
void LoadNewQuestion(HWND hwnd);
//Hàm load dữ liệu từ file ini
void LoadDataFromINI();
//Kiểm tra đáp án và đến câu tiếp theo
void CheckAnswerKey(HWND hwnd, int choice);
//Kết thúc game, thông báo kết quả và restart
void EndGame(HWND hwnd);
#pragma endregion Các hàm của chương trình

#pragma region Variables and constants
//Đường dẫn của thư mục
TCHAR path[MAX_PATH + 1] = L"";
//Truyền đường dẫn hiện tại vào path
DWORD len = GetCurrentDirectory(MAX_PATH, path);
//Handle sẽ khai báo thành label, hiển thị số điểm đạt được tên của pokemon cần trả lời và câu hỏi
HWND lblScore, lblPokemonName, lblQuestion;
//Vector kiểu pokemon, lưu dữ liệu đọc từ file ini khi chương trình khởi chạy
vector<Pokemon> pokemon;
//Điểm số đạt được, mặc định khi load lên điểm = 0
static int score = 0;
//Chỉ số của câu hỏi hiện tại, mặc định câu đầu tiên là câu 1
static int currentQuestion = 1;
//Giá trị cho biết dữ liệu đã load xong hay chưa
bool loaded = false;
//Khai báo tiêu đề của form
#define defaultTitle L"1612602 - Midterm Project - Guessing Game - Pokédex"
//Kích thước mặc định của mỗi hình ảnh pokemon
#define defaultImageSize 250
//Tung độ mặc định của mỗi hình ảnh pokemon
#define defaultYAxis 80
//2 hình ảnh hiển thị ảnh pokemon để người dùng chọn
Image *image[2];
//Tạo 2 khung, cũng là vị trí và kích thước của ảnh lựa chọn
Rect dest[2]{ Rect(50, defaultYAxis, defaultImageSize, defaultImageSize), Rect(375, defaultYAxis, defaultImageSize, defaultImageSize) };
//Tạo region dựa trên vị trí của 2 khung ở trên, mục đích khi load nội dung chỉ cần vô hiệu hóa 2 region đó, giảm số đối tượng phải reload
HRGN hrgn[2]{ CreateRectRgn(dest[0].GetLeft(), dest[0].GetTop(), dest[0].GetRight(), dest[0].GetBottom()),
			CreateRectRgn(dest[1].GetLeft(), dest[1].GetTop(), dest[1].GetRight(), dest[1].GetBottom()) };
//Lưu tạm index của 2 hình ảnh trong câu hiện tại (index của vector)
int loadedIMG[2];
//Lưu vùng mà chuột đã đi qua, -1 là không nằm trong ảnh, 0 là nằm trong ảnh bên trái, 1 là nằm trong ảnh bên phải
int mouseHover = -1;
//Chỉ số của đáp án, 0 là đáp án bên trái, 1 là đáp án bên phải
int key = 0;
//Kiểm tra game đã bắt đầu hay chưa, phục vụ cho mục đích kiểm tra sự kiện click
bool gameStarted = false;

#pragma endregion Các biến và hằng số

#pragma region Unusually used code
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_MIDTERMGUESSINGGAME, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MIDTERMGUESSINGGAME));
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MIDTERMGUESSINGGAME));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MIDTERMGUESSINGGAME);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	return RegisterClassExW(&wcex);
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable
	HWND hWnd = CreateWindowW(szWindowClass, L"1612602 - Midterm Project - Guessing Game - Pokédex", WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU,
		350, 150, 700, 500, nullptr, nullptr, hInstance, nullptr);
	if (!hWnd)
	{
		return FALSE;
	}
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return TRUE;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hWnd, WM_COMMAND, OnCommand);
		HANDLE_MSG(hWnd, WM_PAINT, OnPaint);
		HANDLE_MSG(hWnd, WM_DESTROY, OnDestroy);
		HANDLE_MSG(hWnd, WM_LBUTTONDOWN, OnLButtonDown);
		HANDLE_MSG(hWnd, WM_MOUSEMOVE, OnMouseMove);
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
#pragma endregion Code cho giao diện chính của form, ít dùng đến

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	//Truyền thêm thư mục resources vào path, đây sẽ là nơi lưu ảnh
	wcscat(path, L"\\resources\\");

	//Lấy font hệ thống
	LOGFONT lf;
	GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
	HFONT hFont = CreateFont(lf.lfHeight, lf.lfWidth, lf.lfEscapement, lf.lfOrientation, lf.lfWeight,
		lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet, lf.lfOutPrecision, lf.lfClipPrecision, lf.lfQuality,
		lf.lfPitchAndFamily, lf.lfFaceName);

	//Tạo font với kích thước lớn, dùng cho label hiện câu hỏi L"Which pokemon is mentioned below?" và tên pokemon
	HFONT bigFont = CreateFont(35, lf.lfWidth, lf.lfEscapement, lf.lfOrientation, lf.lfWeight,
		lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet, lf.lfOutPrecision, lf.lfClipPrecision, lf.lfQuality,
		lf.lfPitchAndFamily, lf.lfFaceName);

	//Tạo font với kích thước vừa, dùng cho label hiện điểm số và button
	HFONT medFont = CreateFont(20, lf.lfWidth, lf.lfEscapement, lf.lfOrientation, lf.lfWeight,
		lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet, lf.lfOutPrecision, lf.lfClipPrecision, lf.lfQuality,
		lf.lfPitchAndFamily, lf.lfFaceName);

	//Label hiển thị câu hỏi
	lblQuestion = CreateWindowEx(NULL, L"STATIC", L"Which pokemon is mentioned below?", WS_CHILD | WS_VISIBLE | SS_CENTER,
		45, 20, 600, 35, hwnd, NULL, lpCreateStruct->hInstance, NULL);
	SetWindowFont(lblQuestion, bigFont, TRUE);

	//Buton bắt đầu game
	HWND btnNewGame = CreateWindowEx(NULL, L"BUTTON", L"New Game", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		570, 385, 100, 40, hwnd, (HMENU)IDC_BTNNEWGAME, lpCreateStruct->hInstance, NULL);
	SetWindowFont(btnNewGame, medFont, TRUE);

	//Label hiểm thị số điểm/câu hiện tại, mặc định điểm = 0, câu hiện tại = 1
	lblScore = CreateWindowEx(NULL, L"STATIC", L"Score: 0/1", WS_CHILD | WS_VISIBLE | SS_LEFT,
		20, 400, 130, 35, hwnd, NULL, lpCreateStruct->hInstance, NULL);
	SetWindowFont(lblScore, medFont, TRUE);

	//Label hiển thị tên của pokemon cần trả lời
	lblPokemonName = CreateWindowEx(NULL, L"STATIC", L"<<Pokemon name>>", WS_CHILD | WS_VISIBLE | SS_CENTER,
		145, 380, 415, 35, hwnd, NULL, lpCreateStruct->hInstance, NULL);
	SetWindowFont(lblPokemonName, bigFont, TRUE);

	LoadDataFromINI();
	//Khi game chưa bắt đầu, tạm ẩn các label đi, chỉ hiển thị nút New Game
	ShowWindow(lblQuestion, FALSE);
	ShowWindow(lblScore, FALSE);
	ShowWindow(lblPokemonName, FALSE);
	return true;
}
void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	case IDC_BTNNEWGAME:
	{
		//Đánh dấu bắt đầu game
		gameStarted = true;
		//Reset điểm và câu hỏi hiện tại
		score = 0;
		currentQuestion = 1;
		//Load nội dung câu hỏi mới
		LoadNewQuestion(hwnd);
		//Hiển thị các label
		ShowWindow(lblQuestion, TRUE);
		ShowWindow(lblScore, TRUE);
		ShowWindow(lblPokemonName, TRUE);
		break;
	}
	case IDM_ABOUT:
		//DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
		MessageBox(hwnd, L"1612602 - Nguyễn Quang Thạch\rMidterm Project - Windows Programming CQ2016/31\rFaculty of Information Technology\rUniversity of Science - VNUHCM",
			defaultTitle, MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
		break;
	case IDM_EXIT:
		DestroyWindow(hwnd); break;
	}
}
void OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	EndPaint(hwnd, &ps);
	//Tạm dừng trong nửa giây, cho cảm giác dữ liệu có thời gian để load
	Sleep(500);
	if (true == loaded)
	{
		//MessageBox(hwnd, L"Loaded data from config.ini successfully!", defaultTitle, MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
	}
	else
	{
		MessageBox(hwnd, L"Failed to load data from config.ini.\rClose application, check data and try again!", defaultTitle, MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
		DestroyWindow(hwnd);
	}	
}
void OnDestroy(HWND hwnd)
{
	PostQuitMessage(0);
}

void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	//Sự kiện chỉ hoạt động khi game bắt đầu, tránh trường hợp chưa bắt đầu nhưng có thể click
	//Kiểm tra vị trí mà người chơi click vào có thuộc region đã khai báo hay không
	//Click vào region nào tương ứng với chọn đáp án đó, 0 là đáp án bên trái, 1 là bên phải
	if (gameStarted)
	{
		//Tối giản điều kiện so sánh vì cả 2 Rect đều có tung độ và kích thước giống nhau, chỉ khác nhau hoành độ
		if (y >= dest[0].GetTop() && y <= dest[0].GetBottom())
		{
			//Click vào region bên trái là chọn đáp án bên trái
			if (x >= dest[0].GetLeft() && x <= dest[0].GetRight())
				CheckAnswerKey(hwnd, 0);
			//Chọn đáp án bên phải
			else if (x >= dest[1].GetLeft() && x <= dest[1].GetRight())
				CheckAnswerKey(hwnd, 1);
		}
	}
}
void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	//Thỏa với điều kiện game đã bắt đầu, tránh thao tác khi chưa bắt đầu game
	if (gameStarted)
	{
		//Giá trị hiện tại của vùng tọa độ của chuột
		//-1 nếu chuột không nằm trong ảnh, 0 nếu nằm trong ảnh bên trái, 1 nếu nằm trong ảnh bên phải
		int current = -1;
		if (y >= dest[0].GetTop() && y <= dest[0].GetBottom())
		{
			//Đưa chuột vào ảnh 0
			if (x >= dest[0].GetLeft() && x <= dest[0].GetRight())
				current = 0;
			//Đưa chuột vào ảnh 1
			else if (x >= dest[1].GetLeft() && x <= dest[1].GetRight())
				current = 1;
			//Nếu không, vẫn đang ở ngoài vùng ảnh, current = -1
		}
		//Có sự khác biệt giữa vùng tọa độ hiện tại và vùng tọa độ trước đó
		if (current != mouseHover)
		{
			//Chuột đã di chuyển vào hình nào đó
			if (mouseHover == -1)
			{
				//Vô hiệu hóa hình đó
				InvalidateRgn(hwnd, hrgn[current], TRUE);
				PAINTSTRUCT ps;
				ULONG_PTR gdiplusToken;
				HDC hdc = BeginPaint(hwnd, &ps);
				GdiplusStartupInput gdiplusStartupInput;
				GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
				{
					Graphics graphics(hdc);
					//Hình được lựa chọn cần có khung viền đỏ xung quanh để người chơi phân biệt
					Pen pen(Color(255, 255, 0, 0), 6);
					//Vẽ khung đỏ
					graphics.DrawRectangle(&pen, dest[current]);
					//Lấy lại tên của ảnh trước đó đã load vào hình qua biến tempFile
					wchar_t filename[MAX_PATH + 1];
					wsprintf(filename, L"%s%s", path, pokemon[loadedIMG[current]].GetFileName().c_str());
					//Load lại hình cũ và vẽ lại
					image[current] = new Image(filename);
					graphics.DrawImage(image[current], dest[current]);
				}				
				GdiplusShutdown(gdiplusToken);
				EndPaint(hwnd, &ps);
			}
			//Chuột di chuyển từ 1 hình nào đó ra bên ngoài
			else
			{
				//Xóa hình đã ở trước đó
				InvalidateRgn(hwnd, hrgn[mouseHover], TRUE);
				PAINTSTRUCT ps;
				ULONG_PTR gdiplusToken;
				HDC hdc = BeginPaint(hwnd, &ps);
				GdiplusStartupInput gdiplusStartupInput;
				GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
				{
					Graphics graphics(hdc);
					//Chuột không còn nằm ở hình nào nên không cần dùng khung đỏ để đánh dấu
					//Lấy lại tên của ảnh trước đó đã load vào hình qua biến tempFile
					wchar_t filename[MAX_PATH + 1];
					wsprintf(filename, L"%s%s", path, pokemon[loadedIMG[mouseHover]].GetFileName().c_str());
					//Load lại hình cũ và vẽ lại
					image[mouseHover] = new Image(filename);
					graphics.DrawImage(image[mouseHover], dest[mouseHover]);
				}
				GdiplusShutdown(gdiplusToken);
				EndPaint(hwnd, &ps);
			}
			//Lưu vùng của chuột để sử dụng cho lần sau
			mouseHover = current;
		}
	}
}
void LoadNewQuestion(HWND hwnd)
{
	//Khi load lại câu hỏi mới, không còn hiệu ứng Mouse Hover trên lựa chọn nào
	mouseHover = -1;
	//Chỉ cần invalidate vùng chứa hình ảnh để giảm tình trạng reload, các vùng khác thì không cần
	InvalidateRgn(hwnd, hrgn[0], TRUE);
	InvalidateRgn(hwnd, hrgn[1], TRUE);
	PAINTSTRUCT ps;
	ULONG_PTR gdiplusToken;
	HDC hdc = BeginPaint(hwnd, &ps);
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	{
		//Cập nhật điểm/câu hiện tại
		wchar_t tmp[20];
		wsprintf(tmp, L"Score: %d/%d", score, currentQuestion);
		SetWindowText(lblScore, tmp);
		Graphics graphics(hdc);
		//Random 2 số khác nhau trong khoảng giá trị [0, <số dữ liệu pokemon>-1], giá trị random được sẽ là index để lấy dữ liệu
		srand((int)time(0));
		int i1 = rand() % (pokemon.size() - 1);
		int i2 = i1;
		while (i2 == i1)
			i2 = rand() % (pokemon.size() - 1);

		//file1 lưu địa chỉ của ảnh thứ nhất
		wchar_t file1[MAX_PATH + 1];
		wsprintf(file1, L"%s%s", path, pokemon[i1].GetFileName().c_str());
		//file2 lưu địa chỉ của ảnh thứ hai
		wchar_t file2[MAX_PATH + 1];
		wsprintf(file2, L"%s%s", path, pokemon[i2].GetFileName().c_str());

		//Lưu index của số vừa random vào biến tạm, dùng cho sự kiện Mouse Hover để load lại ảnh
		loadedIMG[0] = i1;
		loadedIMG[1] = i2;
		//Nạp dữ liệu vào từng ảnh tương ứng		
		image[0] = new Image(file1);
		image[1] = new Image(file2);
		//Vẽ ảnh vào khung hình
		graphics.DrawImage(image[0], dest[0]);
		graphics.DrawImage(image[1], dest[1]);
		//Chọn 1 trong 2 tên pokemon để hiển thị làm câu hỏi, chỉ số 0 hoặc 1 sẽ lưu vào key để đối chiếu
		key = rand() % 2;
		if (0 == key)
			SetWindowText(lblPokemonName, pokemon[i1].GetPokemonName().c_str());
		else
			SetWindowText(lblPokemonName, pokemon[i2].GetPokemonName().c_str());
		delete *image;
	}
	GdiplusShutdown(gdiplusToken);
	EndPaint(hwnd, &ps);
}

void LoadDataFromINI()
{
	//Độ dài tối đa của buffer dùng để chứa tên các section vì không biết trước có bao nhiêu section trong tệp ini
	const int max_buff = 65536;
	WCHAR buffer[65536];

	//Lưu đường dẫn của file config.ini vào configPath
	WCHAR curPath[MAX_PATH];
	WCHAR configPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, curPath);
	wsprintf(configPath, L"%s\\config.ini", curPath);

	//Lấy toàn bộ tên các section đưa vào buffer
	int result = GetPrivateProfileString(NULL, NULL, NULL, buffer, max_buff, configPath);
	//Đánh dấu load thất bại nếu không tìm thấy file
	if (0 == result)
	{
		loaded = false;
		return;
	}
	//Mỗi section nằm trong buffer ngăn cách nhau bởi kí tự '\0'
	const wchar_t* pLine = buffer;
	//Theo dõi vị trí lỗi lúc đọc
	int index = 0;
	//Lần lượt duyệt qua từng section
	while ('\0' != *pLine)
	{
		//Biến tạm thời, độ dài của buffer dùng để đọc tên file và tên pokemon
		const int tempBufferLen = 50;
		//Tạo biến tạm lưu tên của file ảnh và tên pokemon
		wchar_t fFileName[tempBufferLen], fPokemonName[tempBufferLen];
		//Tương ứng với mỗi section, đọc key file để lấy tên file ảnh, đọc key name để lấy tên pokemon, đưa vào biến tạm
		//code1, code2 để kiểm tra việc đọc thành công hay không, nếu thất bại sẽ nhận giá trị 0
		int code1 = GetPrivateProfileString(pLine, L"file", NULL, fFileName, tempBufferLen, configPath);
		int code2 = GetPrivateProfileString(pLine, L"name", NULL, fPokemonName, tempBufferLen, configPath);
		//Kiểm tra nếu code là 0 hoặc có key nhưng không có dữ liệu (chưa xét tên file hợp lệ hay không)
		if (wcslen(fFileName) == 0 || wcslen(fPokemonName) == 0 || 0 == code1 || 0 == code2)
		{
			//Đánh dấu load thất bại
			loaded = false;
			//Hiện thông báo lỗi ở đâu, và lỗi do data, không phải do chương trình
			wchar_t msg[400];
			wsprintf(msg, L"Failed to read data from section index: %d\rThis is not application's bug.\rCheck your config.ini again!", index);
			MessageBox(0, msg, defaultTitle, MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
			return;
		}
		//Tạo 1 đối tượng pkm kiểu Pokemon từ 2 tham số vừa lấy được
		Pokemon pkm(fFileName, fPokemonName);
		//Đưa đối tượng vào vector
		pokemon.push_back(pkm);
		//Đi đến section tiếp theo
		pLine += (wcslen(pLine) + 1);
		//Tăng chỉ số index
		index++;
	}
	//Đánh dấu load thành công
	loaded = true;
}

void CheckAnswerKey(HWND hwnd, int choice)
{
	//So sánh lựa chọn với đáp án, cả 2 biến có giá trị 0 hoặc 1
	//Nếu đúng thì cộng điểm
	if (choice == key)
		score++;
	//Nếu trả lời sai, hiện thông báo nhắc nhở
	else
		MessageBox(hwnd, L"Incorrect! You should have chosen the other one.", defaultTitle, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
	if (10 == currentQuestion)
	{
		EndGame(hwnd);
		return;
	}
	//Nếu chưa kết thúc game, tăng thứ tự câu hỏi và load câu hỏi tiếp theo	
	currentQuestion++;
	LoadNewQuestion(hwnd);
}

void EndGame(HWND hwnd)
{
	//Cập nhật điểm/câu hiện tại
	wchar_t tmp[20];
	wsprintf(tmp, L"Score: %d/%d", score, currentQuestion);
	SetWindowText(lblScore, tmp);
	//Hiện thông báo số điểm đạt được và yêu cầu tắt thông báo để chơi lại
	wchar_t msg[100];
	wsprintf(msg, L"Game ended. You scored %d per %d questions!\rDismiss this message to play again!", score, currentQuestion);
	MessageBox(hwnd, msg, defaultTitle, MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
	//Gọi hàm OnCommand với ID_BTNNEWGAME để click nút New Game
	OnCommand(hwnd, IDC_BTNNEWGAME, NULL, NULL);
}