// HW2_201601692(김재희).cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "HW2_201601692(김재희).h"
#include <iostream>
#include "vector" //vector
#include "string" //string
#include "time.h" //rand
#include <functional> //greater
#include <algorithm> //sort

using namespace std;

#define MAX_LOADSTRING 100
#define GAP 30

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

// 벡터
vector<student> v;

int centerx, centery = 0; //전체 스크린의 중심x,y좌표 

D2D_POINT_2F lastdown; // 가장 최근에 눌려진 좌표
D2D_POINT_2F current; // 현재 마우스 좌표 

float Angle = 0; // 각도 조절 인자
float Size = 0; // 사이즈 조절 인자 
int stacksize = 0; // 현재 학생정보 데이터의 사이즈 

boolean inserttingflag = false; // 삽입이 되는 중
boolean inserttedflag = false; // 삽입 완료
boolean deletedflag = false; // 삭제가 되는 중
boolean deletingflag = false; // 삭제 완료

D2D1_RECT_F insertrectangle;

bool comp(const student& x, const student& y) { return x.score < y.score; }

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
    if (SUCCEEDED(CoInitialize(NULL)))
    {
        {
            DemoApp app;

            if (SUCCEEDED(app.Initialize(hInstance)))
            {
                app.RunMessageLoop();
            }
        }
        CoUninitialize();
    }

    return 0;
}

// DemoApp
DemoApp::DemoApp() :
    m_hwnd(NULL),
    m_pD2DFactory(NULL),
    m_pRenderTarget(NULL),
    m_pStrokeStyleDash(NULL),
    m_pGridPatternBitmapBrush(NULL),
    m_pOriginalShapeBrush(NULL),
    m_boundaryBrush(NULL),
    m_RectangleBrush(NULL),
    m_pFillBrush(NULL),
    m_pTextBrush(NULL),
    m_pDWriteFactory(NULL),
    m_pTextFormat(NULL)
{
}

DemoApp::~DemoApp()
{
    SAFE_RELEASE(m_pD2DFactory);
    SAFE_RELEASE(m_pRenderTarget);
    SAFE_RELEASE(m_pStrokeStyleDash);
    SAFE_RELEASE(m_pGridPatternBitmapBrush);
    SAFE_RELEASE(m_pOriginalShapeBrush);
    SAFE_RELEASE(m_boundaryBrush);
    SAFE_RELEASE(m_RectangleBrush);
    SAFE_RELEASE(m_pFillBrush);
    SAFE_RELEASE(m_pTextBrush);
    SAFE_RELEASE(m_pDWriteFactory);
    SAFE_RELEASE(m_pTextFormat);
}

HRESULT DemoApp::Initialize(HINSTANCE hInstance)
{
    HRESULT hr;

    hr = CreateDeviceIndependentResources();
    if (SUCCEEDED(hr))
    {
        WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = DemoApp::WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(LONG_PTR);
        wcex.hInstance = hInstance;
        wcex.hbrBackground = NULL;
        wcex.lpszMenuName = NULL;
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.lpszClassName = L"HW2_201601692(김재희)";
        RegisterClassEx(&wcex);

        m_hwnd = CreateWindow(
            L"HW2_201601692(김재희)", L"HW2_201601692(김재희)",
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
            800, 900, NULL, NULL, hInstance, this
        );
        hr = m_hwnd ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            ShowWindow(m_hwnd, SW_SHOWNORMAL);
            UpdateWindow(m_hwnd);
        }
    }

    return hr;
}

HRESULT DemoApp::CreateDeviceIndependentResources()
{
    HRESULT hr = S_OK;

    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

    if (SUCCEEDED(hr))
    {
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
        );
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pDWriteFactory->CreateTextFormat(
            L"Verdana",
            NULL,
            DWRITE_FONT_WEIGHT_REGULAR,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            13.0f,
            L"en-us",
            &m_pTextFormat
        );
    }

    if (SUCCEEDED(hr))
    {
        // 점선 획 스타일 객체를 생성함.
        float dashes[] = { 10.0f, 2.0f };
        hr = m_pD2DFactory->CreateStrokeStyle(
            D2D1::StrokeStyleProperties(
                D2D1_CAP_STYLE_FLAT,
                D2D1_CAP_STYLE_FLAT,
                D2D1_CAP_STYLE_FLAT,
                D2D1_LINE_JOIN_MITER,
                10.0f,
                D2D1_DASH_STYLE_CUSTOM,
                0.0f),
            dashes,
            ARRAYSIZE(dashes),
            &m_pStrokeStyleDash
        );
    }

    return hr;
}

HRESULT DemoApp::CreateDeviceResources()
{
    HRESULT hr = S_OK;

    if (!m_pRenderTarget)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

        hr = m_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_pRenderTarget
        );

        if (SUCCEEDED(hr))
        {
            // 변환 전 모양의 외곽선을 위한 붓을 생성.
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray), &m_pOriginalShapeBrush);
        }

        if (SUCCEEDED(hr))
        {
            // 변환 후 영역을 채우는 용도의 붓을 생성.
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pFillBrush);
        }

        if (SUCCEEDED(hr))
        {
            // 텍스트를 쓰기 위한 붓을 생성.
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pTextBrush);
        }

        if (SUCCEEDED(hr))
        {
            // 변환 후 모양의 외곽선을 위한 붓을 생성.
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Navy), &m_boundaryBrush);
        }
        if (SUCCEEDED(hr))
        {
            // 변환 후 모양의 외곽선을 위한 붓을 생성.
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightSkyBlue), &m_RectangleBrush);
        }
        if (SUCCEEDED(hr))
        {
            hr = CreateGridPatternBrush(m_pRenderTarget, &m_pGridPatternBitmapBrush);
        }
    }

    return hr;
}

HRESULT DemoApp::CreateGridPatternBrush(ID2D1RenderTarget* pRenderTarget, ID2D1BitmapBrush** ppBitmapBrush)
{
    HRESULT hr = S_OK;

    // 호환 렌더타겟을 생성.
    ID2D1BitmapRenderTarget* pCompatibleRenderTarget = NULL;
    hr = pRenderTarget->CreateCompatibleRenderTarget(D2D1::SizeF(10.0f, 10.0f), &pCompatibleRenderTarget);

    if (SUCCEEDED(hr))
    {
        // 패턴을 그림.
        ID2D1SolidColorBrush* pGridBrush = NULL;
        hr = pCompatibleRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.93f, 0.94f, 0.96f, 1.0f), &pGridBrush);

        if (SUCCEEDED(hr))
        {
            pCompatibleRenderTarget->BeginDraw();
            pCompatibleRenderTarget->FillRectangle(D2D1::RectF(0.0f, 0.0f, 10.0f, 1.0f), pGridBrush);
            pCompatibleRenderTarget->FillRectangle(D2D1::RectF(0.0f, 0.0f, 1.0f, 10.0f), pGridBrush);
            hr = pCompatibleRenderTarget->EndDraw();

            if (hr == D2DERR_RECREATE_TARGET)
            {
                // 모든 장치 의존적 자원들을 반납.
                DiscardDeviceResources();
            }
            if (SUCCEEDED(hr))
            {
                // 렌더타겟으로부터 비트맵을 얻음.
                ID2D1Bitmap* pGridBitmap = NULL;
                hr = pCompatibleRenderTarget->GetBitmap(&pGridBitmap);

                if (SUCCEEDED(hr))
                {
                    // 비트맵 붓을 생성함.
                    hr = m_pRenderTarget->CreateBitmapBrush(
                        pGridBitmap,
                        D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP),
                        ppBitmapBrush
                    );

                    pGridBitmap->Release();
                }
            }

            pGridBrush->Release();
        }

        pCompatibleRenderTarget->Release();
    }

    return hr;
}

void DemoApp::DiscardDeviceResources()
{
    SAFE_RELEASE(m_pRenderTarget);
    SAFE_RELEASE(m_boundaryBrush);
    SAFE_RELEASE(m_pFillBrush);
    SAFE_RELEASE(m_pOriginalShapeBrush);
    SAFE_RELEASE(m_pTextBrush);
    SAFE_RELEASE(m_pGridPatternBitmapBrush);
}

void DemoApp::RunMessageLoop()
{
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void dataPush() { // data를 vector v에 push해줌 
    student temp;
    srand((unsigned)time(NULL));
    temp.name = name[rand() % 10];
    temp.score = rand() % 99;

    v.push_back(temp);
}
D2D1_RECT_F getNextRectangle(int stackSize) { // stacksize를 가지고 다음에 그려질 사각형 좌표를 얻어냄 
    float top = 870 - 30 * (stackSize + 2);
    return D2D1::RectF(centerx - 60, top, centerx + 60, top + 30);
}
void string2wstring(wstring& dest, const string& src)
{
    dest.resize(src.size());
    for (unsigned int i = 0; i < src.size(); i++)
        dest[i] = static_cast<unsigned char>(src[i]);
}

HRESULT DemoApp::OnRender()
{
    HRESULT hr = CreateDeviceResources();

    if (SUCCEEDED(hr))
    {
        // 그리기를 시작함.
        m_pRenderTarget->BeginDraw();

        // 렌더타겟 변환을 항등 변환으로 리셋함.
        m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
        // 렌더타겟 내용을 클리어함.
        m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

        D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

        m_pRenderTarget->FillRectangle(
            D2D1::RectF(0.0f, 0.0f, rtSize.width, rtSize.height),
            m_pGridPatternBitmapBrush);

        if (deletingflag == true || deletedflag == true) { //deletingflag나 deletedflag가 true이면 잠시 stacksize를 줄여준다. 
            stacksize--;
        }
        for (int i = 0; i < stacksize; i++) { // v에 저장된 스택을 그려준다. 
            m_pRenderTarget->FillRectangle(getNextRectangle(i), m_RectangleBrush); // 그다음 그려질 스택 위치를 얻어내서 사각형을 그려준다. 
            const char* multiByte = v[i].name.c_str(); // v[i].name을 const char*형으로 바꾼다. 
            TCHAR temp[15]; // const char*을 TCHAR형으로 바꿔야 하기 때문에 temp변수를 선언한다. 
            memset(temp, 0, sizeof(temp));
            //memset(str, 0, sizeof(str));
            MultiByteToWideChar(CP_ACP, MB_COMPOSITE, multiByte, -1, temp, 15);
            static WCHAR szText[100]; // name과 점수를 한번에 넣어아 햐므로 WCHAR 배열형 변수를 선언한다. 
            swprintf_s(szText, L"%s %d\n", temp, v[i].score);
            m_pRenderTarget->DrawText(szText, wcslen(szText), m_pTextFormat, getNextRectangle(i), m_pTextBrush); // Text를 그려준다.
            m_pRenderTarget->DrawRectangle(getNextRectangle(i), m_boundaryBrush); // 상자 테두리를 그려준다. 
        }
        //m_pFillBrush->SetOpacity(0.5f);
        if (deletingflag == true || deletedflag == true) { // 줄여놨던 stacksize를 다시 늘려준다.
            stacksize++;
        }
        drawInit();
        if (inserttingflag == true) { // 만약 삽입하고 있는과정이라면
            drawTempInsertingRectangle(); // 드래그하는 동안의 상자를 그려준다. 
            m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
            m_pRenderTarget->DrawRectangle(getNextRectangle(stacksize), m_pOriginalShapeBrush); // 상자 위치를 표시해준다. 
        }
        m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        if (deletingflag == true) {
            drawTempdeletingRectangle();

        }
        //DrawAndTranslateARectangle();
        if (inserttedflag == true) { // 삽입 완료됬으면 
    /*		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
            m_pRenderTarget->FillRectangle(getNextRectangle(stacksize), m_pOriginalShapeBrush);*/

            stacksize++; // 스택사이즈를 늘려준다. 
            dataPush(); // 벡터에 data를 넣어준다. 
            inserttedflag = false;
        }
        if (deletedflag == true) { // 삭제되었으면
            stacksize--; //stacksize를 줄여준다.
            v.pop_back(); // 벡터에서 값을 빼온다. 
            deletedflag = false;
        }
        // 그리기 연산들을 제출함.
        m_pRenderTarget->EndDraw();
    }

    if (hr == D2DERR_RECREATE_TARGET)
    {
        hr = S_OK;
        DiscardDeviceResources();
    }

    return hr;
}



void DemoApp::drawTempInsertingRectangle() {
    D2D1_MATRIX_3X2_F translation = D2D1::Matrix3x2F::Translation(current.x - lastdown.x, current.y - lastdown.y); // 최근 눌려진 좌표에서 현재 좌표까지 거리만큼 이동시켜준다. 
    Size = ((current.y - lastdown.y) / (getNextRectangle(stacksize).top - 30)) * 3 + 1; // 목표 지점까지 얼마만큼 왔는지 비율을 구해서 Size를 구해줍니다. 
    D2D1_MATRIX_3X2_F scale = D2D1::Matrix3x2F::Scale(D2D1::Size(Size, 1.0f), D2D1::Point2F(insertrectangle.left, insertrectangle.top));
    Angle = (current.y - lastdown.y) / (getNextRectangle(stacksize).top - 30) * 360; // Size와 비슷하게 목표 지점까지 얼마만큼 왔는지 비율을 구해서 Angle을 구해줍니다. 
    D2D1_MATRIX_3X2_F rotation = D2D1::Matrix3x2F::Rotation(Angle, D2D1::Point2F((insertrectangle.right + insertrectangle.left) / 2, (insertrectangle.top + insertrectangle.bottom) / 2));
    m_pRenderTarget->SetTransform(scale * rotation * translation); // scale->rotation->translation 순

    m_pRenderTarget->FillRectangle(insertrectangle, m_pOriginalShapeBrush); //insertrectangled을 변환하여 그려줍니다.

}

void DemoApp::drawTempdeletingRectangle() {
    stacksize--;
    D2D1_MATRIX_3X2_F translation = D2D1::Matrix3x2F::Translation(current.x - lastdown.x, current.y - lastdown.y); // 최근 눌려진 좌표에서 현재 좌표까지 거리만큼 이동시켜준다.
    if (current.y <= lastdown.y) // 만약 위로 드래깅 한다면
        Size = (getNextRectangle(stacksize).top - 30) / ((lastdown.y - current.y) * 3 + (getNextRectangle(stacksize).top - 30)); // 목표 지점까지 얼마만큼 왔는지 비율을 구해서 Size를 구해줍니다. 
    else //만약 아래로 드래깅 한다면 
        Size = ((current.y - lastdown.y) / (getNextRectangle(stacksize).top - 30)) * 3 + 1; // 삽입 할때의 공식을 적용 
    D2D1_MATRIX_3X2_F scale = D2D1::Matrix3x2F::Scale(D2D1::Size(Size, 1.0f), D2D1::Point2F(lastdown.x, lastdown.y));
    Angle = (lastdown.y - current.y) / (getNextRectangle(stacksize).top - 30) * 360 * -1; //Size와 비슷하게 목표 지점까지 얼마만큼 왔는지 비율을 구해줌 
    //D2D1_MATRIX_3X2_F rotation = D2D1::Matrix3x2F::Rotation(Angle, D2D1::Point2F((getNextRectangle(stacksize).right + getNextRectangle(stacksize).left) / 2, (getNextRectangle(stacksize - 1).top + getNextRectangle(stacksize - 1).bottom) / 2));
    D2D1_MATRIX_3X2_F rotation = D2D1::Matrix3x2F::Rotation(Angle, D2D1::Point2F(lastdown.x, lastdown.y));
    m_pRenderTarget->SetTransform(scale * rotation * translation);
    m_pRenderTarget->FillRectangle(getNextRectangle(stacksize), m_pOriginalShapeBrush); // 최근 입력된 스택을 기준으로 변환하여 그려줍니다.
    stacksize++;

}

// 이동 변환을 렌더타겟에 지정하고 그림.
void DemoApp::drawInit()
{
    m_pOriginalShapeBrush->SetOpacity(0.3); // brsuh에 투명도를 조정해준다.
    D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize(); // rendertarget의 size를 이용하여 
    centerx = rtSize.width / 2; //전체 스크린의 중앙 점을 구해낸다.
    centery = rtSize.height / 2;

    // 렌더타겟 변환을 항등 변환으로 리셋함. 사각형을 그림.
    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    insertrectangle = D2D1::Rect(centerx - 15, GAP, centerx + 15, GAP + 30); // 가운데 상단에
    m_pRenderTarget->FillRectangle(insertrectangle, m_pOriginalShapeBrush); // 삽입을 위한 조그만한 사각형을 그려줌 

    // 캡션 텍스트를 표시함.
    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    WCHAR szText[100];
    _swprintf(szText, L"마우스 X:%.2f\n마우스 Y:%.2f\n회전각도:%.2f\n크기조정 인자 :%.2f\n박스개수 : %d\n",
        current.x, current.y, Angle, Size, v.size());
    m_pRenderTarget->DrawText(szText, wcslen(szText), m_pTextFormat,
        D2D1::RectF(10.0f, 10.5f, 236.0f, 190.5f), m_pTextBrush);
}

void DemoApp::OnResize(UINT width, UINT height)
{
    if (m_pRenderTarget)
    {
        m_pRenderTarget->Resize(D2D1::SizeU(width, height));
    }
}

/*
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HW2201601692));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_HW2201601692);
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
*/
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
LRESULT CALLBACK DemoApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        DemoApp* pDemoApp = (DemoApp*)pcs->lpCreateParams;

        SetWindowLongPtrW(hwnd, GWLP_USERDATA, PtrToUlong(pDemoApp));

        return 1;
    }

    DemoApp* pDemoApp = reinterpret_cast<DemoApp*>(static_cast<LONG_PTR>(GetWindowLongPtrW(hwnd, GWLP_USERDATA)));

    if (pDemoApp)
    {
        switch (message)
        {
        case WM_SIZE:
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            pDemoApp->OnResize(width, height);
        }
        return 0;

        case WM_DISPLAYCHANGE:
        {
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;
        case WM_LBUTTONUP: // 만약 드래그가 끊겼으면 
        {
            inserttingflag = false; // inserttingflag와 deletingflag를 모두 false로 해준다. 
            deletingflag = false;
            Angle = 0; // Angle,Size값 초기화
            Size = 0;
            break;

        }
        case WM_LBUTTONDOWN:
        {

            if (LOWORD(lParam) >= insertrectangle.left && LOWORD(lParam) <= insertrectangle.right
                && HIWORD(lParam) >= insertrectangle.top && HIWORD(lParam) <= insertrectangle.bottom) { // 만약 상단의 가운데 사각형이 눌려졌으면 
                inserttingflag = true; // insertting모드를 켜준다.
                lastdown.x = LOWORD(lParam);
                lastdown.y = HIWORD(lParam);
            }
            else if (v.size() == 0)
                break;
            else {
                D2D1_RECT_F lastRectangle = getNextRectangle(stacksize - 1);
                if (LOWORD(lParam) >= lastRectangle.left && LOWORD(lParam) <= lastRectangle.right
                    && HIWORD(lParam) >= lastRectangle.top && HIWORD(lParam) <= lastRectangle.bottom) { // 만약 최근 삽입된 상자가 눌려졌으면
                    deletingflag = true; // deleting모드를 켜준다. 
                    lastdown.x = LOWORD(lParam);
                    lastdown.y = HIWORD(lParam);
                }

            }
            break;
        }
        case WM_PAINT:
        {
            pDemoApp->OnRender();
            ValidateRect(hwnd, NULL);
            return 0;
        }

        case WM_MOUSEMOVE:
        {
            current.x = LOWORD(lParam);
            current.y = HIWORD(lParam);
            if (inserttingflag == true) {
                if (stacksize >= MAX_STUDENT) { // 만약 maxsize를 초과한다면
                    inserttingflag = false; // inserttingflag를 false로 처리해준다. 
                    ::MessageBox(0, L"the number of box is 20 , it's limit", L"Fatal Error", MB_OK);//메세지를 띄워준다.
                    ::MessageBeep(MB_OK);
                    break;
                }
                D2D1_RECT_F currentstack = getNextRectangle(stacksize);
                if (LOWORD(lParam) >= currentstack.left && LOWORD(lParam) <= currentstack.right
                    && HIWORD(lParam) >= currentstack.top && HIWORD(lParam) <= currentstack.bottom) { // inserttingflag가 true이고 들어와야할 스택에 마우스 커서가 들어왔으면
                    inserttedflag = true; // inserttedflag를 true로 맞춰준다. 
                    Angle = 0; //Angle ,Size 초기화 
                    Size = 0;
                    inserttingflag = false;
                    //InvalidateRect(hwnd, NULL, false);
                }
                InvalidateRect(hwnd, NULL, false);
            }
            else if (deletingflag == true) {
                if (LOWORD(lParam) >= insertrectangle.left && LOWORD(lParam) <= insertrectangle.right
                    && HIWORD(lParam) >= insertrectangle.top && HIWORD(lParam) <= insertrectangle.bottom) {  // deletingflag가 true이고 insertingrectangle에 마우스 커서가 들어왔으면
                    deletedflag = true; // deletedflag를 true로 맞춰준다.
                    Angle = 0; //Angle,Size 초기화 
                    Size = 0;
                    deletingflag = false;
                }
                InvalidateRect(hwnd, NULL, false);
            }
            else {
                RECT rect;
                rect.left = 10; rect.top = 10.5; rect.right = 236; rect.bottom = 190.5;
                InvalidateRect(hwnd, &rect, true);
            }

            return 0;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
        }
        return 1;
        }
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

