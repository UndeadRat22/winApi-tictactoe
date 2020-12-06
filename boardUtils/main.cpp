#define CROSS 1
#define NOUGHT 2
#define EMPTY 0
#define TIE 10

#include "main.h"

int DLL_EXPORT GetVictor(int* boardValues, int* winningCells){
    int wins[] = {
    /*hor */0,1,2,  3,4,5,  6,7,8,
    /*ver */0,3,6,  1,4,7,  2,5,8,
    /*diag */0,4,8,  2,4,6 };

    //someone won
	for (auto i = 0; i < 24; i+=3)
	{
		if (boardValues[wins[i]] != EMPTY &&
            boardValues[wins[i]] == boardValues[wins[i+1]] &&
            boardValues[wins[i]] == boardValues[wins[i+2]])
		{
			winningCells[0] = wins[i];
			winningCells[1] = wins[i + 1];
			winningCells[2] = wins[i + 2];

			return boardValues[wins[i]];
		}
	}
	//return no victor if board is not full
	for (auto i = 0; i < 9; i++){
        if (boardValues[i] == EMPTY){
            return EMPTY;
        }
	}
	//if board is full, and noone won, then it's a tie
	return TIE;
}


extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to process
            // return FALSE to fail DLL load
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}
