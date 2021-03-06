#include "PcConversation.h"
#include "MemMods/Hook.h"
#include "General/ModuleInfo.h"
#include "External/ExternalClasses.h"
#include "Functions/AAPlay/HAi.h"

namespace PlayInjections {
/*
 * Events that are thrown when a npc talks to a pc.
 */
namespace PcConversation {

/********************
 * General tick Event
 ********************/

void __stdcall GeneralPreTick(ExtClass::MainConversationStruct* param) {

}

void __stdcall GeneralPostTick(ExtClass::MainConversationStruct* param) {

}

/********************
 * NPC -> PC interactive conversation tick event
 *******************/
void __stdcall NpcPcInteractivePreTick(ExtClass::NpcPcInteractiveConversationStruct* param) {
	
}

void __stdcall NpcPcInteractivePostTick(ExtClass::NpcPcInteractiveConversationStruct* param) {
	HAi::ConversationTickPost(param);
}

/********************
* NPC -> PC non interactive conversation tick event
*******************/

void __stdcall NpcPcNonInteractivePreTick(ExtClass::NpcPcNonInteractiveConversationStruct* param) {

}

void __stdcall NpcPcNonInteractivePostTick(ExtClass::NpcPcNonInteractiveConversationStruct* param) {
	HAi::ConversationTickPost(param);
}

/********************
 * Called after the NPC sets the response-member of the param struct to answer something.
 * Parameter type is whatever it currently is
 ********************/
void __stdcall NpcAnswer(ExtClass::BaseConversationStruct* param) {
	
}

/*******************
 * Called before the response of the PC in playerAnswer is copied into the response.
 * Parameter type is whatever it currently is
 *******************/
void __stdcall PcAnswer(ExtClass::BaseConversationStruct* param) {
	HAi::ConversationPcResponse(param);
}


/***
* Called for every pc conversation tick. do distributes to other callbacks. do not touch.
*/
void __stdcall PreTick(ExtClass::MainConversationStruct* param) {
	GeneralPreTick(param);
	DWORD substruct = param->GetSubstructType();
	switch (substruct) {
	case ExtClass::PCCONTYPE_NPCPC_GIVEANSWER:
		NpcPcInteractivePreTick((ExtClass::NpcPcInteractiveConversationStruct*) (((DWORD)(param)+param->SubstructOffsets[substruct])));
		break;
	case ExtClass::PCCONTYPE_NPCPC_NOASNWER:
		NpcPcNonInteractivePreTick((ExtClass::NpcPcNonInteractiveConversationStruct*)(((DWORD)(param)+param->SubstructOffsets[substruct])));
		break;
	default:
		break;
	}
}

void __stdcall PostTick(ExtClass::MainConversationStruct* param) {
	GeneralPostTick(param);
	DWORD substruct = param->GetSubstructType();
	switch (substruct) {
	case ExtClass::PCCONTYPE_NPCPC_GIVEANSWER:
		NpcPcInteractivePostTick((ExtClass::NpcPcInteractiveConversationStruct*) (((DWORD)(param)+param->SubstructOffsets[substruct])));
		break;
	case ExtClass::PCCONTYPE_NPCPC_NOASNWER:
		NpcPcNonInteractivePostTick((ExtClass::NpcPcNonInteractiveConversationStruct*)(((DWORD)(param)+param->SubstructOffsets[substruct])));
		break;
	default:
		break;
	}
}


DWORD OriginalTickFunction;
void __declspec(naked) TickRedirect() {
	__asm {
		mov eax, [edi+0x2C]
		push eax
		call PreTick
		mov eax, [edi+ 0x2C]
		call [OriginalTickFunction]
		push eax //rescue return value
		mov eax, [edi+ 0x2C]
		push eax
		call PostTick
		pop eax //restore return value
		ret
	}
}

void TickInjection() {
	//a general callback for conversations with the pc. eax thiscall, no stack parameters.
	//eax is a pointer to a struct that transmorphs as described in ConversationStruct.h
	//AA2Play v12 FP v1.4.0a.exe+4A237 - 8B 47 2C              - mov eax,[edi+2C]
	//AA2Play v12 FP v1.4.0a.exe + 4A23A - E8 416C0000 - call "AA2Play v12 FP v1.4.0a.exe" + 50E80 {->AA2Play v12 FP v1.4.0a.exe + 50E80 }
	DWORD address = General::GameBase + 0x4A23A;
	DWORD redirectAddress = (DWORD)(&TickRedirect);
	Hook((BYTE*)address,
	{ 0xE8, 0x41, 0x6C, 0x00, 0x00 },						//expected values
	{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		(DWORD*)(&OriginalTickFunction));
}

void __declspec(naked) NpcAnswerRedirect() {
	__asm {
		//orignal code
		mov ecx, [edx + 04]
		mov[ecx + ebp + 0x30], eax
		//give a chance to change it
		pushad
		push ebp
		call NpcAnswer
		popad
		ret
	}
}

void NpcAnswerInjection() {
	//this function returns the reaction (positive = 1, negative = 0). ebp is the struct.
	/*AA2Play v12 FP v1.4.0a.exe + 5B3D0 - 8B 48 20 - mov ecx, [eax + 20]
	AA2Play v12 FP v1.4.0a.exe + 5B3D3 - 52 - push edx
	AA2Play v12 FP v1.4.0a.exe + 5B3D4 - 51 - push ecx
	AA2Play v12 FP v1.4.0a.exe + 5B3D5 - 8B 48 50 - mov ecx, [eax + 50]
	AA2Play v12 FP v1.4.0a.exe + 5B3D8 - 8B 40 54 - mov eax, [eax + 54]
	AA2Play v12 FP v1.4.0a.exe + 5B3DB - E8 20C81200 - call "AA2Play v12 FP v1.4.0a.exe" + 187C00{ ->AA2Play v12 FP v1.4.0a.exe + 187C00 }
	AA2Play v12 FP v1.4.0a.exe + 5B3E0 - 8B 55 00 - mov edx, [ebp + 00]
	AA2Play v12 FP v1.4.0a.exe+5B3E0 - 8B 55 00              - mov edx,[ebp+00]
	AA2Play v12 FP v1.4.0a.exe+5B3E3 - 8B 4A 04              - mov ecx,[edx+04]
	AA2Play v12 FP v1.4.0a.exe+5B3E6 - 89 44 29 30           - mov [ecx+ebp+30],eax
*/

	DWORD address = General::GameBase + 0x5B3E3;
	DWORD redirectAddress = (DWORD)(&NpcAnswerRedirect);
	Hook((BYTE*)address,
		{ 0x8B, 0x4A, 0x04, 
		0x89, 0x44, 0x29, 0x30 },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90, 0x90 },	//redirect to our function
		NULL);
}

void __declspec(naked) PcAnswerRedirect() {
	__asm {
		pushad
		push ebp
		call PcAnswer
		popad
		//original code
		mov eax, [ebp+0]
		mov ecx, [eax+4]
		ret
	}
}

void PcAnswerInjection() {
	//ebp is the conversation struct. eax+4C is taken out and after the cmp translated to the regular response.
	/*AA2Play v12 FP v1.4.0a.exe + 58DF8 - 8B 45 00 - mov eax, [ebp + 00]
	AA2Play v12 FP v1.4.0a.exe + 58DFB - 8B 48 04 - mov ecx, [eax + 04]
	AA2Play v12 FP v1.4.0a.exe + 58DFE - 8D 04 29 - lea eax, [ecx + ebp]
	AA2Play v12 FP v1.4.0a.exe + 58E01 - 8B 48 4C - mov ecx, [eax + 4C]
	AA2Play v12 FP v1.4.0a.exe + 58E04 - 83 F9 01 - cmp ecx, 01 { 1 }*/
	DWORD address = General::GameBase + 0x58DF8;
	DWORD redirectAddress = (DWORD)(&PcAnswerRedirect);
	Hook((BYTE*)address,
		{ 0x8B, 0x45, 0x00, 
		0x8B, 0x48, 0x04 },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function
		NULL);
}

/*Called for every conversation tick:
AA2Play v12 FP v1.4.0a.exe+4A237 - 8B 47 2C              - mov eax,[edi+2C]
AA2Play v12 FP v1.4.0a.exe+4A23A - E8 416C0000           - call "AA2Play v12 FP v1.4.0a.exe"+50E80 { ->AA2Play v12 FP v1.4.0a.exe+50E80 }
*/


}
}
