;
; Gens: 2x interpolated scanline renderer. (x86 ASM version)
;
; Copyright (c) 1999-2002 by Stéphane Dallongeville
; Copyright (c) 2003-2004 by Stéphane Akhoun
;
; This program is free software; you can redistribute it and/or modify it
; under the terms of the GNU General Public License as published by the
; Free Software Foundation; either version 2 of the License, or (at your
; option) any later version.
;
; This program is distributed in the hope that it will be useful, but
; WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License along
; with this program; if not, write to the Free Software Foundation, Inc.,
; 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
;


%include "nasmhead.inc"


section .data align=64


	extern MD_Screen
	extern TAB336
	extern Have_MMX
	extern bpp

	MASK_DIV2_15:	dd 0x3DEF3DEF, 0x3DEF3DEF
	MASK_DIV2_16:	dd 0x7BEF7BEF, 0x7BEF7BEF


section .text align=64


	ALIGN64
	
	;*********************************************************************************
	; void Blit2x_Scanline_Int_16_asm(unsigned char *screen, int pitch, int x, int y, int offset)
	DECL Blit2x_Scanline_Int_16_asm

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Nombre de pix par ligne
		mov ebx, [esp + 28]				; ebx = pitch de la surface screen
		lea ecx, [ecx * 4]				; ecx = Nb bytes par ligne screen
		add ebx, ebx					; ebx = pitch * 2
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		sub ebx, ecx					; ebx = Compl�ment offset pour ligne suivante
		shr ecx, 2						; on transfert 4 bytes screen � chaque boucle
		mov edi, [esp + 24]				; edi = destination
		mov [esp + 32], ecx				; on stocke cette nouvelle valeur pour X
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X
;				mov eax, [esi]
;				add esi, byte 2
;				shr eax, 1
;				mov edx, eax
;				and eax, 0x03E0780F
;				and edx, 0x780F03E0
;				rol eax, 16
;				add edi, byte 4
;				add eax, edx
;				mov dx, ax
;				shr eax, 16
;				or dx, ax
;				dec ecx
;				mov ax, [esi]
;				mov [edi + 0 - 4], dx
;				mov [edi + 2 - 4], ax
;				jnz short .Loop_X

				mov ax, [esi]
				mov dx, [esi + 2]
				shr ax, 1
				add esi, byte 2
				shr dx, 1
				and ax, 0x7BCF
				and dx, 0x7BCF
				add edi, byte 4
				add ax, dx
				dec ecx
				mov dx, [esi]
				mov [edi + 0 - 4], ax
				mov [edi + 2 - 4], dx
				jnz short .Loop_X

			add esi, [esp + 40]			; on augmente la source pour pointer sur la prochaine ligne
			add edi, ebx				; on augmente la destination avec le debordement du pitch
			dec dword [esp + 36]		; on continue tant qu'il reste des lignes
			mov ecx, [esp + 32]			; ecx = Nombre de pixels / 4 dans une ligne
			jnz short .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret


	ALIGN64
	
	;*********************************************************************************
	; void Blit2x_Scanline_Int_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset)
	DECL Blit2x_Scanline_Int_16_asm_MMX

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Nombre de pix par ligne
		mov ebx, [esp + 28]				; ebx = pitch de la surface screen
		lea ecx, [ecx * 4]				; ecx = Nb bytes par ligne screen
		add ebx, ebx					; ebx = pitch * 2
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		sub ebx, ecx					; ebx = Compl�ment offset pour ligne suivante
		shr ecx, 4						; on transfert 16 bytes screen � chaque boucle
		mov edi, [esp + 24]				; edi = destination
		mov [esp + 32], ecx				; on stocke cette nouvelle valeur pour X

		movq mm7, [MASK_DIV2_15]
		; Check if this is 15-bit color mode.
		cmp byte [bpp], 15			; set good mask for current video mode
		je short .Loop_Y

		; 16-bit color mode.
		movq mm7, [MASK_DIV2_16]
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X
				movq mm0, [esi]
				add edi, byte 16
				movq mm2, mm0
				movq mm1, [esi + 2]
				psrlw mm0, 1
				psrlw mm1, 1
				pand mm0, mm7
				pand mm1, mm7
				movq mm3, mm2
				paddw mm0, mm1
				add esi, byte 8
				punpcklwd mm2, mm0
				punpckhwd mm3, mm0
				movq [edi + 0 - 16], mm2
				dec ecx
				movq [edi + 8 - 16], mm3
				jnz short .Loop_X

			add esi, [esp + 40]			; on augmente la source pour pointer sur la prochaine ligne
			add edi, ebx				; on augmente la destination avec le debordement du pitch
			dec dword [esp + 36]		; on continue tant qu'il reste des lignes
			mov ecx, [esp + 32]			; ecx = Nombre de pixels / 4 dans une ligne
			jnz short .Loop_Y

		emms
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret
