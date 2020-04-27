#ifndef LB5_LABA_LINUX_H
#define LB5_LABA_LINUX_H

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <aio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>

int running_threads = 0;
int volatile size = 0;
char message[1000];
char buffer[1000];
void *library;

struct Array										//Струкутра с именем файла
{
    char fileName[100];
};

struct Data
{
    struct Array *arr;								//Имена файлов
    pthread_t pReader;								//Поток-читатель
    pthread_t pWriter;								//Поток-писатель
    pthread_mutex_t mutRead;						//Мьютекс чтения
    pthread_mutex_t mutWrite;						//Мьютекс записи
};

void* OpenLib()
{
    library = dlopen("./libfun.so", RTLD_LAZY);		//Окрывает динамическую библиотеку, разрешает неопределенные символы
    return library;
}

//Поиск файлов
void Search(struct Data *data)
{
    DIR *dfd;										//Информация о директории
    struct dirent *dp;
    dfd = opendir("./Files/");						//Открываем директорию

    while((dp = readdir(dfd)) != NULL)				//Открываем следующий файл в директории
    {
        if (!strcmp(dp->d_name, "Allfiles.txt")) 	//Проверяем, не был ли найден конечный файл
        {
            continue;
        }
        if (!strcmp(dp->d_name, ".")) 				//Пропускаем файл - ссылку на себя
        {
            continue;
        }
        if (!strcmp(dp->d_name, "..")) 				//Пропускаем файл - ссылку на предыдущий каталог
        {
            continue;
        }
        else if(size == 0)
        {
            data->arr=(struct Array*) malloc(1 * sizeof(struct Array));		//Выделение памяти под первое имя файла
        }
        else
        {														//Выделение памяти под несколько имен файлов
            data->arr=(struct Array*) realloc(data->arr, (size+1) * sizeof(struct Array));
        }
        strcpy(data->arr[size].fileName, "./Files/");
        strcat(data->arr[size].fileName, dp->d_name);
        size++;									//Увеличиваем счетчик найденных файлов
    }
    size--;
    pthread_mutex_lock(&(data->mutWrite));			//Запрещаем потоку-писателю сработать первым
    for(int i =0;i<=size;i++)
    {
        puts(data->arr[i].fileName);
    }
    closedir(dfd);									//Закрываем директорию
}

//Функция потока-читателя
void* ThreadReader(void *arg)
{
    struct Data *data = (struct Data *)arg;			//Для удобства
    void (*readDataFromFile)(int, char*);
    *(void **) (&readDataFromFile) = dlsym(library, "ReadDataFromFile");	//Получаем указатель на функцию для чтения

    while (1)
    {
        pthread_mutex_lock(&(data->mutRead));		//Захватывает мьютекс чтения
        if (size < 0)
        {
            break;
        }
        int fd = open(data->arr[size].fileName, O_RDONLY);		//Открываем файл только для чтения
        (*readDataFromFile)(fd, buffer);			//Читаем информация из файла
        printf("Reader: I read data from %s\n", data->arr[size].fileName);
        close(fd);									//Закрываем файл
        pthread_mutex_unlock(&(data->mutWrite));	//Разрешаем работу потоку-писателю
    }

    running_threads--;
    return 0;
}

//Функция потока-писателя
void* ThreadWriter(void *arg)
{
    struct Data *data = (struct Data *)arg;
    int fd = open("./Files/Allfiles.txt", O_WRONLY	//Открывает файл только для чтения,
                                          | O_CREAT										//если не существует, создает
                                          | O_TRUNC										//если существует, обрезает до 0
                                          | O_APPEND);									//открывает в режиме добавления.(перед каждой write пермещает указатель в конец)
    void (*writeDataInFile)(int, char*);
    *(void **) (&writeDataInFile) = dlsym(library, "WriteDataInFile");	//Получаем указатель на функцию из динамической библиотеки

    do
    {
        pthread_mutex_lock(&(data->mutWrite));		//Захватывает мьютекс записи
        (*writeDataInFile)(fd, buffer);				//Записывает информацию в файл
        printf("Writer: I wrote data from %s\n", data->arr[size].fileName);
        size--;
        pthread_mutex_unlock(&(data->mutRead));		//Разрешает чтение
    } while (size >= 0);

    running_threads--;
    close(fd);										//Закрывает файл
    return 0;
}

//Создание потоков
void CreateThreads(struct Data *data)
{
    pthread_create(&(data->pReader),				//Идентификатор потока
                   NULL,											//Атрибуты по умолчанию
                   &ThreadReader,									//Функция потока
                   data);											//Параметры, передаваемые в поток
    pthread_create(&(data->pWriter), NULL, &ThreadWriter, data);		//Создание потока-писателя
}

//Инициализирует мьютексы
void CreateEventForThreads(struct Data *data)
{
    pthread_mutex_init(&(data->mutRead),			//Идентификатор мьютекса
                       NULL);											//Атрибуты по умолчанию
    running_threads++;
    pthread_mutex_init(&(data->mutWrite), NULL);
    running_threads++;
}

//Завершает приложение
void CloseApp(struct Data *data)
{
    pthread_mutex_destroy(&(data->mutRead));		//Удаляем мьютекс
    pthread_mutex_destroy(&(data->mutWrite));
    dlclose(library);
}

//Ожилаение завершения потоков
void WaitThreads(struct Data *data)
{
    while (running_threads > 0)
    {
        usleep(1);
    }
}

#endif //LB5_LABA_LINUX_H
