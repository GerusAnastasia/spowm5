#include "library.h"

#include <stdio.h>

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <aio.h>

#define BUFFER_SIZE 512

extern "C" void WriteDataInFile(int fd, char* buff)
{
    struct aiocb aiocb;
    memset(&aiocb, 0, sizeof(struct aiocb));		//Зануляем структуру
    aiocb.aio_fildes = fd;							//Дескриптор файла
    aiocb.aio_buf = buff;							//Буфер для записи
    aiocb.aio_nbytes = strlen(buff);				//Размер буфера
    aiocb.aio_lio_opcode = LIO_WRITE;				//Операция записи
    if (aio_write(&aiocb) == -1)
    {
        printf("Error at aio_write()\n");
        close(fd);
        exit(0);
    }
    while (aio_error(&aiocb) == EINPROGRESS);		//Ждем пока выполнится запрос
}

extern "C" void ReadDataFromFile(int fd, char* buff)
{
    memset(buff, 0, BUFFER_SIZE * sizeof(char));	//Буфер
    struct aiocb aiocb;
    memset(&aiocb, 0, sizeof(struct aiocb));		//Зануляем структуру
    aiocb.aio_fildes = fd;							//Дескриптор файла
    aiocb.aio_buf = buff;							//Буфер для чтения
    aiocb.aio_nbytes = BUFFER_SIZE;					//Размер буфера
    aiocb.aio_lio_opcode = LIO_READ;				//Чтение из файла
    if (aio_read(&aiocb) == -1)
    {
        printf("Error at aio_read()\n");
        close(fd);
        exit(0);
    }
    while (aio_error(&aiocb) == EINPROGRESS);		//Ждем пока операция завершится
}