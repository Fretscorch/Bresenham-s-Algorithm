#include <iostream>
#include <fstream>
#include "windows.h"

using namespace std;

#define IMAGE_SIZE 256

void DrawLine(int x1, int y1, int x2, int y2, char bits[IMAGE_SIZE][IMAGE_SIZE]) {
	int x, y, xDistance, yDistance, dx1, dy1, px, py, xe, ye;

	__asm {
		mov eax, x2
		mov ebx, y2
		sub eax, x1
		sub ebx, y1
		mov xDistance, eax
		mov yDistance, ebx
		mov eax, x1
		mov ebx, y1
		mov ecx, x2
		mov edx, y2
		mov esi, xDistance
		mov edi, yDistance
		cmp xDistance, 0
		jl TurnDXAbsolute
		jge L1
		TurnDXAbsolute : ; abs(xDistance)
		neg esi
		L1 :
		mov dx1, esi
			cmp yDistance, 0
			jl TurnDYAbsolute
			jge L2
			TurnDYAbsolute : ; abs(yDistance)
			neg edi
			L2 :
		mov dy1, edi
			cmp edi, esi; dy1 <= dx1
			jle CheckXDistance
			jg CheckYDistance
			CheckXDistance :
		cmp xDistance, 0
			jge DXGreaterThanZero; xDistance >= 0
			jl DXLessThanZero; xDistance < 0
			DXGreaterThanZero :
		mov x, eax; x = x1
			mov y, ebx; y = y1
			mov xe, ecx; xe = x2
			jmp L3
			DXLessThanZero :
		mov x, ecx; x = x2
			mov y, edx; y = y2
			mov xe, eax; xe = x1
			jmp L3
			CheckYDistance :
		cmp yDistance, 0
			jge DYGreaterThanZero; yDistance >= 0
			jl DYLessThanZero; yDistance < 0
			DYGreaterThanZero :
		mov x, eax; x = x1
			mov y, ebx; y = y1
			mov ye, edx; ye = y2
			jmp L3
			DYLessThanZero :
		mov x, ecx; x = x2
			mov y, edx; y = y2
			mov ye, ebx; ye = y1
			jmp L3
			L3 :
		mov edi, dy1
			imul edi, 2
			sub edi, esi
			mov px, edi
			mov edi, dy1
			mov esi, dx1
			imul esi, 2
			sub esi, edi
			mov py, esi
			mov eax, y
			add eax, 7 //Pixel drawing
			imul eax, 256
			add eax, x
			add eax, 16
			lea ebx, [bits + eax]
			mov ah, 255
			sub ah, [ebx]
			mov[ebx], ah;
			cmp edi, esi; dy1 <= dx1
				jle DrawLineLR
				jg DrawLineRL
				DrawLineLR :
			mov esi, x
				add esi, 1
				mov x, esi
				cmp px, 0
				jl XErrorAdjust
				jge CompareXDistanceLessThanZero
				XErrorAdjust :
			mov edi, dy1
				imul edi, 2
				add edi, px
				mov px, edi
				jmp L4
				CompareXDistanceLessThanZero :
			cmp xDistance, 0
				jl CompareYDistanceLessThanZero
				jge CompareXDistanceGreaterThanZero
				CompareYDistanceLessThanZero :
			cmp yDistance, 0
				jl IncrementY
				jge CompareXDistanceGreaterThanZero
				CompareXDistanceGreaterThanZero :
			cmp xDistance, 0
				jg CompareYDistanceGreaterThanZero
				jle DecrementY
				CompareYDistanceGreaterThanZero :
			cmp yDistance, 0
				jg IncrementY
				jle DecrementY
				IncrementY :
			add y, 1
				mov edi, dy1
				sub edi, dx1
				imul edi, 2
				add edi, px
				mov px, edi
				jmp L4
				DecrementY :
			sub y, 1
				mov edi, dy1
				sub edi, dx1
				imul edi, 2
				add edi, px
				mov px, edi
				jmp L4
				L4 :
			mov eax, y
				add eax, 7 //Pixel drawing
				imul eax, 256
				add eax, x
				add eax, 16
				lea ebx, [bits + eax]
				mov ah, 255
				sub ah, [ebx]
				mov[ebx], ah;
				cmp esi, xe
					jl DrawLineLR
					jge Done
					DrawLineRL :
				mov esi, y
					add esi, 1
					mov y, esi
					cmp py, 0
					jl YErrorAdjust
					jge CompareXDistanceLessThanZeroLR
					YErrorAdjust :
				mov edi, dx1
					imul edi, 2
					add edi, py
					mov py, edi
					jmp L5
					CompareXDistanceLessThanZeroLR :
				cmp xDistance, 0
					jl CompareYDistanceLessThanZeroLR
					jge CompareXDistanceGreaterThanZeroLR
					CompareYDistanceLessThanZeroLR :
				cmp yDistance, 0
					jl IncrementX
					jge CompareXDistanceGreaterThanZeroLR
					CompareXDistanceGreaterThanZeroLR :
				cmp xDistance, 0
					jg CompareYDistanceGreaterThanZeroLR
					jle DecrementX
					CompareYDistanceGreaterThanZeroLR :
				cmp yDistance, 0
					jg IncrementX
					jle DecrementX
					IncrementX :
				add x, 1
					mov edi, dx1
					sub edi, dy1
					imul edi, 2
					add edi, py
					mov py, edi
					jmp L5
					DecrementX :
				sub x, 1
					mov edi, dx1
					sub edi, dy1
					imul edi, 2
					add edi, py
					mov py, edi
					jmp L5
					L5 :
				mov eax, y
					add eax, 7 //Pixel drawing
					imul eax, 256
					add eax, x
					add eax, 16
					lea ebx, [bits + eax]
					mov ah, 255
					sub ah, [ebx]
					mov[ebx], ah;
					cmp esi, ye
						jl DrawLineRL
						jge Done
						Done :
	}
}

int main(int argc, char* argv[]) {
	int maxSize = IMAGE_SIZE;
	int x1, x2, y1, y2;
	bool valid = true;

	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	char colorTable[1024];
	char bits[IMAGE_SIZE][IMAGE_SIZE] = { 0 };
	int i, j;
	ofstream bmpOut("foo.bmp", ios::out + ios::binary);

	if (!bmpOut) {
		cout << "...could not open file, ending.";
		return -1;
	}

	bmfh.bfType = 0x4d42;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof(bmfh) + sizeof(bmih) + sizeof(colorTable);
	bmfh.bfSize = bmfh.bfOffBits + sizeof(bits);
	bmih.biSize = 40;
	bmih.biWidth = IMAGE_SIZE;
	bmih.biHeight = IMAGE_SIZE;
	bmih.biPlanes = 1;
	bmih.biBitCount = 8;
	bmih.biCompression = 0;
	bmih.biSizeImage = IMAGE_SIZE * IMAGE_SIZE;
	bmih.biXPelsPerMeter = 2835;
	bmih.biYPelsPerMeter = 2835;
	bmih.biClrUsed = 256;
	bmih.biClrImportant = 0;

	for (i = 0; i < 256; i++) {
		j = i * 4;
		colorTable[j] = colorTable[j + 1] = colorTable[j + 2] = colorTable[j + 3] = i;
	}

	for (i = 0; i < IMAGE_SIZE; i++) {
		for (j = 0; j < IMAGE_SIZE; j++) {
			bits[i][j] = j;
		}
	}

	cout << "Pierce College CS230 Spring 2025 Lab Assignment 2 - Hathaway, Connor\n";
	cout << "Enter two pairs of point coordinates in the range of 0 to " << IMAGE_SIZE - 1 << ".\n";
	cin >> x1 >> y1 >> x2 >> y2;

	if (x1 < 0 || x1 > 255) {
		cout << "Value " << x1 << " out of range, ending.\n";
		valid = false;
	}
	if (y1 < 0 || y1 > 255) {
		cout << "Value " << y1 << " out of range, ending.\n";
		valid = false;
	}
	if (x2 < 0 || x2 > 255) {
		cout << "Value " << x2 << " out of range, ending.\n";
		valid = false;
	}
	if (y2 < 0 || y2 > 255) {
		cout << "Value " << y2 << " out of range, ending.";
		valid = false;
	}

	if (valid == true) {
		DrawLine(x1, y1, x2, y2, bits);

		char* workPtr;
		workPtr = (char*)&bmfh;
		bmpOut.write(workPtr, 14);
		workPtr = (char*)&bmih;
		bmpOut.write(workPtr, 40);
		workPtr = &colorTable[0];
		bmpOut.write(workPtr, sizeof(colorTable));
		workPtr = &bits[0][0];
		bmpOut.write(workPtr, IMAGE_SIZE * IMAGE_SIZE);
		bmpOut.close();

		system("mspaint foo.bmp");
	}
	return 0;
}