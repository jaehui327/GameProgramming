// HW2_201601692(김재희).cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "HW2.h"
#include <iostream>
#include "vector" //vector
#include "string" //string
#include "time.h" //rand
#include <functional> //greater
#include <algorithm> //sort

#pragma comment(lib, "D2D1")
#pragma comment(lib, "Dwrite")

using namespace std;

// 사이즈
int width = 50;
int guide = 600;

// 학생 정보 구조체
struct student {
    string name;
    int score;
};

// 학생 이름 배열
string name[10] = { "김재희", "주지훈", "손예진", "정해인", "조은정", "소지섭", "김태희", "정지훈", "한효주", "강동원" };

// 벡터
vector<student> v;

// 전체 스크린중심 좌표 
int centerX, centerY = 0; 

// 최근 좌표
D2D_POINT_2F last;
// 현재 좌표
D2D_POINT_2F current; 

float angle = 0;
float boxSize = 0; 
int vectorSize = 0; 

boolean insertingFlag = false; 
boolean insertedFlag = false; 
boolean deletedFlag = false; 
boolean deletingFlag = false; 

D2D1_RECT_F ractangle;

bool comp(const student& x, const student& y) { return x.score < y.score; }

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
    m_pOriginBorderBrush(NULL),
    m_pRectangleBorderBrush(NULL),
    m_pRectangleBrush{NULL, },
    m_pOrginFillBrush(NULL),
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
    SAFE_RELEASE(m_pOriginBorderBrush);
    SAFE_RELEASE(m_pRectangleBorderBrush);
    for(int i = 0; i < MAX_STUDENT; i ++) 
        SAFE_RELEASE(m_pRectangleBrush[i]);
    SAFE_RELEASE(m_pOrginFillBrush);
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
        wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
        wcex.lpszClassName = L"HW2_201601692(김재희)";
        RegisterClassEx(&wcex);

        m_hwnd = CreateWindow(
            L"HW2_201601692(김재희)",
            L"HW2_201601692(김재희)",
            WS_OVERLAPPEDWINDOW, 
            CW_USEDEFAULT, 
            CW_USEDEFAULT,
            guide + 250,
            guide, 
            NULL, 
            NULL, 
            hInstance,
            this
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

/******************************************************************
*                                                                 *
*  DemoApp::CreateDeviceIndependentResources                      *
*                                                                 *
*  This method is used to create resources which are not bound    *
*  to any device. Their lifetime effectively extends for the      *
*  duration of the app.                                           *
*                                                                 *
******************************************************************/

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
    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::CreateDeviceResources                                 *
*                                                                 *
*  This method creates resources which are bound to a particular  *
*  D3D device. It's all centralized here, in case the resources   *
*  need to be recreated in case of D3D device loss (eg. display   *
*  change, remoting, removal of video card, etc).                 *
*                                                                 *
******************************************************************/

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
            // 변환 전 외곽선 BORDER
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Gray), 
                &m_pOriginBorderBrush);
        }

        if (SUCCEEDED(hr))
        {
            // 변환 후 외곽선 영역
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::White), 
                &m_pOrginFillBrush);
        }

        if (SUCCEEDED(hr))
        {
            // 텍스트
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Black), 
                &m_pTextBrush);
        }

        if (SUCCEEDED(hr))
        {
            // 상자 외곽선
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Black), 
                &m_pRectangleBorderBrush);
        }
        float color[3];
        for (int i = 0; i < MAX_STUDENT; i++) {
            color[0] = rand() % 255 / 100.0;
            color[1] = rand() % 255 / 100.0;
            color[2] = rand() % 255 / 100.0;
            if (SUCCEEDED(hr))
            {
                // 상자 영역
                hr = m_pRenderTarget->CreateSolidColorBrush(
                    D2D1::ColorF(
                        color[0], color[1], color[2], 0.5),
                    &m_pRectangleBrush[i]);
            }
        }
        if (SUCCEEDED(hr))
        {
            hr = CreateGridPatternBrush(m_pRenderTarget, &m_pGridPatternBitmapBrush);
        }
    }

    return hr;
}

// 격자 모양 만들기
HRESULT DemoApp::CreateGridPatternBrush(ID2D1RenderTarget* pRenderTarget, ID2D1BitmapBrush** ppBitmapBrush)
{
    HRESULT hr = S_OK;

    ID2D1BitmapRenderTarget* pCompatibleRenderTarget = NULL;
    hr = pRenderTarget->CreateCompatibleRenderTarget(
        D2D1::SizeF(10.0f, 10.0f), 
        &pCompatibleRenderTarget);

    if (SUCCEEDED(hr))
    {
        ID2D1SolidColorBrush* pGridBrush = NULL;
        hr = pCompatibleRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(0.93f, 0.94f, 0.96f, 1.0f),
            &pGridBrush);

        if (SUCCEEDED(hr))
        {
            pCompatibleRenderTarget->BeginDraw();
            pCompatibleRenderTarget->FillRectangle(D2D1::RectF(0.0f, 0.0f, 10.0f, 1.0f), pGridBrush);
            pCompatibleRenderTarget->FillRectangle(D2D1::RectF(0.0f, 0.0f, 1.0f, 10.0f), pGridBrush);
            hr = pCompatibleRenderTarget->EndDraw();

            if (hr == D2DERR_RECREATE_TARGET)
            {
                DiscardDeviceResources();
            }
            if (SUCCEEDED(hr))
            {
                ID2D1Bitmap* pGridBitmap = NULL;
                hr = pCompatibleRenderTarget->GetBitmap(&pGridBitmap);

                if (SUCCEEDED(hr))
                {
                    hr = m_pRenderTarget->CreateBitmapBrush(
                        pGridBitmap,
                        D2D1::BitmapBrushProperties(
                            D2D1_EXTEND_MODE_WRAP, 
                            D2D1_EXTEND_MODE_WRAP),
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

/******************************************************************
*                                                                 *
*  DemoApp::DiscardDeviceResources                                *
*                                                                 *
*  Discard device-specific resources which need to be recreated   *
*  when a Direct3D device is lost.                                *
*                                                                 *
******************************************************************/

void DemoApp::DiscardDeviceResources()
{
    SAFE_RELEASE(m_pRenderTarget);
    SAFE_RELEASE(m_pRectangleBorderBrush);
    SAFE_RELEASE(m_pOrginFillBrush);
    SAFE_RELEASE(m_pOriginBorderBrush);
    SAFE_RELEASE(m_pTextBrush);
    SAFE_RELEASE(m_pGridPatternBitmapBrush);
    for (int i = 0; i < MAX_STUDENT; i++)
        SAFE_RELEASE(m_pRectangleBrush[i]);
}

/******************************************************************
*                                                                 *
*  DemoApp::RunMessageLoop                                        *
*                                                                 *
*  Main window message loop                                       *
*                                                                 *
******************************************************************/

void DemoApp::RunMessageLoop()
{
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// 다음에 그려질 사각형의 크기
D2D1_RECT_F getNextRect(int vectorSize) { 
    float top = (guide - GAP) - width * (vectorSize + 2);
    return D2D1::RectF(centerX - width * 2, top, centerX + width * 2, top + width);
}

/******************************************************************
*                                                                 *
*  DemoApp::OnRender                                              *
*                                                                 *
*  Called whenever the application needs to display the client    *
*  window.                                                        *
*                                                                 *
*  Note that this function will automatically discard             *
*  device-specific resources if the Direct3D device disappears    *
*  during execution, and will recreate the resources the          *
*  next time it's invoked.                                        *
*                                                                 *
******************************************************************/

HRESULT DemoApp::OnRender()
{
    HRESULT hr = CreateDeviceResources();

    if (SUCCEEDED(hr))
    {
        m_pRenderTarget->BeginDraw();

        m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

        D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

        m_pRenderTarget->FillRectangle(
            D2D1::RectF(0.0f, 0.0f, rtSize.width, rtSize.height),
            m_pGridPatternBitmapBrush);

        if (deletingFlag == true || deletedFlag == true) { 
            vectorSize--;
        }
        for (int i = 0; i < vectorSize; i++) { 
            m_pRenderTarget->FillRectangle(getNextRect(i), m_pRectangleBrush[i]); 
            // 형 변환
            const char* multiByte = v[i].name.c_str(); 
            TCHAR temp[15];
            memset(temp, 0, sizeof(temp));
            MultiByteToWideChar(CP_ACP, MB_COMPOSITE, multiByte, -1, temp, 15);

            // 이름 + 점수
            static WCHAR nameAndScore[100]; 
            swprintf_s(nameAndScore, L"%s %d\n", temp, v[i].score);
            // DrawText
            m_pRenderTarget->DrawText(nameAndScore, wcslen(nameAndScore), m_pTextFormat, getNextRect(i), m_pTextBrush); 
            // DrawRectangle
            m_pRenderTarget->DrawRectangle(getNextRect(i), m_pRectangleBorderBrush);
        }

        if (deletingFlag == true || deletedFlag == true) {
            vectorSize++;
        }
        drawInit();
        // 삽입 중
        if (insertingFlag == true) {
            // 드래그 중
            drawTempInsertingRectangle();
            m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
            m_pRenderTarget->DrawRectangle(getNextRect(vectorSize), m_pOriginBorderBrush); 
        }
        m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        if (deletingFlag == true) {
            drawTempdeletingRectangle();

        }
        // 삽입 완료
        if (insertedFlag == true) { 
            vectorSize++; 
            
            student temp;
            srand((unsigned)time(NULL));
            temp.name = name[rand() % 10];
            temp.score = rand() % 99;

            v.push_back(temp);
            sort(v.begin(), v.end(), comp);
            insertedFlag = false;
        }
        // 삭제 완료
        if (deletedFlag == true) { 
            vectorSize--; 
            v.pop_back();
            deletedFlag = false;
        }
        // 그리기
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
    D2D1_MATRIX_3X2_F translation = D2D1::Matrix3x2F::Translation(current.x - last.x, current.y - last.y); 

    boxSize = ((current.y - last.y) / (getNextRect(vectorSize).top - width)) * 3 + 1;
    D2D1_MATRIX_3X2_F scale = D2D1::Matrix3x2F::Scale(D2D1::Size(boxSize, 1.0f), D2D1::Point2F(ractangle.left, ractangle.top));

    angle = (current.y - last.y) / (getNextRect(vectorSize).top - width) * 360;
    D2D1_MATRIX_3X2_F rotation = D2D1::Matrix3x2F::Rotation(angle, D2D1::Point2F((ractangle.right + ractangle.left) / 2, (ractangle.top + ractangle.bottom) / 2));

    m_pRenderTarget->SetTransform(scale * rotation * translation);

    m_pRenderTarget->FillRectangle(ractangle, m_pOriginBorderBrush); //insertrectangled을 변환하여 그려줍니다.

}

void DemoApp::drawTempdeletingRectangle() {
    vectorSize--;
    D2D1_MATRIX_3X2_F translation = D2D1::Matrix3x2F::Translation(current.x - last.x, current.y - last.y); 
    // 삭제
    if (current.y <= last.y) 
        boxSize = (getNextRect(vectorSize).top - width) / ((last.y - current.y) * 3 + (getNextRect(vectorSize).top - width));
    // 삽입
    else 
        boxSize = ((current.y - last.y) / (getNextRect(vectorSize).top - width)) * 3 + 1;
    D2D1_MATRIX_3X2_F scale = D2D1::Matrix3x2F::Scale(D2D1::Size(boxSize, 1.0f), D2D1::Point2F(last.x, last.y));

    angle = (last.y - current.y) / (getNextRect(vectorSize).top - width) * 360 * -1;
   
    D2D1_MATRIX_3X2_F rotation = D2D1::Matrix3x2F::Rotation(angle, D2D1::Point2F(last.x, last.y));

    m_pRenderTarget->SetTransform(scale * rotation * translation);
    m_pRenderTarget->FillRectangle(getNextRect(vectorSize), m_pOriginBorderBrush); 

    vectorSize++;

}

void DemoApp::drawInit()
{
    m_pOriginBorderBrush->SetOpacity(0.3);
    D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize(); 
    centerX = rtSize.width / 2; 
    centerY = rtSize.height / 2;

    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    ractangle = D2D1::Rect(centerX - width / 2, GAP, centerX + width / 2, GAP + width);
    m_pRenderTarget->FillRectangle(ractangle, m_pOriginBorderBrush);

    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    WCHAR szText[100];

    swprintf_s(szText, L"마우스 X 좌표: %.2f\n마우스 Y 좌표: %.2f\n회전 인자: %.2f\n크기조정 인자: %.2f\n박스 개수 : %d\n",
        current.x, current.y, angle, boxSize, v.size());

    m_pRenderTarget->DrawText(szText, wcslen(szText), m_pTextFormat,
        D2D1::RectF(
            10.0f, 
            10.5f, 
            236.0f, 
            190.5f), 
        m_pTextBrush);
}

void DemoApp::OnResize(UINT width, UINT height)
{
    if (m_pRenderTarget)
    {
        m_pRenderTarget->Resize(D2D1::SizeU(width, height));
    }
}

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
        // 왼쪽 눌렸을 때
        case WM_LBUTTONDOWN:
        {
            // 상자 삽입 모드
            if (LOWORD(lParam) >= ractangle.left && LOWORD(lParam) <= ractangle.right
                && HIWORD(lParam) >= ractangle.top && HIWORD(lParam) <= ractangle.bottom) {
                insertingFlag = true; // insertting 모드
                last.x = LOWORD(lParam);
                last.y = HIWORD(lParam);
            }
            else if (v.size() == 0)
                break;
            else {
                D2D1_RECT_F lastRectangle = getNextRect(vectorSize - 1);
                // 가장 위에 있는 상자
                if (LOWORD(lParam) >= lastRectangle.left && LOWORD(lParam) <= lastRectangle.right
                    && HIWORD(lParam) >= lastRectangle.top && HIWORD(lParam) <= lastRectangle.bottom) { 
                    deletingFlag = true; // deleting 모드
                    last.x = LOWORD(lParam);
                    last.y = HIWORD(lParam);
                }

            }
            break;
        }
        // 드래그가 끊길 때
        case WM_LBUTTONUP:
        {
            current.x = LOWORD(lParam);
            current.y = HIWORD(lParam);
            if (insertingFlag == true) {
                // 최대 개수
                if (vectorSize >= MAX_STUDENT) {
                    insertingFlag = false;
                    ::MessageBox(0, L"최대 7개까지만 가능합니다.", L"Error", MB_OK);
                    ::MessageBeep(MB_OK);
                    break;
                }
                D2D1_RECT_F currentstack = getNextRect(vectorSize);
                if (LOWORD(lParam) >= currentstack.left && LOWORD(lParam) <= currentstack.right
                    && HIWORD(lParam) >= currentstack.top && HIWORD(lParam) <= currentstack.bottom) {
                    insertedFlag = true;
                    insertingFlag = false;
                }
                InvalidateRect(hwnd, NULL, false);
            }
            else if (deletingFlag == true) {
                if (LOWORD(lParam) >= ractangle.left && LOWORD(lParam) <= ractangle.right
                    && HIWORD(lParam) >= ractangle.top && HIWORD(lParam) <= ractangle.bottom) {
                    deletedFlag = true;
                    deletingFlag = false;
                }
                InvalidateRect(hwnd, NULL, false);
            }
            else {
                RECT rect;
                rect.left = 10;
                rect.top = 10.5;
                rect.right = 236;
                rect.bottom = 190.5;
                InvalidateRect(hwnd, &rect, true);
            }
            insertingFlag = false;
            deletingFlag = false;
            InvalidateRect(hwnd, NULL, false);

            // 초기화
            angle = 0;
            boxSize = 0;

            return 0;
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
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;
        case WM_DESTROY:
        {
            PostQuitMessage(0);
        }
        return 1;
        }
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

