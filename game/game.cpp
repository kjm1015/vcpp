// game.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "game.h"
#include <vector>

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GAME, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GAME));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAME));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GAME);
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
const UINT_PTR TIMER_UPDATE = 1;   // 화면 갱신 및 이동 처리 타이머
const UINT_PTR TIMER_SHOOT = 2;   // 총알 발사 타이머

RECT g_enemy;                           //적
RECT g_me;                              //나
RECT g_parrying;                        //패링 영역
struct bullet                         //총알 구조체
{
    RECT rc;                            // 총알의 위치와 크기(Rect)
    int  speedY;                        // 상하 속도
    bool active;                        // 사용 중인지 여부
};
std::vector<bullet> g_bullets;        // bullet들을 담을 동적 전역 배열
int my_x  = 275;                        // g_me의 x좌표 초기값 
int my_y = 500;                         // g_me의 y좌표 초기값 상하 이동은 구현 안하기 떄문에 바뀔 일이 없다
int g_speed = 0;                        // 내 캐릭터의 이동속도
int g_parry_timer = 0;                  // 패링의 남은 지속 프레임을 담을 변수
bool g_parry = 0;                       // 패링 on off 플래그 변수 이게 TRUE일때만 g_bullets.b와 g_parrying의 겹침 여부를 계산 

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    
    case WM_CREATE:
        {
        SetTimer(hWnd, TIMER_UPDATE, 16, NULL);  // 지속적인 화면 그리기를 위한 타이머 1, 다시 그리기는 이걸로만
        SetTimer(hWnd, TIMER_SHOOT, 700, NULL);  // 적 공격을 위한 타이머 2, 700은 임시적인 값입니다.

        // 적 위치와 크기 설정
        g_enemy.left = 300;
        g_enemy.top = 100;
        g_enemy.right = 400;
        g_enemy.bottom = 200;

        // 나의 위치 설정
        g_me.left = my_x;
        g_me.top = my_y;
        g_me.right = my_x + 50;
        g_me.bottom = my_y + 50;


        //패링 범위
        g_parrying.left = g_me.left - 10;
        g_parrying.top = g_me.top - 25;
        g_parrying.right = g_me.right + 10;
        g_parrying.bottom = g_me.top;

        }
            break;
    case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_LEFT: // 왼쪽 이동
                {
                    g_speed = -5;
                    break;
                }
                case VK_RIGHT: // 오른쪽 이동
                {
                    g_speed = +5;
                    break;
                }
                case VK_SPACE: // 패링
                {
                    g_parry = TRUE;
                    g_parry_timer = 5; // 5프레임(?)동안 패링 유지
                    break;
                }

            }
            
            g_me.left = my_x;
            g_me.top = my_y;
            g_me.right = my_x + 50;
            g_me.bottom = my_y + 50;

            g_parrying.left = g_me.left - 10;
            g_parrying.top = g_me.top - 25;
            g_parrying.right = g_me.right + 10;
            g_parrying.bottom = g_me.top;

            break;
        }
            break;

    case WM_KEYUP:
        if (wParam == VK_LEFT || wParam == VK_RIGHT)
            g_speed = 0;   // 방향키 떼면 정지하도록
        break;

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
    case WM_TIMER:
    {   
        if (wParam == TIMER_UPDATE) // 화면 다시 그리기 타이머
        {
            my_x += g_speed;    // g_me의 좌표를 변경

            if (g_parry_timer > 0)
            {
                g_parry_timer--; // 패링 남은 프레임을프레임마다 감소
            }
            else
            {
                g_parry = false; // g_parry_timer가 0이 되면 g_parry false로 변경(총알과 패링 영역의 충돌 계산 안하도록)
            }

            //me 위치 갱신
            g_me.left = my_x;
            g_me.top = my_y;
            g_me.right = my_x + 50;
            g_me.bottom = my_y + 50;

            // parrying 위치 갱신
            g_parrying.left = g_me.left - 10;
            g_parrying.top = g_me.top - 25;
            g_parrying.right = g_me.right + 10;
            g_parrying.bottom = g_me.top;

            // g_bullets 위치 갱신
            for (auto& b : g_bullets)
            {
                if (!b.active) continue;
                b.rc.top += b.speedY;
                b.rc.bottom += b.speedY;

                // 화면 아래 나가면 비활성화
                if (b.rc.top > 1200)                //1200은 임시적인 값입니다.
                    b.active = false;
            }


            if (g_parry)                            //패링이 활성화된 상태라면 총알과 패링 범위의 겹침 계산 실시
            {
                for (auto& b : g_bullets)
                {
                    if (!b.active) continue;

                    RECT check;
                    if (IntersectRect(&check, &g_parrying, &b.rc))
                    {
                        // 패링 성공 → 총알 튕기기
                        b.speedY = -8;
                    }
                }
            }

            InvalidateRect(hWnd, NULL, TRUE); // 위 갱신된 rect들을 전부 다시 그림
        }
        

        if (wParam == TIMER_SHOOT) // 적 총알 발사 타이머
        {
            bullet b;                         //

            // 총알 시작 위치 = 적의 중앙
            int enemyCenterX = (g_enemy.left + g_enemy.right) / 2;

            b.rc = { enemyCenterX - 5, g_enemy.bottom, enemyCenterX + 5, g_enemy.bottom + 10 };
            b.speedY = 5;     // 총알의 아래로 이동하는 속도
            b.active = true;

            g_bullets.push_back(b);
        }
    }
    break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            Rectangle(hdc, g_me.left, g_me.top, g_me.right, g_me.bottom);
            Rectangle(hdc, g_enemy.left, g_enemy.top, g_enemy.right, g_enemy.bottom);
            for (auto& b : g_bullets)
            {
                if (!b.active) continue;
                Rectangle(hdc, b.rc.left, b.rc.top, b.rc.right, b.rc.bottom);
            }
            if (g_parry) {
                Rectangle(hdc, g_parrying.left, g_parrying.top, g_parrying.right, g_parrying.bottom);
            }
            EndPaint(hWnd, &ps);
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

/// 해야할 것들
/// 1.나와 적의 총알과의 겹침 계산 후 체력 감소
/// 2.적과 튕겨낸 총알과의 겹침 계산 후 적 체력 감소
/// 3.패링하지 못 해서 흘러 내린 총알 삭제
/// 4.적 공격 패턴 다양화
/// 5.적 스킬 만들기(슬로우 장판, 이동방해 벽 등등)
/// 6.나에게 도움이 되는 아이템 만들기(체력 회복, 공격력 증가, 이동속도 증가, 패링 판정 강화)
/// 7.