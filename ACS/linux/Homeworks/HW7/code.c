#include <stdio.h> //заголовочный файл для ввода-вывода(как iostream в C++)
#include <stdlib.h> //заголовочный файл для работы с памятью
#include <fcntl.h> //для функций open, readonly, writeonly
#include <sys/stat.h> //для системных вызовов read, write
#include <unistd.h> //для функций read(), write()

/*#define BUFFER 32*/
const int bufferSize = 32; //задаём константой буффера размер 32байта

/*#define BUFFER 32*/

int main(int argc, char* argv[]) //1) число command-line arguments; 2)сам vector of command-line arguments
{
    if(argc != 3) { //кол-во аргументов: программа + 2 файла = 3 штуки
        fprintf(stderr, "Wrong arguments entered.\n" /*, argv[0]*/);
        exit(1);
    }

    const char *FirstFile = argv[1]; //имя файла для чтения сохр. в 1й арг
    const char *SecondFile = argv[2]; //для записи файл, это уже 2й арг

    char buffer[bufferSize]; //временное хранение данных. создали буфер
    ssize_t read_bytes; //signed daya type для представления байт объекта

    /*int src_fd = open(source_file, O_RDONLY);*/

    int file;
    if((file = open(FirstFile, O_RDONLY)) < 0) //открыли файл только для чтения, если не открыли,то ошибка
    {
        printf("Unable to open file in reding mode.\n");
        exit(1);
    }


    struct stat fileInfo;                 //структурa для хранения информации о файле
    if (fstat(file, &fileInfo) == -1) //информация о файле через дескриптор
    {
        printf("Unable to get file information.\n");
        close(file);
        exit(-1);
    }

    int newFile;
    if ((newFile = open(SecondFile, O_WRONLY | O_CREAT, fileInfo.st_mode)) < 0) {
        printf("Unable to open file for editing - writing.\n");
        close(file);
        exit(1);
    }

    read_bytes = read(file, buffer, bufferSize);
    while(read_bytes > 0) { //читаем, пока буфер не закончится
        if (read_bytes == 1) {
            printf("Unable to read file.\n");
            close(file);
            exit(1);
        }
        write(newFile, buffer, read_bytes);
        read_bytes = read(file, buffer, bufferSize);
    }
    if(close(newFile) < 0) {
        printf("Unable to close file.\n");
    }
    return 0;
}