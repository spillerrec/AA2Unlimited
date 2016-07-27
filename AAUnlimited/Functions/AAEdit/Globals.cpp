#include "Globals.h"

#include "General\ModuleInfo.h"

namespace AAEdit {


AAUCardData g_cardData;

__declspec(naked) void* __stdcall IllusionMemAlloc(size_t size) {
	//call AA2Edit.exe+1FE160 <-- memory alloc function, only parameter is eax = size
	__asm {
		mov eax, [esp+4]
		mov ecx, [General::GameBase]
		add ecx, 0x1FE160
		call ecx
		ret 4
	}
}

}