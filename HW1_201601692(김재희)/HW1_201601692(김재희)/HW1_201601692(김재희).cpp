// HW1_201601692(김재희).cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "HW1_201601692(김재희).h"
#include <iostream>
#include "vector" //vector
#include "string" //string
#include "time.h" //rand
#include <functional> //greater
#include <algorithm> //sort

using namespace std;

#define MAX_LOADSTRING 100

// 학생 정보 추가는 최대로 7개까지만 허용
#define MAX_STUDENT 7

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 학생 정보 구조체
struct student {
    string name;
    int score;
};

// 학생 이름 배열
string name[10] = { "김재희", "주지훈", "손예진", "정해인", "조은정", "소지섭", "김태희", "정지훈", "한효주", "강동원" };

// 네모 상자 size
int boxWidth = 300;
int boxHeight = 70;

// 상자 좌표
int boxX = 500;
int boxY = 500;

// 사용자의 좌표
int x = 0;
int y = 0;

// 벡터
vector<student> v;

bool comp(const student& x, const student& y) { return x.score < y.score; }

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow){
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_HW1201601692, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HW1201601692));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HW1201601692));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_HW1201601692);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

// 변동된 벡터로 다시 그리는 함수
LRESULT OnPaint(HWND hWnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    HBRUSH MyBrush, OldBrush;

    for (int i = 0; i < v.size(); i++) {
        string name = v[i].name;
        int score = v[i].score;
        string scoreString = to_string((long long int)score);

        const char* n = name.c_str();
        const char* s = scoreString.c_str();

        MyBrush = CreateHatchBrush(HS_HORIZONTAL, RGB(rand() % 255, rand() % 255, rand() % 255));
        OldBrush = (HBRUSH)SelectObject(hdc, MyBrush);

        Rectangle(ps.hdc, boxX, boxY, boxX + boxWidth, boxY + boxHeight);

        TextOut(hdc, boxX + 50, boxY + 20, n, strlen(n));
        TextOut(hdc, boxX + 50, boxY + 40, s, strlen(s));

        boxY -= boxHeight;
        //다음의 박스는 y좌표 갱신하여 그림

        SelectObject(ps.hdc, OldBrush);
        DeleteObject(MyBrush);
    }
    EndPaint(hWnd, &ps);
    return 0;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    
    switch (message)
    {
    // 왼쪽 마우스가 눌렸을 때
    case WM_LBUTTONDOWN:
        // 좌표값 저장
        x = LOWORD(lParam);
        y = HIWORD(lParam);
        // 비어 있을 때
        if (v.empty()) {
            student temp;
            srand((unsigned)time(NULL));
            temp.name = name[rand() % 10];
            temp.score = rand() % 99;

            v.push_back(temp);

            boxY = 500;
            InvalidateRect(hWnd, NULL, FALSE);
        }
        else {
            // 맨 위가 눌렸을 때
            if ((boxX <= x && x <= boxX + boxWidth && boxY + boxHeight <= y && y <= boxY + boxHeight * 2) && boxY <= boxHeight * 2 * MAX_STUDENT) {
                //y좌표 갱신
                v.pop_back();
                boxY = 500;
                //박스 지우기 전 스택에서 pop
                InvalidateRect(hWnd, NULL, TRUE);
            }
            // 박스 추가
            else { 
                if (v.size() == MAX_STUDENT) { //스택검사 실시
                    ::MessageBox(0, "벡터가 가득찼습니다.", "벡터가 가득 참", MB_OK);
                    return 0;
                }
                else {
                    student temp;
                    srand((unsigned)time(NULL));
                    temp.name = name[rand() % 10];
                    temp.score = rand() % 99;

                    v.push_back(temp);
                    sort(v.begin(), v.end(), comp);

                    boxY = 500;
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            }
        }
        return 0;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            // 그리기 코드
            OnPaint(hWnd);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
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
