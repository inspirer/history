// errno.h

#ifndef ERRNO_H_INCLUDED
#define ERRNO_H_INCLUDED

#include <stdlib.h>

#define ENOERR			0   // ������ �� ���������
#define EBADF           1   // ������ � �������� �����
#define ENOMEM          2	// �� ���������� ������, ������������ � �-��� fopen,setvbuf � ��
#define EACCES          3   // ������ ��������
#define EFAULT          4   // ��������� ������
#define EEXIST          5   // ���� ��� ����������
#define EINVAL          6   // ������� ������������ ��������
#define ENFILE          7   // ���� �� ������
#define EMFILE          8   // ������� ����� ������
#define ENOENT			9   // ���� ��� ���������� �� �������

#endif
