#include "winControl.h"
#include <time.h>

//������ ����
HINSTANCE g_hInst;
static int WindowCount = 0;
int screenWidth;
int screenHeight;
//HWND
HWND hWndTitle = NULL;
HWND hWndMain = NULL;
HWND hWndTarget = NULL;
HWND hWndLight = NULL;

const RECT titleRect = { 500,200,500,500 };
const int lightWinSize = 500;
const RECT lightRect = { 0,0,500,500 };
//��ü�� �ʱ� ũ��, �Ӽ�
const int myR = 50;
const int bulletR = 10;

const int bulletSpeed = 10;
const int enemySpeed = 3;
const int enemyHpMax = 3;
const int bulletLV_MAX = 5;
int bulletLV_CUR;
CirclePos my;
int myHpMax;
int myHpCur;

const int targetWinSize = 500;	

const COLORREF myColor = RGB(255, 255, 0);
const COLORREF enemyColor = RGB(255, 0, 0);
const COLORREF enemyColor_dark = RGB(20, 0, 0);
const COLORREF bulletColor = RGB(0, 0, 255);
const COLORREF whiteColor = RGB(255, 255, 255);
//��ü ����

list<Enemy*>listEnemy;
list<Bullet*>listBullet;

POS targetPos;
static bool isOver = false;

//mainTimer
const static int TimerUpdate = 1;		const static int TimerUpdateUnit = 10;
const static int TimerCreateB = 2;	const static int TimerCreateBUnit = 500;
const static int TimerMoveB = 3;		const static int TimerMoveBUnit = 10;
const static int TimerCreateE = 4;	const static int TimerCreateEUnit = 1000;
const static int TimerMoveE = 5;		const static int TimerMoveEUnit = 10;
const static int TimerColl = 6;			const static int TimerCollUnit = 10;

//����Ʈ, ����
int point;
const int pointUnit = 30;
int score;
const int scoreUnit = 1;

void Init(HINSTANCE hInstance, int nCmdShow)
{
	srand((unsigned int)time(NULL));

	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	hWndTitle = MakeWindow(hInstance, &titleRect, WndProcTitle, TEXT("Ÿ��Ʋ"), WHITE_BRUSH, nCmdShow);	
}

void ClientToClient(HWND hWndOrg, HWND hWndTarget, POS* pPos)
{
	POINT point = { pPos->x, pPos->y };
	ClientToScreen(hWndOrg, &point);
	ScreenToClient(hWndTarget, &point);
	pPos->x = point.x;
	pPos->y = point.y;
}

double LengthPts(int x1, int y1, int x2, int y2)
{
	return(sqrt((float)((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1))));
}

BOOL InCircle(int x, int y, int mx, int my, int r)
{
	if (LengthPts(x, y, mx, my) < r)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL InRect(const RECT* rect, int mx, int my)
{
	if (rect->left < mx &&
		rect->right > mx &&
		rect->top < my &&
		rect->bottom > my)
	{
		return TRUE;
	}
	return FALSE;
}

void VectorNormalize(POS* vector)
{
	double d = sqrt((vector->x * vector->x) + (vector->y * vector->y));
	vector->x = vector->x / d;
	vector->y = vector->y / d;
}

BOOL SetNoIntersect(const LPRECT pHold, LPRECT pRect)
{
	RECT rcInter = { 0 };
	if (IntersectRect(&rcInter, pHold, pRect))
	{
		int nW = rcInter.right - rcInter.left;
		int nH = rcInter.bottom - rcInter.top;

		if (nW > nH)
		{
			if (rcInter.top == pHold->top)
			{
				pRect->top -= nH;
				pRect->bottom -= nH;
			}
			else if (rcInter.bottom == pHold->bottom)
			{
				pRect->top += nH;
				pRect->bottom += nH;
			}
		}
		else
		{
			if (rcInter.left == pHold->left)
			{
				pRect->left -= nW;
				pRect->right -= nW;
			}
			else if (rcInter.right == pHold->right)
			{
				pRect->left += nW;
				pRect->right += nW;
			}
		}
		return TRUE;
	}
	return FALSE;
}

LRESULT CALLBACK WndProcTitle(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static HDC hdc;
	static PAINTSTRUCT ps;
	static RECT startBtnRect = { 100, 80 ,300, 120 };
	static RECT endBtnRect = { 100, 140 ,300, 180 };
	switch (iMessage)
	{
	case WM_CREATE:
	{
		WindowCount++;
		break;
	}
	case WM_LBUTTONDOWN:
	{
		if (InRect(&startBtnRect, LOWORD(lParam), HIWORD(lParam)))
		{
			RECT mainRect = { 0, 0, screenWidth, screenHeight };
			RECT TargetRect = { (screenWidth - targetWinSize) / 2, (screenHeight - targetWinSize) / 2, targetWinSize, targetWinSize };
			hWndMain = MakeWindow(g_hInst, &mainRect, WndProcMain, TEXT("����"), BLACK_BRUSH, SW_SHOW);
			hWndTarget = MakeWindow(g_hInst, &TargetRect, WndProcTarget, TEXT("����"), WHITE_BRUSH, SW_SHOW);
			DestroyWindow(hWnd);
			hWndTitle = NULL;
		}
		else if (InRect(&endBtnRect, LOWORD(lParam), HIWORD(lParam)))
		{
			MessageBox(hWnd, TEXT("�����մϴ�..."), TEXT("����"), MB_OK);
			DestroyWindow(hWnd);
		}
		break;
	}
	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);

		Rectangle(hdc, startBtnRect.left, startBtnRect.top, startBtnRect.right, startBtnRect.bottom);
		TextOut(hdc, startBtnRect.left + 10, (startBtnRect.top + startBtnRect.bottom) / 2, TEXT("���� ����"), lstrlenW(TEXT("���� ����")));

		Rectangle(hdc, endBtnRect.left, endBtnRect.top, endBtnRect.right, endBtnRect.bottom);
		TextOut(hdc, endBtnRect.left + 10, (endBtnRect.top + endBtnRect.bottom) / 2, TEXT("���� ����"), lstrlenW(TEXT("���� ����")));

		EndPaint(hWnd, &ps);
		break;
	}
	case WM_DESTROY:
	{
		WindowCount--;
		if (WindowCount <= 0)
		{
			PostQuitMessage(0);
		}
		break;
	}
	}

	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

LRESULT CALLBACK WndProcMain(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static HDC hdc;
	static PAINTSTRUCT ps;


	//������۸��� ����
	static HDC hdcBuff;
	static HBITMAP hBmpBuff;
	static HBITMAP hBmpBuffOld;
	static RECT clientRect = { 0 };
	//
	static HBRUSH brush;

	switch (iMessage)
	{
	case WM_CREATE:
	{		
		WindowCount++;

		isOver = false;
		GetClientRect(hWnd, &clientRect);
		my.pos.x = clientRect.right / 2;
		my.pos.y = clientRect.bottom - 100;
		my.r = myR;
		myHpMax = 5;
		myHpCur = myHpMax;
		bulletLV_CUR = 0;
		//
		targetPos.x = clientRect.right / 2;
		targetPos.y = clientRect.top;
		point = 0;
		//
		SetTimer(hWnd, TimerUpdate, TimerUpdateUnit, NULL);
		SetTimer(hWnd, TimerCreateB, TimerCreateBUnit, NULL);
		SetTimer(hWnd, TimerMoveB, TimerMoveBUnit, NULL);
		SetTimer(hWnd, TimerCreateE, TimerCreateEUnit, NULL);
		SetTimer(hWnd, TimerMoveE, TimerMoveEUnit, NULL);
		SetTimer(hWnd, TimerColl, TimerCollUnit, NULL);
		break;
	}
	case WM_ACTIVATE:
	{
		if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
		{
			if (hWndTarget != NULL)
			{
				SetActiveWindow(hWndTarget);
			}
		}
		//if (HIWORD(wParam))
		//{
		//	ShowWindow(hWnd, SW_SHOW);
		//}
		break;
	}
	case WM_GETMINMAXINFO:
	{
		((MINMAXINFO*)lParam)->ptMaxTrackSize.x = screenWidth;
		((MINMAXINFO*)lParam)->ptMaxTrackSize.y = screenHeight;
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = screenWidth;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = screenHeight;
		break;
	}
	case WM_TIMER:
	{
		switch (wParam)
		{
		case TimerUpdate:
		{
			score += scoreUnit;
			if (isOver)
			{
				KillTimer(hWnd, TimerUpdate);
				isOver = false;
				WCHAR text[30] = { 0 };
				wsprintf(text, TEXT("SCORE : %d"), point);
				MessageBox(hWnd, text, TEXT("���� ����"), MB_OK);
				hWndTitle = MakeWindow(g_hInst, &titleRect, WndProcTitle, TEXT("Ÿ��Ʋ"), WHITE_BRUSH, SW_SHOW);
				DestroyWindow(hWndTarget);
				DestroyWindow(hWndMain);
				hWndMain = NULL;
				hWndTarget = NULL;
				if (hWndLight)
				{
					DestroyWindow(hWndLight);
					hWndLight = NULL;
				}
			}
			InvalidateRect(hWnd, NULL, false);
			break;
		}
		case TimerCreateB:
		{
			Bullet* newBullet = new Bullet;
			newBullet->circlePos.pos = my.pos;
			newBullet->circlePos.r = bulletR;
			newBullet->dir.x = targetPos.x - my.pos.x;
			newBullet->dir.y = targetPos.y - my.pos.y;
			VectorNormalize(&(newBullet->dir));

			listBullet.push_back(newBullet);
			break;
		}
		case TimerMoveB:
		{
			list<Bullet*>::iterator iter = listBullet.begin();
			while (iter != listBullet.end())
			{
				(*iter)->circlePos.pos.x += (*iter)->dir.x * bulletSpeed;
				(*iter)->circlePos.pos.y += (*iter)->dir.y * bulletSpeed;

				//ȭ�� �� �浹
				if ((*iter)->circlePos.pos.x <= -200 ||
					(*iter)->circlePos.pos.x >= clientRect.right + 200 ||
					(*iter)->circlePos.pos.y <= -200 ||
					(*iter)->circlePos.pos.y >= clientRect.bottom + 200)
				{
					delete (*iter);
					list<Bullet*>::iterator deleteIter = iter;
					iter++;
					listBullet.erase(deleteIter);
				}
				else
				{
					iter++;
				}
			}
			break;
		}
		case TimerCreateE:
		{
			Enemy* newEnemy = new Enemy;
			newEnemy->circlePos.pos.x = rand() % clientRect.right;
			newEnemy->circlePos.pos.y = clientRect.top;
			newEnemy->circlePos.r = 50 + (rand() % 50);
			newEnemy->dir.x = my.pos.x - newEnemy->circlePos.pos.x;
			newEnemy->dir.y = my.pos.y - newEnemy->circlePos.pos.y;
			newEnemy->hpMax = enemyHpMax;
			newEnemy->hpCur = newEnemy->hpMax;
			VectorNormalize(&(newEnemy->dir));
			listEnemy.push_back(newEnemy);
			break;
		}
		case TimerMoveE:
		{
			list<Enemy*>::iterator iter = listEnemy.begin();
			while (iter != listEnemy.end())
			{
				(*iter)->circlePos.pos.x += (*iter)->dir.x * enemySpeed;
				(*iter)->circlePos.pos.y += (*iter)->dir.y * enemySpeed;
				//���� �浹
				if (InCircle((*iter)->circlePos.pos.x, (*iter)->circlePos.pos.y, my.pos.x, my.pos.y, (*iter)->circlePos.r + my.r))
				{
					delete (*iter);
					list<Enemy*>::iterator deleteIter = iter;
					iter++;
					listEnemy.erase(deleteIter);
					//�߰�ó��
					myHpCur--;
					if (myHpCur <= 0)	isOver = true;
				}
				//ȭ�� �� �浹 (������ġ)
				else if ((*iter)->circlePos.pos.x <= -200 ||
					(*iter)->circlePos.pos.x >= screenWidth + 200 ||
					(*iter)->circlePos.pos.y <= -200 ||
					(*iter)->circlePos.pos.x >= screenHeight + 200)
				{
					delete (*iter);
					list<Enemy*>::iterator deleteIter = iter;
					iter++;
					listEnemy.erase(deleteIter);
				}
				else
				{
					iter++;
				}
			}
			break;
		}
		case TimerColl:
		{
			list<Bullet*>::iterator iterB = listBullet.begin();
			list<Enemy*>::iterator iterE;
			bool isDeleteE = false;
			bool isDeleteB = false;
			while (iterB != listBullet.end())
			{
				isDeleteE = false;
				isDeleteB = false;

				iterE = listEnemy.begin();
				while (iterE != listEnemy.end())
				{
					//enemy - bullet �浹
					if(InCircle((*iterB)->circlePos.pos.x, (*iterB)->circlePos.pos.y,
						(*iterE)->circlePos.pos.x, (*iterE)->circlePos.pos.y,
						(*iterB)->circlePos.r + (*iterE)->circlePos.r))
					{
						delete (*iterB);
						list<Bullet*>::iterator deleteIterB = iterB;
						iterB++;
						listBullet.erase(deleteIterB);
						isDeleteB = true;

						(*iterE)->hpCur--;
						if ((*iterE)->hpCur <= 0)
						{
							point += pointUnit;
							delete (*iterE);
							list<Enemy*>::iterator deleteIterE = iterE;
							iterE++;
							listEnemy.erase(deleteIterE);
							isDeleteE = true;
						}
						break;
					}
					if (!isDeleteE)	iterE++;
				}
				if (!isDeleteB)	iterB++;
			}
			break;
		}
		}
		break;
	}
	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);

		//���� ���۸��� DC, BMP
		hdcBuff = CreateCompatibleDC(hdc);
		hBmpBuff = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
		hBmpBuffOld = (HBITMAP)SelectObject(hdcBuff, hBmpBuff);
		PatBlt(hdcBuff, 0, 0, clientRect.right, clientRect.bottom, BLACKNESS);

		//���ۿ� �׸���
		//enemy
		brush = CreateSolidBrush(enemyColor_dark);
		SelectObject(hdcBuff, brush);

		for (list<Enemy*>::iterator iter = listEnemy.begin(); iter != listEnemy.end(); iter++)
		{
			Ellipse(hdcBuff, (*iter)->circlePos.pos.x - (*iter)->circlePos.r, (*iter)->circlePos.pos.y - (*iter)->circlePos.r,
				(*iter)->circlePos.pos.x + (*iter)->circlePos.r, (*iter)->circlePos.pos.y + (*iter)->circlePos.r);
		}

		DeleteObject(brush);
		//bullet
		brush = CreateSolidBrush(whiteColor);
		SelectObject(hdcBuff, brush);

		for (list<Bullet*>::iterator iter = listBullet.begin(); iter != listBullet.end(); iter++)
		{
			Ellipse(hdcBuff, (*iter)->circlePos.pos.x - (*iter)->circlePos.r, (*iter)->circlePos.pos.y - (*iter)->circlePos.r,
				(*iter)->circlePos.pos.x + (*iter)->circlePos.r, (*iter)->circlePos.pos.y + (*iter)->circlePos.r);
		}

		DeleteObject(brush);
		//my
		brush = CreateSolidBrush(whiteColor);
		SelectObject(hdcBuff, brush);

		Ellipse(hdcBuff, my.pos.x - my.r, my.pos.y - my.r, my.pos.x + my.r, my.pos.y + my.r);

		DeleteObject(brush);
		// UI
		WCHAR str[50] = { 0 };
		wsprintf(str, TEXT("���� �� �� : %d"), listEnemy.size());
		TextOut(hdcBuff, clientRect.right - 300, clientRect.bottom - 200, str, lstrlen(str));
		wsprintf(str, TEXT("SCORE : %d"), score);
		TextOut(hdcBuff, clientRect.right - 300, clientRect.bottom - 100, str, lstrlen(str));
		//���� ���۸� ����
		BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, hdcBuff, 0, 0, SRCCOPY);
		SelectObject(hdcBuff, hBmpBuffOld);
		//delete
		DeleteObject(hBmpBuff);
		DeleteDC(hdcBuff);

		EndPaint(hWnd, &ps);
		break;
	}
	case WM_DESTROY:
	{
		for (list<Bullet*>::iterator iter = listBullet.begin(); iter != listBullet.end(); iter++)
		{
			delete (*iter);
		}
		listBullet.clear();
		for (list<Enemy*>::iterator iter = listEnemy.begin(); iter != listEnemy.end(); iter++)
		{
			delete (*iter);
		}
		listEnemy.clear();

		KillTimer(hWnd, TimerUpdate);
		KillTimer(hWnd, TimerCreateB);
		KillTimer(hWnd, TimerMoveB);
		KillTimer(hWnd, TimerCreateE);
		KillTimer(hWnd, TimerMoveE);
		KillTimer(hWnd, TimerColl);

		hWndMain = NULL;

		WindowCount--;
		if (WindowCount <= 0)
		{
			PostQuitMessage(0);
		}
		break;
	}
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

LRESULT WndProcTarget(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static HDC hdc;
	static PAINTSTRUCT ps;
	//Ÿ�̸� ����
	const static int TimerUpdate = 1;	const static int TimerUpdateUnit = 10;
	//������۸��� ����
	static HDC hdcBuff;
	static HBITMAP hBmpBuff;
	static HBITMAP hBmpBuffOld;
	static RECT clientRect = { 0 };
	//���� ���� ����
	const static RECT btnHP = { 300,300,470,330 };
	const static RECT btnCBTime = { 300,340,470,370 };
	const static RECT btnLight = { 300,380,470,410 };
	const static POINT pointPos = { 300, 420 };
	const static int price_hp = 100;
	const static int price_CBTime = 150;
	const static int price_light = 200;

	//��ǥ�� ��ȯ�� ����
	static POS tmpPos = { 0 };
	//
	static HBRUSH brush;
	static HPEN pen;
	static HPEN oldPen;
	const static int targetSize = 20;
	switch (iMessage)
	{
	case WM_CREATE:
	{
		WindowCount++;
		GetClientRect(hWnd, &clientRect);
		SetTimer(hWnd, TimerUpdate, TimerUpdateUnit, NULL);
		break;
	}
	case WM_GETMINMAXINFO:
	{
		((MINMAXINFO*)lParam)->ptMaxTrackSize.x = targetWinSize;
		((MINMAXINFO*)lParam)->ptMaxTrackSize.y = targetWinSize;
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = targetWinSize;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = targetWinSize;
		break;
	}
	case WM_WINDOWPOSCHANGED:
	{
		targetPos.x = clientRect.right / 2;
		targetPos.y = clientRect.bottom / 2;
		ClientToClient(hWnd, hWndMain, &targetPos);
		break;
	}
	case WM_TIMER:
	{
		switch (wParam)
		{
		case TimerUpdate:
		{
			InvalidateRect(hWnd, NULL, false);
			break;
		}
		}
		break;
	}
	case WM_KEYDOWN:
		//�׽�Ʈ��
		if(wParam == VK_ESCAPE)
			isOver = true;
		break;
	case WM_LBUTTONDOWN:
		if (InRect(&btnHP, LOWORD(lParam), HIWORD(lParam)))
		{
			if (myHpCur + 1 <= myHpMax && point >= price_hp)
			{
				myHpCur++;
				point -= price_hp;
			}
		}
		if (InRect(&btnCBTime, LOWORD(lParam), HIWORD(lParam)))
		{
			if (bulletLV_CUR + 1 <= bulletLV_MAX && point >= price_CBTime)
			{
				bulletLV_CUR++;
				point -= price_CBTime;
				KillTimer(hWndMain, TimerCreateB);
				SetTimer(hWndMain, TimerCreateB, TimerCreateBUnit - (bulletLV_CUR * 70), NULL);
			}
		}
		if (InRect(&btnLight, LOWORD(lParam), HIWORD(lParam)))
		{
			if (!hWndLight)
			{
				hWndLight = MakeWindow(g_hInst, &lightRect, WndProcLight, TEXT("��!!!"), WHITE_BRUSH, SW_SHOW);
			}
		}
		break;
	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);

		//���� ���۸��� DC, BMP
		hdcBuff = CreateCompatibleDC(hdc);
		hBmpBuff = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
		hBmpBuffOld = (HBITMAP)SelectObject(hdcBuff, hBmpBuff);
		PatBlt(hdcBuff, 0, 0, clientRect.right, clientRect.bottom, WHITENESS);

		//���ۿ� �׸���
		//enemy
		for (list<Enemy*>::iterator iter = listEnemy.begin(); iter != listEnemy.end(); iter++)
		{
			tmpPos = (*iter)->circlePos.pos;
			ClientToClient(hWndMain, hWnd, &tmpPos);

			//�� ��ü �׸���
			brush = CreateSolidBrush(enemyColor);
			SelectObject(hdcBuff, brush);
			Ellipse(hdcBuff, tmpPos.x - (*iter)->circlePos.r, tmpPos.y - (*iter)->circlePos.r,
				tmpPos.x + (*iter)->circlePos.r, tmpPos.y + (*iter)->circlePos.r);
			DeleteObject(brush);

			//�� HP�� �׸���
			//HpMax
			brush = CreateSolidBrush(RGB(45,45,45));
			SelectObject(hdcBuff, brush);
			Rectangle(hdcBuff, tmpPos.x - (*iter)->circlePos.r, tmpPos.y - ((*iter)->circlePos.r / 5.0),
				tmpPos.x + (*iter)->circlePos.r, (tmpPos.y + (*iter)->circlePos.r / 5.0));
			DeleteObject(brush);
			//HpCur
			brush = CreateSolidBrush(RGB(0, 255, 0));
			SelectObject(hdcBuff, brush);
			Rectangle(hdcBuff, 
				tmpPos.x - (*iter)->circlePos.r, 
				tmpPos.y - ((*iter)->circlePos.r / 5.0),
				tmpPos.x - (*iter)->circlePos.r + ((*iter)->circlePos.r * 2.0) * ((double)(*iter)->hpCur / (double)(*iter)->hpMax), 
				(tmpPos.y + (*iter)->circlePos.r / 5.0));
			DeleteObject(brush);
		}
		
		//bullet
		brush = CreateSolidBrush(bulletColor);
		SelectObject(hdcBuff, brush);
		for (list<Bullet*>::iterator iter = listBullet.begin(); iter != listBullet.end(); iter++)
		{
			tmpPos = (*iter)->circlePos.pos;
			ClientToClient(hWndMain, hWnd, &tmpPos);
			Ellipse(hdcBuff, tmpPos.x - (*iter)->circlePos.r, tmpPos.y - (*iter)->circlePos.r,
				tmpPos.x + (*iter)->circlePos.r, tmpPos.y + (*iter)->circlePos.r);
		}

		DeleteObject(brush);
		//my
		brush = CreateSolidBrush(myColor);
		SelectObject(hdcBuff, brush);
		tmpPos = my.pos;
		ClientToClient(hWndMain, hWnd, &tmpPos);
		Ellipse(hdcBuff, tmpPos.x - my.r, tmpPos.y - my.r, tmpPos.x + my.r, tmpPos.y + my.r);
		DeleteObject(brush);
		//myhp
		//HpMax
		brush = CreateSolidBrush(RGB(45, 45, 45));
		SelectObject(hdcBuff, brush);
		Rectangle(hdcBuff, tmpPos.x - my.r, tmpPos.y - (my.r / 5.0),
			tmpPos.x + my.r, (tmpPos.y + my.r / 5.0));
		DeleteObject(brush);
		//HpCur
		brush = CreateSolidBrush(RGB(0, 255, 0));
		SelectObject(hdcBuff, brush);
		Rectangle(hdcBuff,
			tmpPos.x - my.r,
			tmpPos.y - my.r / 5.0,
			tmpPos.x - my.r + (my.r * 2.0) * ((double)myHpCur / (double)myHpMax),
			(tmpPos.y + my.r / 5.0));
		DeleteObject(brush);

		//target
		brush = CreateSolidBrush(RGB(255, 255, 255));
		pen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
		SelectObject(hdcBuff, brush);
		oldPen = (HPEN)SelectObject(hdcBuff, pen);

		Ellipse(hdcBuff, clientRect.right / 2 - targetSize, clientRect.bottom / 2 - targetSize, clientRect.right / 2 + targetSize, clientRect.bottom / 2 + targetSize);
		MoveToEx(hdcBuff, clientRect.right / 2, clientRect.bottom / 2 - targetSize, NULL);
		LineTo(hdcBuff, clientRect.right / 2, clientRect.bottom / 2 + targetSize);
		MoveToEx(hdcBuff, clientRect.right / 2 - targetSize, clientRect.bottom / 2, NULL);
		LineTo(hdcBuff, clientRect.right / 2 + targetSize, clientRect.bottom / 2);

		SelectObject(hdcBuff, oldPen);
		DeleteObject(brush);
		DeleteObject(pen);
		//����
		{
			WCHAR str[20];
			if (point >= price_hp)
				brush = CreateSolidBrush(RGB(127, 255, 127));
			else
				brush = CreateSolidBrush(RGB(255, 127, 127));
			SelectObject(hdcBuff, brush);
			Rectangle(hdcBuff, btnHP.left, btnHP.top, btnHP.right, btnHP.bottom);
			wsprintf(str, TEXT("HP���� : %dp"), price_hp);
			TextOut(hdcBuff, btnHP.left + 10, (btnHP.top + btnHP.bottom) / 2 - 10, str, lstrlen(str));
			DeleteObject(brush);

			if (point >= price_CBTime && bulletLV_CUR + 1 <= bulletLV_MAX )
				brush = CreateSolidBrush(RGB(127, 255, 127));
			else
				brush = CreateSolidBrush(RGB(255, 127, 127));
			SelectObject(hdcBuff, brush);
			Rectangle(hdcBuff, btnCBTime.left, btnCBTime.top, btnCBTime.right, btnCBTime.bottom);
			if(bulletLV_CUR + 1 <= bulletLV_MAX)	wsprintf(str, TEXT("����ӵ�UP : %dp"), price_CBTime);
			else wsprintf(str, TEXT("LV MAX"));
			TextOut(hdcBuff, btnCBTime.left + 10, (btnCBTime.top + btnCBTime.bottom) / 2 - 10, str, lstrlen(str));
			DeleteObject(brush);

			if (point >= price_light && !hWndLight)
				brush = CreateSolidBrush(RGB(127, 255, 127));
			else
				brush = CreateSolidBrush(RGB(255, 127, 127));
			SelectObject(hdcBuff, brush);
			Rectangle(hdcBuff, btnLight.left, btnLight.top, btnLight.right, btnLight.bottom);
			if(!hWndLight) wsprintf(str, TEXT("�� ���� : %dp"), price_light);
			else wsprintf(str, TEXT("Sold out"));
			TextOut(hdcBuff, btnLight.left + 10, (btnLight.top + btnLight.bottom) / 2 - 10, str, lstrlen(str));
			DeleteObject(brush);

			wsprintf(str, TEXT("POINT : %d"), point);
			TextOut(hdcBuff, pointPos.x, pointPos.y, str, lstrlen(str));
		}
		//���� ���۸� ����
		BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, hdcBuff, 0, 0, SRCCOPY);
		SelectObject(hdcBuff, hBmpBuffOld);
		//delete
		DeleteObject(hBmpBuff);
		DeleteDC(hdcBuff);
		EndPaint(hWnd, &ps);
		break;
	}

	case WM_DESTROY:
	{
		KillTimer(hWnd, TimerUpdate);

		hWndTarget = NULL;

		WindowCount--;
		if (WindowCount <= 0)
		{
			PostQuitMessage(0);
		}
		break;
	}
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

LRESULT WndProcLight(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static HDC hdc;
	static PAINTSTRUCT ps;
	//Ÿ�̸� ����
	const static int TimerUpdate = 1;	const static int TimerUpdateUnit = 10;
	//������۸��� ����
	static HDC hdcBuff;
	static HBITMAP hBmpBuff;
	static HBITMAP hBmpBuffOld;
	static RECT clientRect = { 0 };

	//��ǥ�� ��ȯ�� ����
	static POS tmpPos = { 0 };
	//
	static HBRUSH brush;
	static HPEN pen;
	static HPEN oldPen;
	switch (iMessage)
	{
	case WM_CREATE:
	{
		WindowCount++;
		GetClientRect(hWnd, &clientRect);
		SetTimer(hWnd, TimerUpdate, TimerUpdateUnit, NULL);
		break;
	}
	case WM_GETMINMAXINFO:
	{
		((MINMAXINFO*)lParam)->ptMaxTrackSize.x = lightWinSize;
		((MINMAXINFO*)lParam)->ptMaxTrackSize.y = lightWinSize;
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = lightWinSize;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = lightWinSize;
		break;
	}
	case WM_TIMER:
	{
		switch (wParam)
		{
		case TimerUpdate:
		{
			InvalidateRect(hWnd, NULL, false);
			break;
		}
		}
		break;
	}
	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);

		//���� ���۸��� DC, BMP
		hdcBuff = CreateCompatibleDC(hdc);
		hBmpBuff = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
		hBmpBuffOld = (HBITMAP)SelectObject(hdcBuff, hBmpBuff);
		PatBlt(hdcBuff, 0, 0, clientRect.right, clientRect.bottom, WHITENESS);

		//���ۿ� �׸���
		//enemy
		for (list<Enemy*>::iterator iter = listEnemy.begin(); iter != listEnemy.end(); iter++)
		{
			tmpPos = (*iter)->circlePos.pos;
			ClientToClient(hWndMain, hWnd, &tmpPos);

			//�� ��ü �׸���
			brush = CreateSolidBrush(enemyColor);
			SelectObject(hdcBuff, brush);
			Ellipse(hdcBuff, tmpPos.x - (*iter)->circlePos.r, tmpPos.y - (*iter)->circlePos.r,
				tmpPos.x + (*iter)->circlePos.r, tmpPos.y + (*iter)->circlePos.r);
			DeleteObject(brush);

			//�� HP�� �׸���
			//HpMax
			brush = CreateSolidBrush(RGB(45, 45, 45));
			SelectObject(hdcBuff, brush);
			Rectangle(hdcBuff, tmpPos.x - (*iter)->circlePos.r, tmpPos.y - ((*iter)->circlePos.r / 5.0),
				tmpPos.x + (*iter)->circlePos.r, (tmpPos.y + (*iter)->circlePos.r / 5.0));
			DeleteObject(brush);
			//HpCur
			brush = CreateSolidBrush(RGB(0, 255, 0));
			SelectObject(hdcBuff, brush);
			Rectangle(hdcBuff,
				tmpPos.x - (*iter)->circlePos.r,
				tmpPos.y - ((*iter)->circlePos.r / 5.0),
				tmpPos.x - (*iter)->circlePos.r + ((*iter)->circlePos.r * 2.0) * ((double)(*iter)->hpCur / (double)(*iter)->hpMax),
				(tmpPos.y + (*iter)->circlePos.r / 5.0));
			DeleteObject(brush);
		}

		//bullet
		brush = CreateSolidBrush(bulletColor);
		SelectObject(hdcBuff, brush);
		for (list<Bullet*>::iterator iter = listBullet.begin(); iter != listBullet.end(); iter++)
		{
			tmpPos = (*iter)->circlePos.pos;
			ClientToClient(hWndMain, hWnd, &tmpPos);
			Ellipse(hdcBuff, tmpPos.x - (*iter)->circlePos.r, tmpPos.y - (*iter)->circlePos.r,
				tmpPos.x + (*iter)->circlePos.r, tmpPos.y + (*iter)->circlePos.r);
		}

		DeleteObject(brush);
		//my
		brush = CreateSolidBrush(myColor);
		SelectObject(hdcBuff, brush);
		tmpPos = my.pos;
		ClientToClient(hWndMain, hWnd, &tmpPos);
		Ellipse(hdcBuff, tmpPos.x - my.r, tmpPos.y - my.r, tmpPos.x + my.r, tmpPos.y + my.r);
		DeleteObject(brush);
		//myhp
		//HpMax
		brush = CreateSolidBrush(RGB(45, 45, 45));
		SelectObject(hdcBuff, brush);
		Rectangle(hdcBuff, tmpPos.x - my.r, tmpPos.y - (my.r / 5.0),
			tmpPos.x + my.r, (tmpPos.y + my.r / 5.0));
		DeleteObject(brush);
		//HpCur
		brush = CreateSolidBrush(RGB(0, 255, 0));
		SelectObject(hdcBuff, brush);
		Rectangle(hdcBuff,
			tmpPos.x - my.r,
			tmpPos.y - my.r / 5.0,
			tmpPos.x - my.r + (my.r * 2.0) * ((double)myHpCur / (double)myHpMax),
			(tmpPos.y + my.r / 5.0));
		DeleteObject(brush);

		//���� ���۸� ����
		BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, hdcBuff, 0, 0, SRCCOPY);
		SelectObject(hdcBuff, hBmpBuffOld);
		//delete
		DeleteObject(hBmpBuff);
		DeleteDC(hdcBuff);
		EndPaint(hWnd, &ps);
		break;
	}

	case WM_DESTROY:
	{
		KillTimer(hWnd, TimerUpdate);

		hWndLight = NULL;

		WindowCount--;
		if (WindowCount <= 0)
		{
			PostQuitMessage(0);
		}
		break;
	}
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

HWND MakeWindow(HINSTANCE hInstance, const RECT* rect, WNDPROC winproc, LPCTSTR name, int BackColor, int nCmdShow)
{										//������ �ڵ� ����
	MSG Message;										//�޼��� ����ü ���� ����
	WNDCLASS WndClass;									//Windows Class ����ü ���� ����
	g_hInst = hInstance;								//hInstance���� �ܺο����� ����� �� �ֵ��� ���������� ���� ����

	WndClass.cbClsExtra = 0;							//���� ����. ������ ���X
	WndClass.cbWndExtra = 0;							//���� ����
	WndClass.hbrBackground = (HBRUSH)GetStockObject(BackColor);	// �������� ��� ������ ����
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);		//�������� ���콺������ ����� ����
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);	//�������� Ŀ�� ������ ��� ����
	WndClass.hInstance = hInstance;						//������ Ŭ������ ����ϴ� ���α׷� ��ȣ
	WndClass.lpfnWndProc = winproc;						//������ �޼��� ó�� �Լ� ����
	WndClass.lpszClassName = name;					//������ Ŭ������ �̸� ����
	WndClass.lpszMenuName = NULL;						//�޴� ����
	WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_NOCLOSE;			//�������� ��Ÿ���� ����

	RegisterClass(&WndClass);							//WNDCLASS ����ü�� ������ ����

	HWND hWnd = CreateWindow(name, name,			//�����츦 ����
		WS_OVERLAPPEDWINDOW,
		rect->left, rect->top, rect->right, rect->bottom, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	return hWnd;
}


