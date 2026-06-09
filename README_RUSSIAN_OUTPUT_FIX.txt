Исправление вывода русского языка

Что изменено:
1. Добавлен файл console_encoding.h.
2. В main вызывается efd::setupRussianConsole().
3. На Windows используется SetConsoleCP(CP_UTF8) и SetConsoleOutputCP(CP_UTF8).
4. В .vcxproj оставлен параметр компилятора /utf-8.
5. В .cpp/.h добавлен pragma execution_character_set("utf-8"), чтобы MSVC не портил русские строки.

Если в старой консоли Windows всё ещё видны квадраты, поменяй шрифт консоли на Consolas или Cascadia Mono и запускай проект через Windows Terminal / встроенный терминал Visual Studio.
