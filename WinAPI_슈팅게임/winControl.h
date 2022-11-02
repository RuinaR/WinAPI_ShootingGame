#pragma once
#include <windows.h>		// 윈도우 헤더파일
#include <math.h>
#include <string.h>		
#include <list>
using namespace std;

typedef struct _POS
{
	double x;
	double y;
}POS;
typedef struct _CirclePos
{
	POS pos;
	int r;
}CirclePos;
typedef struct _Enemy
{
	int hpMax;
	int hpCur;
	CirclePos circlePos;
	POS dir;
}Enemy;
typedef struct _Bullet
{
	CirclePos circlePos;
	POS dir;
}Bullet;

void Init(HINSTANCE hInstance, int nCmdShow);
void ClientToClient(HWND hWndOrg, HWND hWndTarget, POS* pPos);
double LengthPts(int x1, int y1, int x2, int y2);
BOOL InCircle(int x, int y, int mx, int my, int r);
BOOL InRect(const RECT* rect, int mx, int my);
void VectorNormalize(POS* vector);

BOOL SetNoIntersect(const LPRECT pHold, LPRECT pRect);
LRESULT CALLBACK WndProcTitle(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcMain(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcTarget(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcLight(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
HWND MakeWindow(HINSTANCE hInstance, const RECT* rect, WNDPROC winproc, LPCTSTR name, int BackColor, int nCmdShow);
