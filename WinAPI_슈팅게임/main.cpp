#include "winControl.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	MSG Message;
	Init(hInstance, nCmdShow);
	//메세지 루프를 실행
	while (GetMessage(&Message, NULL, 0, 0)) {			//Queue에 있는 메세지를 읽어들인다
		TranslateMessage(&Message);								//키보드 입력 메세지를 가공하여 프로그램에서 쉽게 
																				//  사용할 수 있도록
		DispatchMessage(&Message);								//메세지를 윈도우의 메세지 처리 함수 WndProc로 전달
	}

	return (int)Message.wParam;							//탈출 코드. 프로그램 종료
}

