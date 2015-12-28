#include "stm32f10x_flash.h"
#include "stdbool.h"
#include "string.h"
#include "ctype.h"

#define __markSavedAddr 0x0800F800
static const unsigned int __activeFlag = 0xA5A55A5A;
static const unsigned int __RequiredFlag = 0xF8F88F8F;

struct UpdateMark {
	unsigned int activeFlag;
	char ftpHost[20];
	char remotePath[40];
	unsigned int type;
	unsigned int ftpPort;
	unsigned int timesFlag[5];
};

struct ComUpdateSign{
	unsigned int RequiredFlag;
	unsigned int SizeOfPAK;
	unsigned int type;
	unsigned int timesFlag[5];	
};

static struct UpdateMark *const __mark = (struct UpdateMark *)__markSavedAddr;
static struct ComUpdateSign *const __sign = (struct ComUpdateSign *)__markSavedAddr;

static struct ComUpdateSign __forProgram;


bool isValidUpdateMark(const struct UpdateMark *mark) {
	int i;
	if ((strlen(mark->ftpHost) < 9) || (strlen(mark->ftpHost) > 17)) {
		return false;
	}

	if((strlen(mark->remotePath) < 5) || (strlen(mark->remotePath) >= (sizeof(mark->remotePath)-1))) {
		return false;
	}
	
	if((mark->type != 1) && (mark->type != 2)){
		return false;
	}

	for (i=0; i < strlen(mark->ftpHost); ++i) {
		if( (!isdigit(mark->ftpHost[i])) && (mark->ftpHost[i] != '.')) {
			return false ;
		}
	}

	return true;
}

void eraseupdatemark(void)
{
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(0x0800F800);
	FLASH_Lock();
}

bool setFirmwareUpdate(unsigned int lenth, unsigned int type) {
	int i;
	unsigned int *pint;
	
	__forProgram.RequiredFlag= __RequiredFlag;
	__forProgram.SizeOfPAK = lenth;
	__forProgram.type = type;
	for (i = 0; i < sizeof(__forProgram.timesFlag)/sizeof(__forProgram.timesFlag[0]); ++i) {
		__forProgram.timesFlag[i] = 0xFFFFFFFF;
	}
	
	FLASH_Unlock();	
	FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(__markSavedAddr);
	
	pint = (unsigned int *)&__forProgram;
	
	for (i = 0; i < sizeof(__forProgram)/sizeof(unsigned int); ++i) {
		FLASH_ProgramWord(__markSavedAddr + i*sizeof(unsigned int), *pint++);
	}
	
	FLASH_Lock();
	
	if (__sign->RequiredFlag != __forProgram.RequiredFlag) {
		eraseupdatemark();
		return false;
	}
	
	if(__sign->SizeOfPAK != __forProgram.SizeOfPAK) {
		eraseupdatemark();
		return false;
	}
	
	if(__sign->type != __forProgram.type) {
		eraseupdatemark();
		return false;
	}
	
	for (i = 0; i < sizeof(__forProgram.timesFlag)/sizeof(__forProgram.timesFlag[0]); ++i) {
		if (__sign->timesFlag[i] != __forProgram.timesFlag[i]) {
			eraseupdatemark();
			return false;
		}
	}
		
	return true;
}
