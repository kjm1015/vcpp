// game.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "game.h"
#include <iostream>
#include <string>
#include <vector>

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

HDC g_hMemDC = NULL;                            // 더블 버퍼링용 HDC, g_hMemBitmap를 담는데 사용
HBITMAP g_hMemBitmap = NULL;                    // 화면 데이터를 저장하는 객체
HBITMAP g_hOldBitmap = NULL;                    // 화면 데이터를 저장하는 객체

const UINT_PTR TIMER_UPDATE = 1;                // 화면 갱신 및 이동 처리 타이머
const UINT_PTR TIMER_SHOOT = 2;                 // 총알 발사 타이머
const UINT_PTR TIMER_SKILL = 3;                 // 적 스킬 타이머

RECT g_enemy;                                   //적
RECT g_me;                                      //나
RECT g_parrying;                                //패링 영역
RECT g_slowZone;                                //슬로우 장판
RECT g_Wall;                                    // 적 스킬 벽

struct bullet                                   //총알 구조체
{
    RECT rc;                                    // 총알의 위치와 크기(Rect)
    int speedX;                                 // 좌우 속도
    int speedY;                                 // 상하 속도
    bool active;                                // 사용 중인지 여부
    bool lifeUp;                                // 체력 회복 아이템인지 아닌지
};

std::vector<bullet> g_bullets;                  // bullet들을 담을 동적 전역 배열

int my_x = 275;                                 // g_me의 x좌표 초기값 
int my_y = 500;                                 // g_me의 y좌표 초기값 상하 이동은 구현 안했기 때문에 바뀔 일은 없을듯하네요
int g_move = 0;                                 // g_me의 이동속도
int g_parry_timer = 0;                          // 패링의 남은 지속 프레임을 담을 변수
int g_parryCoolTime = 0;                        // 패링의 남은 쿨타임을 담을 변수
int g_parryCool = 30;                           // 패링의 쿨타임
int g_enemyLife = 10;                           // 적 체력
int g_myLife = 5;                               // 내 체력
int g_hit_timer = 0;                            // 현재 남은 피격 효과 표시 프레임 값이며 동시에 무적 판정이 되는 프레임
int g_enemy_hit_timer = 0;                      // 현재 남은 적 피격 효과 표시 프레임 값
int g_bullet_pattern = 1;                       // TIMER_SHOOT 안에서 case문의 조건값으로 사용할 변
int g_patternTimer = 0;                         // 패턴 변경 남은 시간
int g_patternInterval = 180;                    // 180프레임마다 변경
int g_slowZoneWarningTimer = 0;                 // 슬로우 장판 예고 효과 유지 프레임을 담을 변수 실제 값은 SpawnSlowZone()에서 초기화하면서 넣음
int g_slowZoneDurationTimer = 0;                // 슬로우 장판 현재 남은 지속 프레임 담을 변수 
int g_WallDurationTimer = 0;                    // 슬로우 장판 지속 시간
int g_WallWarningTimer = 0;                     // 슬로우 장판 예고 효과 유지 프레임을 담을 변수
int g_WallTimer = 0;                            // 벽의 현재 남은 남은 지속 프레임을 담을 변수

bool g_parry = 0;                               // 패링 on off 플래그 변수 이게 TRUE일때만 g_bullets.b와 g_parrying의 겹침 여부를 계산
bool g_hit_effect = false;                      // 피격 효과 on off 플래그 변수
bool g_enemy_hit_effect = false;                // 적 피격 효과 on off 플래그 변수
bool g_slowZoneWarning = false;                 // 슬로우 장판 예고 플래그 변수
bool g_slowZoneActive = false;                  // 슬로우 장판 발동 플래그 변수
bool g_WallWarning = false;                     // 벽 예고 플래그 변수
bool g_WallActive = false;                      // 이동 범위 제한하는 벽 발동 플래그 변수
bool g_used = false;                            // 패턴 한번 당 아이템이 한번씩만 나오도록 하기 위해 선언한 플래그 변수



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

   HWND hWnd = CreateWindowW(
    szWindowClass, szTitle, 
    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,  // 창 크기 조절 막기
    CW_USEDEFAULT, CW_USEDEFAULT,
    700, 700,                       // 윈도우 x,y크기 700으로 고정
    nullptr, nullptr, hInstance, nullptr);

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

//사각형들 위치 갱신하는 함수
void UpdateRect()
{
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

    // 총알 위치 갱신
    for (auto& b : g_bullets)
    {
        if (!b.active) continue;
        b.rc.left += b.speedX;
        b.rc.right += b.speedX;

        b.rc.top += b.speedY;
        b.rc.bottom += b.speedY;

        //유도 패턴 추적 로직 구현
        if (g_bullet_pattern == 4)                              
        {
            if (b.speedY > 0) {                                 // 이거 없으면 패링한 총알도 나의 x좌표를 따라옴
                if (b.rc.left < my_x) b.speedX = 3;
                else if (abs(b.rc.left-my_x)<5) b.speedX = 0;
                else b.speedX = -3;
            }
        }

        if (b.rc.top > 800 || b.rc.bottom < 0 || b.rc.right < 0 || b.rc.left > 2000)                // 총알 화면 밖으로 나가면 비활성화 800, 2000은 일단 넣어둔 값
            b.active = false;
    }
}

//패링
void Parrying()
{
    if (g_parry)                            //패링이 활성화된 상태일때만 총알과 패링 범위의 겹침 계산 실시
    {
        for (auto& b : g_bullets)
        {
            if (!b.active) continue;

            RECT check;
            if (IntersectRect(&check, &g_parrying, &b.rc))
            {
                b.speedY = -8;
                if (g_move < 0)             //이동방향에 따른 총알의 x속도 변화
                {
                    b.speedX = -2;
                }
                else if (g_move > 0)
                {
                    b.speedX = 2;
                }
            }
        }
    }
}

//피격 계산 및 효과
void Hit()
{
    // 총알 피격
    for (auto& b : g_bullets)
    {
        if (!b.active) continue;

        RECT hit;
        if (IntersectRect(&hit, &g_me, &b.rc))
        {
            //회복 아이템일 경우
            if (b.lifeUp)               
            {
                if (g_myLife < 10)     // 최대 체력 제한
                    g_myLife++;

                b.active = false;     // 아이템 제거
                continue;             // 총알 피격 처리로 넘어가지 않게 함
            }
            

            //일반 총알일 경우
            g_myLife--;                     
            b.active = false;              // 총알 비활성화(삭제 처리)

            g_hit_effect = TRUE;           //피격 효과 on? off?
            g_hit_timer = 120;             //피격 후 무적 지속 프레임
        }
    }

    // 총알 적이 맞음
    for (auto& b : g_bullets)
    {
        if (!b.active) continue;

        RECT hit;
        if (IntersectRect(&hit, &g_enemy, &b.rc))
        {
            g_enemyLife--;
            b.active = false;              // 총알 비활성화(메모리에서 지워진건 아님!!!)

            g_enemy_hit_effect = TRUE;           //피격 효과 on? off?
            g_enemy_hit_timer = 15;              //피격 이팩트 지속시간
        }
    }

   

}

//확산 패턴 + 회복 아이템 확률 등장
void PatternSpread()

{
    int enemyCenterX = (g_enemy.left + g_enemy.right) / 2;  // 총알의 생성 위치 설정 때문에 선언 필요

    for (int i = -4; i <= 5; i += 2)                        // 총 5개의 총알 생성
    {
        if (rand() % 20 == 0 && !g_used)            // 5% 확률로 체력 회복 아이템이 등장 but 패턴 1번당 한번만 등장하도록
        {
            bullet item;

            //아이템의 위치와 크기 기본적으로 총알과 동일하나 색깔만 다를 예정
            item.rc = { enemyCenterX - 5, g_enemy.bottom,
                        enemyCenterX + 5, g_enemy.bottom + 10 };

            item.speedX = i;                                
            item.speedY = 5;

            item.active = true;
            item.lifeUp = true;

            g_bullets.push_back(item);
            g_used = true;
        }
        else
        {
            bullet b{};

            // 총알 위치와 크기
            b.rc = { enemyCenterX - 5, g_enemy.bottom,
                     enemyCenterX + 5, g_enemy.bottom + 10 };

            b.speedX = i;               // 
            b.speedY = 5;               // 아래로 이동

            b.active = true;
            b.lifeUp = false;
            g_bullets.push_back(b);
        }
    }
}

//직구 패턴
void PatternStraight()
{
    int enemyCenterX = (g_enemy.left + g_enemy.right) / 2;



    bullet b;
    b.rc = { enemyCenterX - 3, g_enemy.bottom,
             enemyCenterX + 3, g_enemy.bottom + 12 };

    b.speedX = 0;
    b.speedY = 10;
    b.lifeUp = false;                       //쓰레기값 안들어가도록 초기화!
    b.active = true;
    g_bullets.push_back(b);
}

//사인 곡선 패턴 (구현하기 어렵네.....)
//void PatternSine()
//{
//    int enemyCenterX = (g_enemy.left + g_enemy.right) / 2;
//
//    for (int i = -3; i <= 3; i += 2)
//    {
//       bullet b;
//       b.rc = { enemyCenterX - 4, g_enemy.bottom,
//                enemyCenterX + 4, g_enemy.bottom + 8 };
//
//        b.speedX = i;       // 좌우로
//        b.speedY = 6;       // 기본 하강 속도
//
//       b.active = true;
//        g_bullets.push_back(b);
//      }
//}

//유도 패턴
void PatternHoming()
{
    int enemyCenterX = (g_enemy.left + g_enemy.right) / 2;

    bullet b;
    b.rc = { enemyCenterX - 5, g_enemy.bottom,
             enemyCenterX + 5, g_enemy.bottom + 10 };

    b.speedX = 0;
    b.speedY = 5;
    b.lifeUp = false;                       //쓰레기값 안들어가도록 초기화!
    b.active = true;
    g_bullets.push_back(b);
}

//슬로우 장판
void SpawnSlowZone(int left, int top, int right, int bottom)
{
    g_slowZone = {left, top, right, bottom };

    g_slowZoneWarning = true;
    g_slowZoneActive = false;
    g_slowZoneWarningTimer = 150;                   // 슬로우 장판 예고 지속시간
    g_slowZoneDurationTimer = 120;                  // 함수 호출 시 슬로우 장판 시속시간 초기화
}

//이동 범위를 제한하는 벽
void SpawnWall(int left, int top, int right, int bottom)
{
    g_Wall = { left, top, right, bottom };

    g_WallWarning = true;
    g_WallActive = false;
    g_WallWarningTimer = 150;                       // 벽 예고 지속 (1초)
    g_WallDurationTimer = 180;                      // 활성화 지속시간
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    
    case WM_CREATE:
        {
        srand((unsigned)time(NULL)); //랜덤 시드 초기화

        SetTimer(hWnd, TIMER_UPDATE, 16, NULL);  // 지속적인 화면 그리기를 위한 타이머 1, 1초에 약 60프레임
        SetTimer(hWnd, TIMER_SHOOT, 700, NULL);  // 적 공격을 위한 타이머 2, 0.7초
        SetTimer(hWnd, TIMER_SKILL, 2000, NULL); // 적 스킬을 위한 타이머 3, 2초마다 호출

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
                case VK_SPACE: // 패링
                {
                    if (g_parryCoolTime == 0) // 패링 남은 풀타임이 0일시에만 패링 발동
                    {
                        g_parry = TRUE;
                        g_parry_timer = 3; // 3프레임동안 패링 유지

                        g_parryCoolTime = g_parryCool;  // 패링 쿨타임 시작
                    }
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
        if (wParam == TIMER_UPDATE) // 전체적인 타이머
        {
            SHORT left = GetAsyncKeyState(VK_LEFT);
            SHORT right = GetAsyncKeyState(VK_RIGHT);

            g_move = 0;         //g_move 초기화..;
            int speed = 7;
            if (left & 0x8000)  g_move -= 1;
            if (right & 0x8000) g_move += 1;
            
            //슬로우 장판 처리
            if (g_slowZoneActive)                                       //슬로우 장판이 활성화 상태이면
            {
                g_slowZoneDurationTimer--;                              //슬로우 장판 남은 프레임 지속적으로 감소

                if (g_slowZoneDurationTimer <= 0)                       //남은 프레임 0 되면 비활성화
                {
                    g_slowZoneActive = false;
                }
                RECT overlap;
                if (IntersectRect(&overlap, &g_me, &g_slowZone))        //슬로우 장판과 me가 겹침 상태이면 속도 3으로 감소
                    speed = 3;
            }

            // 벽 처리
            if (g_WallActive)
            {
                g_WallTimer--;

                if (g_WallTimer <= 0)
                {
                    g_WallActive = false;
                }
                RECT block;
                if (IntersectRect(&block, &g_me, &g_Wall))
                {
                    if (g_hit_timer == 0)
                    {
                    g_myLife--;
                    g_hit_effect = true;
                    g_hit_timer = 120;
                    }

                    // 왼쪽에서 벽과 충돌한 경우
                    if (g_me.right > g_Wall.left && g_me.left < g_Wall.left)
                    {
                        my_x = g_Wall.left - 50;                                //50 : 내 캐릭터 x길이
                    }
                    // 오른쪽에서 벽과 충돌한 경우
                    else if (g_me.left < g_Wall.right && g_me.right > g_Wall.right)
                    {
                        my_x = g_Wall.right;
                    }

                    // 겹침 해소 후 RECT 갱신
                    UpdateRect();
                }
            }


            my_x += g_move * speed;    // g_me의 좌표를 변경

            if (g_parry_timer > 0)
            {
                g_parry_timer--; // 패링 남은 프레임을프레임마다 
            }
            else
            {
                g_parry = false; // g_parry_timer가 0이 되면 g_parry false로 변경(총알과 패링 영역의 충돌 계산 안하도록)
            }

            if (g_parryCoolTime > 0)
            {
                g_parryCoolTime--;  // 패링 쿨타임 감소
            }

            UpdateRect();           //사각형 위치 갱신
            Hit();                  //피격
            Parrying();             //패링

            // 슬로우 장판 예고
            if (g_slowZoneWarning)
            {
                g_slowZoneWarningTimer--;

                if (g_slowZoneWarningTimer <= 0)
                {
                    g_slowZoneWarning = false;
                    g_slowZoneActive = true;

                    g_slowZoneDurationTimer = 120;
                }
            }

            // 벽 예고 처리
            if (g_WallWarning)
            {
                g_WallWarningTimer--;

                if (g_WallWarningTimer <= 0)
                {
                    g_WallWarning = false;
                    g_WallActive = true;       

                    g_WallTimer = g_WallDurationTimer;   
                }
            }

            // 패턴 변경 
            g_patternTimer--;
            if (g_patternTimer <= 0)
            {
                g_bullet_pattern++;  
                if (g_bullet_pattern > 3) // 패턴 1~5 순환 패턴 추가시 이부분은 따로 수정 필요!!
                    g_bullet_pattern = 1;

                g_patternTimer = g_patternInterval; // 다시 초기화

                if (g_bullet_pattern == 1) 
                {
                    g_used = false;                 //아이템 여러개 안나오도록 하기 위함
                }
            }

            // 피격 이팩트의 남은 프레임을 프레임 하나 업데이트 될 때마다 하나씩 줄임
            if (g_hit_effect)
            {
                g_hit_timer--;              //WM_PAINT에서 blinking이라는 불타입 변수의 값을 결정하기도 함
                if (g_hit_timer <= 0)       //피격 효과 지속시간 종료되면 g_git_effect false로
                    g_hit_effect = FALSE;
            }

            //적 피격 이펙트 구조는 위와 동일
            if (g_enemy_hit_effect)
            {
                g_enemy_hit_timer--;
                if (g_enemy_hit_timer <= 0)
                    g_enemy_hit_effect = false;
            }

            //체력 0 되면 게임 종료
            if (g_myLife == 0)
            {
                KillTimer(hWnd, TIMER_UPDATE);
                KillTimer(hWnd, TIMER_SHOOT);

                MessageBox(hWnd, L"게임 오버!", L"Game Over", MB_OK);
                DestroyWindow(hWnd);
            }
            // 적 체력 0 되면 게임 종료
            if (g_enemyLife == 0)
            {
                KillTimer(hWnd, TIMER_UPDATE);
                KillTimer(hWnd, TIMER_SHOOT);

                MessageBox(hWnd, L"게임 승리!", L"Game Win", MB_OK);
                DestroyWindow(hWnd);
            }

            //비활성화 된 총알 메모리에서 삭제
            g_bullets.erase(
                std::remove_if(g_bullets.begin(), g_bullets.end(),
                    [](const bullet& b) { return !b.active; }),
                g_bullets.end()
            );

            InvalidateRect(hWnd, NULL, FALSE); // 위 갱신된 rect들을 전부 다시 그림
        }

        if (wParam == TIMER_SHOOT) // 적 총알 발사 타이머
        {
            switch (g_bullet_pattern)
            {
            

                case 1:
                {
                    PatternSpread();
                }
                    break;

                case 2:
                {
                    PatternStraight();
                }
                    break;


                case 3:
                {
                    PatternHoming();
                }
                    break;
                //case 4:
                //{
                //    /*PatternSine();*/
                //    
                //}
                //    break;

                default:
                    break;
            }
            
            //기본 공격 패턴
            /* b;

            // 총알 시작 위치 = 적의 중앙
            int enemyCenterX = (g_enemy.left + g_enemy.right) / 2;

            b.rc = { enemyCenterX - 5, g_enemy.bottom, enemyCenterX + 5, g_enemy.bottom + 10 };
            b.speedY = 5;     // 총알의 아래로 이동하는 속도
            b.active = true;

            g_bullets.push_back(b);
            */
        }

        // 적 스킬 타이머
        if (wParam == TIMER_SKILL)
        {
            int skill = rand() % 2;  // 0 = 벽, 1 = 장판

            if (skill == 0)
            {
                if (!g_WallWarning && !g_WallActive)            // 장판 예고가 활성화 or 장판이 활성화 되어있으면 패스
                {
                    int left = rand() % 600 + 50;               // 벽 위치 X (화면 내 랜덤)
                    int top = g_me.top - 50;                    // 벽 Y 위치 (원하면 조절)
                    int right = left + 50;                            // 벽 가로 길이
                    int bottom = g_me.bottom + 50;              // 벽 높이

                    SpawnWall(left, top, right, bottom);
                }
            }
            else
            {
                if (!g_slowZoneWarning && !g_slowZoneActive)            // 장판 예고가 활성화 or 장판이 활성화 되어있으면 패스
                {
                    int screenMin = 0;                                  //장판 생성 x좌표 최소 값 
                    int screenMax = 700;                                //장판 생성 x좌표 최대 값 


                    int left = rand() % (screenMax - 200);
                    int right = left + 200;

                    SpawnSlowZone(left, g_me.top, right, g_me.bottom);
                }
            }
        }


        
    }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...

            //백버퍼 생성
            if(g_hMemDC == NULL)
            {
                g_hMemDC = CreateCompatibleDC(hdc);
                g_hMemBitmap = CreateCompatibleBitmap(hdc, 700, 700);
                g_hOldBitmap = (HBITMAP)SelectObject(g_hMemDC, g_hMemBitmap);
            }

            //백버퍼 초기화
            HBRUSH backBrush = CreateSolidBrush(RGB(255, 255, 255));
            RECT backRect = { 0, 0, 700, 700};
            FillRect(g_hMemDC, &backRect, backBrush);

            DeleteObject(backBrush);
            // 슬로우 장판 예고 그리기
            if (g_slowZoneWarning)
            {
                HPEN warnPen = CreatePen(PS_DOT, 2, RGB(60, 60, 255));
                HPEN oldPen = (HPEN)SelectObject(g_hMemDC, warnPen);

                Rectangle(g_hMemDC, g_slowZone.left, g_slowZone.top, g_slowZone.right, g_slowZone.bottom);

                SelectObject(g_hMemDC, oldPen);
                DeleteObject(warnPen);
            }

            // 활성화 된 슬로우 장판 그리기
            if (g_slowZoneActive)
            {
                HBRUSH brush = CreateSolidBrush(RGB(100, 100, 255)); // 파란 장판
                FillRect(g_hMemDC, &g_slowZone, brush);
                DeleteObject(brush);
            }

            HPEN hPenNormal = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
            HPEN hPenHit = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
            HPEN hEnemyPenNormal = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
            HPEN hEnemyPenHit = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));

            HPEN hOldPen = nullptr;
            HPEN oldEnemyPen = nullptr;

            // 벽 예고 (점선)
            if (g_WallWarning)
            {
                HPEN warnPen = CreatePen(PS_DOT, 2, RGB(255, 80, 80));
                HPEN oldPen = (HPEN)SelectObject(g_hMemDC, warnPen);

                Rectangle(g_hMemDC, g_Wall.left, g_Wall.top, g_Wall.right, g_Wall.bottom);

                SelectObject(g_hMemDC, oldPen);
                DeleteObject(warnPen);
            }

            // 벽 활성화 (채움)
            if (g_WallActive)
            {
                HBRUSH wb = CreateSolidBrush(RGB(255, 80, 80));
                FillRect(g_hMemDC, &g_Wall, wb);
                DeleteObject(wb);
            }


            // 깜빡임 효과: 2프레임 보이고 2프레임 숨김
            bool blinking = (g_hit_effect && (g_hit_timer % 4 < 2));    //g_hit_timer : 현재 남은 피격 효과의 프레임 피격되면 15로 되었다 매 프레임마다 1씩 감소

            if (blinking)
                hOldPen = (HPEN)SelectObject(g_hMemDC, hPenHit);
            else
                hOldPen = (HPEN)SelectObject(g_hMemDC, hPenNormal);

            // 테두리만 그리기
            Rectangle(g_hMemDC, g_me.left, g_me.top, g_me.right, g_me.bottom);\
            
            // 원래 펜 복구
            SelectObject(g_hMemDC, hOldPen);
            DeleteObject(hPenNormal);
            DeleteObject(hPenHit);

            // 적 깜빡임 효과
            bool enemyBlink = (g_enemy_hit_effect && (g_enemy_hit_timer % 4 < 2));

            if (enemyBlink)
                oldEnemyPen = (HPEN)SelectObject(g_hMemDC, hEnemyPenHit);
            else
                oldEnemyPen = (HPEN)SelectObject(g_hMemDC, hEnemyPenNormal);

            // 테두리만 그리기
            
            Rectangle(g_hMemDC, g_enemy.left, g_enemy.top, g_enemy.right, g_enemy.bottom);

            // 원래 펜 복구
            SelectObject(g_hMemDC, oldEnemyPen);
            DeleteObject(hEnemyPenNormal);
            DeleteObject(hEnemyPenHit);


            // 총알(또는 아이템) 그리기
            for (auto& b : g_bullets)
            {
                if (!b.active) continue;

                if (b.lifeUp)   //회복 아이템
                {
                    
                    HBRUSH itemBrush = CreateSolidBrush(RGB(255, 255, 0));
                    FillRect(g_hMemDC, &b.rc, itemBrush);
                    DeleteObject(itemBrush);
                }
                else
                {
                    Rectangle(g_hMemDC, b.rc.left, b.rc.top, b.rc.right, b.rc.bottom);
                }
            }

            //패링 그리기
            if (g_parry) {
                Rectangle(g_hMemDC, g_parrying.left, g_parrying.top, g_parrying.right, g_parrying.bottom);
            }

            /// 내 체력 표시
            wchar_t buffer[32];                             
            swprintf(buffer, 32, L"HP: %d", g_myLife);

            TextOutW(g_hMemDC, 10, 500, buffer, lstrlenW(buffer));

            /// 적 체력 표시
            wchar_t enemyBuffer[32];
            swprintf(enemyBuffer, 32, L"HP: %d", g_enemyLife);

            TextOutW(g_hMemDC, 10, 0, enemyBuffer, lstrlenW(enemyBuffer));

            // 패링 쿨타임 시각화
            if (g_parryCoolTime > 0)
            {
                float ratio = 1.0f - (float)g_parryCoolTime / g_parryCool;  // 0~1 사이

                int fullHeight = g_me.bottom - g_me.top;
                int gaugeHeight = (int)(fullHeight * ratio);

                RECT gauge = {
                    g_me.left,
                    g_me.top,
                    g_me.right,
                    g_me.bottom - gaugeHeight
                };

                HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
                FillRect(g_hMemDC, &gauge, brush);
                DeleteObject(brush);
            }
            //백버퍼에 있는 화면 출력
            BitBlt(hdc, 0, 0, 700, 700, g_hMemDC, 0, 0, SRCCOPY);

            /// 총알 생성 및 삭제 확인용 코드
            /*int activeCount = 0;
            for (auto& b : g_bullets)
                if (b.active) activeCount++;

            std::wstring text = L"Active Bullets: " + std::to_wstring(g_bullets.size());
            TextOut(hdc, 10, 10, text.c_str(), text.length());*/

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        if (g_hMemDC)
        {
            SelectObject(g_hMemDC, g_hOldBitmap);
            DeleteObject(g_hMemBitmap);
            DeleteDC(g_hMemDC);
        }
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
/// 사인파동 공격 패턴 만들기
