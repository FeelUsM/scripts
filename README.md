# scripts

мои скрипты

Некоторые скрипты требуют что бы в директории `/usr/include` была символьная ссылка `strstr` на [common-parse-lib](https://github.com/FeelUsM/common-parse-lib).

**runcpp** --help  - находит, компилирует и запускает одну единицу трансляции на си, c++, asm...

(*) **ll2lpath**  - создает список всех файлов в текущей (или заданных) директории, и помещает его в .files/ (или в заданную директорию)

(*) **lsdups**  - ищет повторяющиеся файлы и папки и генерирует скрипт для их удаления

(*) **diffiles-internal.cpp** - по двум спискам файлов выдает список измененных, перемещеных/переименованных и созданных/удаленных файлов

(*) **diffiles** - выдает список изменений в каталоге с момента последнего добавления строки `commit` в .files/list

(*) **unique-datesize.cpp** - печатает файлы с одинаковыми датой и размером

**rmvoiddirs** - удаляет пустые директории и выводит их список

**whichpack** - как which, только еще ищет какому пакету принадлежит файл (под Cygwin и dpkg)

**nthline** - выводит каждую n-ю строку стандартного входа

**symlinkpath**  - по имени символьной ссылки печатает то, на что она ссылается

**evalcom** = eval, тоько как отдельная команда

**find-non-ascii.c** имя_файла -  выводит имя файла, если в нем содержаться не ascii символы

**nircmd.exe** shortcut файл папка название             - создает виндовый ярлык (не моя, где-то скачал)

**aliases-and-functions** - алиасы и функции, которые можно запускать из других скриптов, если подключить это файл.
(Но я так ни разу не делал, у меня он подключается в configs/shellrc.)

* **wgetsite** адрес            - рекурсивный wget, который игнорирует robots.txt
* **yacadav** - cadaver for yandex disk
* **path-n**  - sed-скрипт который выводит $PATH с одним путем на строку
* **winlink** файл папка название = nircmd shortcut
* **push** имя значение         - положить в стек
* **pop** имя                   - достать из стека и напечатать на стд. вывод. Если использовать как $(pop stknm) то сам стек не измениться
* **popto** имя переменная      - достать из стека в заданную переменную
* **myuntar**  - (не тестировал) - если в архиве одна папка - достает ее, иначе достает все в папку с именем архива
* **mytar** - даже не помню синтаксис
* **rm-to-trash** - удаляет в корзину
* **linkto** - создает симлинк $1 указывающий на $2
* **append_line** - добавляет строку $1 в файл $2, если там ее нет (TODO сделать кавычки для grep-а)
