#include "MyPeLoader.h"

using namespace std;
typedef struct {
	WORD offset : 12;
	WORD type : 4;
}IMAGE_RELOC, *PIMAGE_RELOC;
int main()
{
	LPWSTR* szArgsList;
	int nArgs;

	szArgsList = CommandLineToArgvW(GetCommandLineW(), &nArgs);

	if (nArgs != 2) {
		ShowGreetings();
		return 0;
	}

	LPBYTE fileImage = ReadPe(szArgsList[1]);
	if (fileImage == NULL) {
		SetLastError(0x1001);
		exit(1);
	}

	IMAGE_DOS_HEADER* dosHeader = PIMAGE_DOS_HEADER(fileImage);
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
		cout << "DOS signature mismatch" << endl;
		exit(1001);
	}

	cout << "ELFANEW " << dosHeader->e_lfanew << endl;
	IMAGE_NT_HEADERS* ntHeaders = PIMAGE_NT_HEADERS((DWORD)fileImage + dosHeader->e_lfanew);
	cout << "EP " << ntHeaders->OptionalHeader.AddressOfEntryPoint << endl;
	IMAGE_FILE_HEADER imageFileHeader = (IMAGE_FILE_HEADER)ntHeaders->FileHeader;
	IMAGE_OPTIONAL_HEADER imageOptionalHeader = (IMAGE_OPTIONAL_HEADER)ntHeaders->OptionalHeader;
	DWORD imageSignature = ntHeaders->Signature;

	if (imageSignature != IMAGE_NT_SIGNATURE) {
		cout << "NT signature mismatch" << endl;
		exit(1002);
	}

	if (DEBUG) {
		cout << "File header:" << endl;
		wcout << "\tMachine: " << std::hex << imageFileHeader.Machine << endl;
		wcout << "\tCharacteristics: " << std::hex << imageFileHeader.Characteristics << endl;
		wcout << "\tTimedatestamp: " << imageFileHeader.TimeDateStamp << endl;

		cout << "Optional header:" << endl;
		wcout << "\tImageBase: " << std::hex << imageOptionalHeader.ImageBase << endl;
		wcout << "\tSizeOfImage: " << std::hex << imageOptionalHeader.SizeOfImage << endl;
		wcout << "\tSubsystem: " << std::hex << imageOptionalHeader.Subsystem << endl;
	}

	LPBYTE peImage = (LPBYTE)VirtualAlloc(NULL, imageOptionalHeader.SizeOfImage, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	MoveMemory(peImage, fileImage, imageOptionalHeader.SizeOfHeaders);

	IMAGE_SECTION_HEADER* imageSectionHeader = PIMAGE_SECTION_HEADER((BYTE*)& ntHeaders->OptionalHeader + imageFileHeader.SizeOfOptionalHeader);

	if (DEBUG) {
		cout << "Sections:" << endl;
	}

	for (int i = 0; i < imageFileHeader.NumberOfSections; i++) {
		if (DEBUG) {
			cout << "\tName: " << imageSectionHeader->Name << endl;
			wcout << "\tPointer to raw data: " << imageSectionHeader->PointerToRawData << endl;
			wcout << "\tSize of raw data: " << imageSectionHeader->SizeOfRawData << endl;
			wcout << "\tVirtual address: " << imageSectionHeader->VirtualAddress << endl;
			wcout << "\tCharacteristics: " << imageSectionHeader->Characteristics << endl;
			cout << endl;
		}

		MoveMemory((LPVOID)(peImage + imageSectionHeader->VirtualAddress), (LPVOID)(fileImage + imageSectionHeader->PointerToRawData), imageSectionHeader->SizeOfRawData);
		VirtualAlloc((LPVOID)(peImage + imageSectionHeader->VirtualAddress), imageSectionHeader->SizeOfRawData, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		imageSectionHeader++;
	}


	IMAGE_DATA_DIRECTORY * imageDataDirectory = PIMAGE_DATA_DIRECTORY((BYTE*)& ntHeaders->OptionalHeader.DataDirectory);
	DWORD importTableVAddr, importTableVSize, importTableRawAddr;


	importTableVAddr = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

	PIMAGE_IMPORT_DESCRIPTOR pImageImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(peImage + importTableVAddr);

	if (DEBUG)
		cout << "Import descriptor" << endl;
	while (pImageImportDescriptor->Name) {
		if (DEBUG) {
			wcout << "\tName: " << pImageImportDescriptor->Characteristics << endl;
			wcout << "\tTimeDateStamp: " << pImageImportDescriptor->TimeDateStamp << endl;
			wcout << "\tFirst thunk: " << pImageImportDescriptor->FirstThunk << endl;
		}
		LPCSTR libName = (LPCSTR)((DWORD)peImage +  pImageImportDescriptor->Name);
		if (DEBUG) {
			wcout << "\tVisible name: " << libName << endl;
			cout << endl;
		}

		HINSTANCE hInstLib = LoadLibraryExA(libName, NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_USER_DIRS);
		if (DEBUG && (hInstLib == INVALID_HANDLE_VALUE || hInstLib == NULL)) {
			DWORD error = GetLastError();
			wcout << "Error while loading library " << libName << " with error " << error << endl;
			cout << "Sorry((" << endl;
			cout << endl;
		}

		PIMAGE_THUNK_DATA pThunkData, pThunkDataOrig;
		PIMAGE_IMPORT_BY_NAME imageImportByName;
		pThunkData = PIMAGE_THUNK_DATA((DWORD)peImage + pImageImportDescriptor->FirstThunk);
		if (pImageImportDescriptor->Characteristics != 0)
			pThunkDataOrig = (PIMAGE_THUNK_DATA)((DWORD)peImage + pImageImportDescriptor->Characteristics);
		else
			pThunkDataOrig = pThunkData;

		while (pThunkDataOrig->u1.AddressOfData) {
			if (IMAGE_SNAP_BY_ORDINAL(pThunkDataOrig->u1.Ordinal)) {
				LPCSTR Ordinal = (LPCSTR)IMAGE_ORDINAL(pThunkDataOrig->u1.Ordinal);
				pThunkData->u1.Function = (DWORD)GetProcAddress(hInstLib, Ordinal);
				if (DEBUG) {
					wcout << "\tImport function by ordinal " << (int)Ordinal << endl;
					wcout << endl;
				}
			}
			else {
				imageImportByName = PIMAGE_IMPORT_BY_NAME((DWORD)peImage + pThunkDataOrig->u1.AddressOfData);
				pThunkData->u1.Function = (DWORD)GetProcAddress(hInstLib, imageImportByName->Name);
				if (DEBUG) {
					wcout << "\tImport function by name " << imageImportByName->Name << endl;
					wcout << endl;
				}
			}
			
			pThunkData++;
			pThunkDataOrig++;
		}
			
		pImageImportDescriptor++;
	}
	
	DWORD relocTableVAddr = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
	DWORD relocTableRAW = RVA2RAW(peImage, relocTableVAddr);

	DWORD relocTableSize = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
	DWORD diff = (DWORD)peImage - ntHeaders->OptionalHeader.ImageBase;
		PIMAGE_BASE_RELOCATION pImageBaseRelocation = PIMAGE_BASE_RELOCATION(peImage + relocTableVAddr);
		if (relocTableSize) {
			DWORD x;
			PIMAGE_RELOC pReloc;
			while (pImageBaseRelocation->SizeOfBlock && pImageBaseRelocation->VirtualAddress) {
				x = (DWORD)peImage + pImageBaseRelocation->VirtualAddress;
				DWORD countOfRelocs = (pImageBaseRelocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / 2;
				wcout << "Relocs: " << pImageBaseRelocation->VirtualAddress << " " << pImageBaseRelocation->SizeOfBlock << " " << countOfRelocs << endl;
				pReloc = (PIMAGE_RELOC)(((DWORD)pImageBaseRelocation) + sizeof(IMAGE_BASE_RELOCATION));
				for (int i = 0; i < countOfRelocs; i++) {
					switch (pReloc->type) {
					case IMAGE_REL_BASED_DIR64:
						*((UINT_PTR*)(x + pReloc->offset)) += (DWORD)peImage - ntHeaders->OptionalHeader.ImageBase;
						break;
					case IMAGE_REL_BASED_HIGHLOW:
						*((DWORD*)(x + pReloc->offset)) += (DWORD)(peImage - ntHeaders->OptionalHeader.ImageBase);
						break;
					case IMAGE_REL_BASED_HIGH:
						*((WORD*)(x + pReloc->offset)) += HIWORD(peImage - ntHeaders->OptionalHeader.ImageBase);
						break;
					case IMAGE_REL_BASED_LOW:
						*((WORD*)(x + pReloc->offset)) += LOWORD(peImage - ntHeaders->OptionalHeader.ImageBase);
						break;
					default:
						cout << "Unkown reloc type " << endl;
						break;
					}
					pReloc += 1;
				}
				pImageBaseRelocation = (PIMAGE_BASE_RELOCATION)((DWORD)pImageBaseRelocation + pImageBaseRelocation->SizeOfBlock);
			}
		}

		_PPEB peb = (_PPEB)__readfsdword(0x30);
		peb->lpImageBaseAddress = (LPVOID)peImage;

		DWORD entryPoint = (DWORD)peImage + ntHeaders->OptionalHeader.AddressOfEntryPoint;
	
	__asm {
		mov eax, entryPoint
		call eax
		int 3
	}
	
	free(fileImage);
	LocalFree(szArgsList);
	exit(0);
}

DWORD RVA2RAW(LPVOID fileImage, DWORD dwRva) {
	DWORD dwRawRVAddr(0);
	IMAGE_DOS_HEADER* dosHeader = PIMAGE_DOS_HEADER(fileImage);
	IMAGE_NT_HEADERS* ntHeaders = PIMAGE_NT_HEADERS((DWORD)fileImage + dosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSections = IMAGE_FIRST_SECTION(ntHeaders);
	if (!pSections)
		return dwRawRVAddr;

	while (pSections->VirtualAddress != 0) {
		if (dwRva >= pSections->VirtualAddress && dwRva < pSections->VirtualAddress + pSections->SizeOfRawData) {
			dwRawRVAddr = (dwRva - pSections->VirtualAddress) + pSections->PointerToRawData;
			break;
		}
		pSections++;
	}
	return dwRawRVAddr;
}

VOID ShowGreetings() {
	cout << "PeLoader v0.0.1" << endl;
	cout << "Usage: MyPeLoader.exe <path/to/executable>" << endl;
}

LPBYTE ReadPe(LPWSTR PeName) {
	HANDLE hFile;
	DWORD fileSize;
	DWORD bytesRead;
	LPBYTE fileBuffer;

	hFile = CreateFile(PeName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		wcout << "Error while opening file " << PeName << "with error " << GetLastError() << endl;
		return NULL;
	}

	fileSize = GetFileSize(hFile, NULL);
	fileBuffer = (LPBYTE)malloc(fileSize);

	if (!ReadFile(hFile, fileBuffer, fileSize - 1, &bytesRead, NULL)) {
		cout << "Error while reading file" << endl;
		return NULL;
	}

	return fileBuffer;
}
