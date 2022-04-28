//1. Нужно создать проект на github и написать веб-сервер. Протокол HTTP 1.0, нужно реализовать только команду GET (POST - опционально),
// ответы 200 и 404, а также MIME-тип text/html (другие типы, например image/jpeg - опционально).

//2. Запустить виртуальную машину и сохранить http-путь до вашего github репозитория в файле /home/box/final.txt
//3. Первым делом тестовая среда проверяет наличие этого файла и сама клонирует репозиторий в /home/box/final.
//5. В результате сборки должен появиться исполняемый файл /home/box/final/final - ваш веб-сервер. Тестовая среда проверяет его наличие.

//6. Веб-сервер должен запускаться командой:
// /home/box/final/final -h <ip> -p <port> -d <directory>

#include <iostream>

#include "web_params.h"
#include "handler.h"


int main(int argc, char** argv)
{



    WebParams web_params;
    web_params.parse(argc, argv);

    Handler handler(web_params);
    handler.run();

    return 0;
}