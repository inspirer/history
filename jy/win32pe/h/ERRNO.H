// errno.h

#ifndef ERRNO_H_INCLUDED
#define ERRNO_H_INCLUDED

#include <stdlib.h>

#define ENOERR			0   // ошибки не произошло
#define EBADF           1   // ошибка в открытии файла
#define ENOMEM          2	// не достаточно памяти, используется в ф-ции fopen,setvbuf и др
#define EACCES          3   // доступ запрещен
#define EFAULT          4   // системная ошибка
#define EEXIST          5   // файл уже существует
#define EINVAL          6   // передан неправильный параметр
#define ENFILE          7   // файл не найден
#define EMFILE          8   // слишком много файлов
#define ENOENT			9   // файл или директория не найдены

#endif
