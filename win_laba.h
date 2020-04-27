#pragma once
#define BUFFER_SIZE 512
#define MESSAGE_SIZE 100

char buffer[BUFFER_SIZE];
char message[MESSAGE_SIZE];
int size = 0;
HINSTANCE library;

struct Array										//��������� � ������ �����
{
	char fileName[100];
};

struct Data
{
	struct Array *arr;								//����� ������
	HANDLE hThreads[2];								//������
	HANDLE hSemRead;								//������� ������
	HANDLE hSemWrite;								//������� ������
};

typedef void(*ReadDataFromFile)(HANDLE, char*);
typedef void(*WriteDataInFile)(HANDLE, char*);

//����� ������
void Search(struct Data *data)
{
	WIN32_FIND_DATAA wfd;

	HANDLE hFind = FindFirstFileA("D:/Files/*.txt", 					//��� �����
		&wfd);															//���������� � ��������
	//setlocale(LC_ALL, "");

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!strcmp((char*)(wfd.cFileName), "Allfiles.txt"))				//���� �� ����� ����� ��� ������,
			{
				continue;											//�� �������� ���� ������
			}
			if (size == 0)
			{
				data->arr = (struct Array*) malloc(1 * sizeof(struct Array));		//��������� ������ ��� ������ ��� �����
			}
			else
			{														//��������� ������ ��� ��������� ���� ������
				data->arr = (struct Array*) realloc(data->arr, (size + 1) * sizeof(struct Array));
			}
			strcpy_s(data->arr[size].fileName, "D:/Files/");
			strcat_s(data->arr[size].fileName, wfd.cFileName);
			size++;
		} while (FindNextFileA(hFind, &wfd) != 0);
		size--;
		FindClose(hFind);											//��������� ���������� ������ �����
	}
	else
	{
		printf("Can't open directory.\n");
		exit(0);
	}
}

//������� ������-��������
unsigned int ThreadReader(void* fdata)
{
	struct Data *data = (struct Data *)fdata;

	ReadDataFromFile readDataFromFile = (ReadDataFromFile)GetProcAddress(
		library,											//���������� dll ����������
		"ReadDataFromFile");
								//��� ������ �������
	if (readDataFromFile == nullptr) {
		printf("%d\n", GetLastError());
	}

	while (1)
	{
		WaitForSingleObject(data->hSemRead, INFINITE);	//���� ���� ����� ��������� ������, ������� ��������� �����������
		if (size == -1)
		{
			break;
		}
		
		HANDLE hFile = CreateFileA(
			data->arr[size].fileName,						//��� �����
			GENERIC_READ,									//��� �������
			0,												//��� ����������� �������
			NULL,											//�� ����� ������������� ����������
			OPEN_EXISTING,									//������� ������������
			FILE_FLAG_OVERLAPPED,							//������������ ������
			NULL);											//��� �����-�������
		if (hFile == INVALID_HANDLE_VALUE)
		{
			printf("ERROR READ\n");
			printf("%d\n", GetLastError());
			exit(0);
		}
		(*readDataFromFile)(hFile, buffer);				//������ �� �����
		printf("Reader: I read data from '%s'\n", data->arr[size].fileName);
		CloseHandle(hFile);
		ReleaseSemaphore(data->hSemWrite, 1, NULL);		//��������� ������
	}

	_endthreadex(0);									//��������� �����
	return 0;
}

//������� ������-��������
unsigned int ThreadWriter(void* fdata)
{
	struct Data *data = (struct Data *)fdata;
	HANDLE hFile = CreateFileA("D:/Files/Allfiles.txt", GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS,											//��������� ������������, ���� �� ����������, ����� �������
		FILE_FLAG_OVERLAPPED, NULL);

	//�������� ��������� �� ������� �� dll ����������
	WriteDataInFile writeDataInFile = (WriteDataInFile)GetProcAddress(library, "WriteDataInFile");

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("ERROR WRITE\n");
		exit(0);
	}

	do
	{
		WaitForSingleObject(data->hSemWrite, INFINITE);		//���� ���������� �� ������
		(*writeDataInFile)(hFile, buffer);					//����� � ����
		printf("Writer: I wrote data from '%s'\n", data->arr[size].fileName);
		size--;
		ReleaseSemaphore(data->hSemRead, 1, NULL);			//��������� ������
	} while (size != -1);

	CloseHandle(hFile);										//��������� ����
	_endthreadex(0);										//��������� �����
	return 0;
}

//������� �������� ������� �������������
void CreateEventForThreads(struct Data *data)
{
	data->hSemRead = CreateSemaphoreA(NULL,					//��������
		1,														//��������� ���������
		1,														//������������ ���������� ���������
		"SemRead");												//��� ��������
	data->hSemWrite = CreateSemaphoreA(NULL, 0, 1, "SemWrite");
}

//������� �������� �������
void CreateThreads(struct Data *data)
{
	//�����-��������
	data->hThreads[1] = (HANDLE)CreateThread(
		NULL,												//���������� ������
		0,													//��������� ������ ����� ��� � ������������ ���������
		(LPTHREAD_START_ROUTINE)&ThreadReader,				//������� ������
		(void*)data,										//��������, ������������ ������
		0,													//��������� ����������
		NULL);												//���������, ������� ��������� ������������� ������
		//�����-��������
	data->hThreads[0] = (HANDLE)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ThreadWriter, (void*)data, 0, NULL);
}

void *OpenLib()
{
	library = LoadLibraryA("fun.dll");
	return library;
}

void WaitThreads(struct Data *data)
{
	WaitForMultipleObjects(2, data->hThreads, TRUE, INFINITE);
}

void CloseApp(struct Data *data)
{
	CloseHandle(data->hThreads[0]);
	CloseHandle(data->hThreads[1]);
	CloseHandle(data->hSemWrite);
	CloseHandle(data->hSemRead);
	FreeLibrary(library);								//�������� dll ����������					
}