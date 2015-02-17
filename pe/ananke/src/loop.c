/*
	$ Ananke PE Loader  (C) 2005-2012 mfboy
	$ loop.c, 0.1.1027
*/

#include <Windows.h>
#include <Psapi.h>
#include "base.h"
#include "pe.h"


extern process_h	PROCESS;
extern bp_s			*BP;

bp_s *active = NULL;


static BOOL loopBreak(DWORD eip) {
	bp_s *bp = BP->next;

	if(active) {
		bpUpdateTrigger(bp, FALSE);

		active = NULL;
	}

	while(bp != NULL) {
		if((eip >= bp->addrStart) && (eip <= bp->addrEnd)) {
			active = bp;
			bpUpdateTrigger(bp, TRUE);

			return TRUE;
		}
	}

	return FALSE;
}

void loop(DWORD eip) {
	if(loopBreak(eip)) {
		PROCESS.status = BREAK;

		cmdWorking(0, "¶ÏµãÓÚ EIP=%08X", eip);

		while(PROCESS.status != RUN) {
			Sleep(1);
		}
	}
}
