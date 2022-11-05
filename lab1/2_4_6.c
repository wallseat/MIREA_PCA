#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/time.h>

int main(int argc, char **argv)
{
    // Проверяем наличие имени файла первым аргументом
    char *filename;
    if (argc >= 2)
    {
        filename = argv[1];
    }
    else
    {
        printf("Usage: %s FILENAME PROCESS_NUM\n", argv[0]);
        return -1;
    }

    // Проверяем наличие аргумента количества процессов
    int p_num;
    if (argc >= 3)
    {
        p_num = atoi(argv[2]);
    }
    else
    {
        printf("Usage: %s FILENAME PROCESS_NUM\n", argv[0]);
        return -1;
    }

    // Открываем файл по переданному имени
    int fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("Error opening file %s", filename);
        return -1;
    }

    // Получаем размер файла
    int size = lseek(fd, 0, SEEK_END);

    // Отображаем файл в память
    char *mm_addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // По описанию mmap файловый дескриптор можно закрыть сразу после отображения
    close(fd);

    // Создаем дополнительный буфер для чтения
    char *buf = malloc(size);

    // Копируем данные файла в буфер
    memcpy(buf, mm_addr, size);

    // Рассчитываем какая часть файла будет обрабатываться каждым процессом
    int bytes_per_process = size / p_num;
    int bytes_remainder = size % p_num;

    // Создаем структуру для записи времени
    struct timeval start;
    gettimeofday(&start, NULL);

    int p_i = 0;

    // Создаем дочерние процессы
    for (int i = 1; i < p_num; i++)
    {
        int f_res = fork();
        if (f_res == -1)
        {
            printf("Error creating process %d", p_i);
            return -1;
        }
        else if (f_res == 0)
        {
            p_i = i;
            break;
        }
    }

    int offset = 0;
    if (bytes_remainder && (p_i == (p_num - 1)))
    {
        offset = bytes_remainder;
    }

    // Процесс обрабатывает свою часть файла
    for (int i = bytes_per_process + offset - 1; i >= 0; i--)
    {
        mm_addr[bytes_remainder + (p_num - p_i) * bytes_per_process - 1 - i] = buf[i + p_i * bytes_per_process];
    }

    // Для родительского процесса ждем завершения всех дочерних
    if (p_i == 0)
    {
        while (1)
        {
            int status;
            pid_t done = wait(&status);
            if (done == -1)
            {
                if (errno == ECHILD)
                {
                    break;
                }
            }
            else
            {
                if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
                {
                    printf("Error in child process %d\n", done);
                    return -1;
                }
            }
        }

        // Закрываем отображение в память
        if (munmap(mm_addr, size))
        {
            printf("Error unmaping file %s", filename);
            return -1;
        }

        // Освобождаем память под буфер
        free(buf);

        // Записываем время выполнения
        struct timeval stop;
        gettimeofday(&stop, NULL);
        printf("took %lu us\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);
    }

    return 0;
}