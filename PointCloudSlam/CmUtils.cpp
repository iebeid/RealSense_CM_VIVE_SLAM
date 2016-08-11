#include "CmUtils.h"
#include <iostream>
CmDevice* create_device() {
	CmDevice* pCmDev = NULL;
	UINT version = 0;
	int result = ::CreateCmDevice(pCmDev, version);
	if (result != CM_SUCCESS) std::cout << "Cannot create device" << std::endl;
	return pCmDev;
}

CmProgram* load_kernel_program(CmDevice* pCmDev, const char * filename) {
	CmProgram* program = NULL;
	FILE* pISA = fopen(filename, "rb");
	if (pISA == NULL) {
		perror(filename);
	}
	fseek(pISA, 0, SEEK_END);
	int codeSize = ftell(pISA);
	rewind(pISA);
	void *pCommonISACode = (BYTE*)malloc(codeSize);
	if (fread(pCommonISACode, 1, codeSize, pISA) != codeSize) {
		perror(filename);
	}
	fclose(pISA);
	pCmDev->LoadProgram(pCommonISACode, codeSize, program);
	printf("Kernel program loaded\n");
	return program;
}