#pragma once
#define BUFFER_SIZE 512
#define MESSAGE_SIZE 100

char buffer[BUFFER_SIZE];
char message[MESSAGE_SIZE];
int size = 0;
HINSTANCE library;

struct Array										//Струкутра с именем файла
{
	char fileName[100];
};

struct Data
{
	struct Array *arr;								//Имена файлов
	HANDLE hThreads[2];								//Потоки
	HANDLE hSemRead;								//Семафор чтения
	HANDLE hSemWrite;								//Семафор записи
};

typedef void(*ReadDataFromFile)(HANDLE, char*);
typedef void(*WriteDataInFile)(HANDLE, char*);

//Поиск файлов
void Search(struct Data *data)
{
	WIN32_FIND_DATAA wfd;

	HANDLE hFind = FindFirstFileA("D:/Files/*.txt", 					//Имя файла
		&wfd);															//Информация о каталоге
	//setlocale(LC_ALL, "");

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!strcmp((char*)(wfd.cFileName), "Allfiles.txt"))				//Если мы нашли файла для записи,
			{
				continue;											//то начинаем цикл заново
			}
			if (size == 0)
			{
				data->arr = (struct Array*) malloc(1 * sizeof(struct Array));		//Выделение памяти под первое имя файла
			}
			else
			{														//Выделение памяти под несколько имен файлов
				data->arr = (struct Array*) realloc(data->arr, (size + 1) * sizeof(struct Array));
			}
			strcpy_s(data->arr[size].fileName, "D:/Files/");
			strcat_s(data->arr[size].fileName, wfd.cFileName);
			size++;
		} while (FindNextFileA(hFind, &wfd) != 0);
		size--;
		FindClose(hFind);											//Закрываем дескриптор поиска файла
	}
	else
	{
		printf("Can't open directory.\n");
		exit(0);
	}
}

//Фукнция потока-читателя
unsigned int ThreadReader(void* fdata)
{
	struct Data *data = (struct Data *)fdata;

	ReadDataFromFile readDataFromFile = (ReadDataFromFile)GetProcAddress(
		library,											//Дескриптор dll библиотеки
		"ReadDataFromFile");
								//Имя нужной функции
	if (readDataFromFile == nullptr) {
		printf("%d\n", GetLastError());
	}

	while (1)
	{
		WaitForSingleObject(data->hSemRead, INFINITE);	//Ждем пока будет разрешено чтение, семафор автоматом уменьшается
		if (size == -1)
		{
			break;
		}
		
		HANDLE hFile = CreateFileA(
			data->arr[size].fileName,						//Имя файла
			GENERIC_READ,									//Тип доступа
			0,												//Нет совместного доступа
			NULL,											//Не может наследоваться дескриптор
			OPEN_EXISTING,									//Открыть существующий
			FILE_FLAG_OVERLAPPED,							//Ассинхронная работа
			NULL);											//Нет файла-шаблона
		if (hFile == INVALID_HANDLE_VALUE)
		{
			printf("ERROR READ\n");
			printf("%d\n", GetLastError());
			exit(0);
		}
		(*readDataFromFile)(hFile, buffer);				//Чтение из файла
		printf("Reader: I read data from '%s'\n", data->arr[size].fileName);
		CloseHandle(hFile);
		ReleaseSemaphore(data->hSemWrite, 1, NULL);		//Разрешаем запись
	}

	_endthreadex(0);									//Завершаем поток
	return 0;
}

//Фукнция потока-писателя
unsigned int ThreadWriter(void* fdata)
{
	struct Data *data = (struct Data *)fdata;
	HANDLE hFile = CreateFileA("D:/Files/Allfiles.txt", GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS,											//Открывает существующий, если он существует, иначе создает
		FILE_FLAG_OVERLAPPED, NULL);

	//Плучение указателя на функцию из dll библиотеки
	WriteDataInFile writeDataInFile = (WriteDataInFile)GetProcAddress(library, "WriteDataInFile");

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("ERROR WRITE\n");
		exit(0);
	}

	do
	{
		WaitForSingleObject(data->hSemWrite, INFINITE);		//Ждем разрешения на запись
		(*writeDataInFile)(hFile, buffer);					//Пишем в файл
		printf("Writer: I wrote data from '%s'\n", data->arr[size].fileName);
		size--;
		ReleaseSemaphore(data->hSemRead, 1, NULL);			//Разрешаем чтение
	} while (size != -1);

	CloseHandle(hFile);										//Закрываем файл
	_endthreadex(0);										//Завершаем поток
	return 0;
}

//Функция создания событий синхронизации
void CreateEventForThreads(struct Data *data)
{
	data->hSemRead = CreateSemaphoreA(NULL,					//Атрибуты
		1,														//Начальное состояние
		1,														//Максимальное количество обращений
		"SemRead");												//Имя семафора
	data->hSemWrite = CreateSemaphoreA(NULL, 0, 1, "SemWrite");
}

//Функция создания потоков
void CreateThreads(struct Data *data)
{
	//Поток-читатель
	data->hThreads[1] = (HANDLE)CreateThread(
		NULL,												//Дескриптор защиты
		0,													//Начальный размер стека как у исполнаяемой программы
		(LPTHREAD_START_ROUTINE)&ThreadReader,				//Фукнция потока
		(void*)data,										//Параметр, передаваемый потоку
		0,													//Запустить немедленно
		NULL);												//Перменная, которая принимает идентификатор потока
		//Поток-писатель
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
	FreeLibrary(library);								//Выгркзка dll библиотеки					
}