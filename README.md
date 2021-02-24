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

* **bool** - pred=$(bool 2 = 2); if $pred; then ... ; $pred && echo 1 || echo 2
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
* **linksave** - создает симлинк $2 указывающий на $1
* **append_ifnot_line** - добавляет строку $1 в файл $2, если там ее нет (TODO сделать кавычки для grep-а)
* **curpos** - возвращает столбец;строку , где находится курсор

# lsdups - manual

получает на вход список файлов и список хешей некоторых файлов (последнее необязательно)
выделяет файлы с одинаковыми размерами, 
и среди тех из них у кого нет хеша - вычисляет его и записывает в файл с хешами
после чего рассматриваются только файлы с одинаковыми хешами
--- после чего идет умная сортировка ---
ГФ - группа файлов - группа одинаковых файлов (не меньше двух)
ГП - группа папок - набор папок, содержащих одинаковые файлы с одинаковыми именами
	
	состоит из списка папок (не меньше двух) и списка файлов (1 или более) относительно каждой папки
	в каждой ГФ:
		ищутся файлы с самым длинным совпадающим с конца путем, вычисляется его длина
		после чего ищутся файлы с совпадающим с конца путем этой длины,
		совпадающая часть отбрасывается, и из полученного формируется список папок
	И так в каждый список папок добавляется файл, а требуемая длина совпадающего с конца пути постепенно уменьшается
ГГПФ - группа групп папок и файлов - находящихся внутри определенной папки
	в _ГГП помещается какая-то одна ГП
	_ГГП имеет
		список общих папок 
		и список общих файлов
	по всем ГП ищется пересечение
		списка общих папок со списком папок текущей ГП
		списка общих файлов со списком фалов текущей ГП
	если хотябы одно из них не пусто, то
		эта ГП добавляется в _ГГП, и списки общих папок и общих файлов обновляются
	после чего в списке общих папок ищется общий путь, и по нему эта _ГГП добавляется в ГГПФ
ГФы, которые учавствовали в ГПах игнорируются
	но из ГФ с файлами размера 0 файлы, файлы которые учавствовали в ГП-ах просто удаляются,
	и таким образом эта ГФ не может игнорироваться, и в ней может присутствовать 1 файл (или более)
для остальных ГФов для каджой ищется общий путь, и по нему она добавляется в ГГПФ
--- после чего идет вывод ---
он разбит на ГГПФ с заголовками #=== === и функцией f, с последующим ее вызовом, и выводом е размера
вначале выводятся все _ГГП, в конце отделенные #--- --- со статистикой
	внутри выводяится ГПы - функция rmfromdir со статистикой и последующие ее вызовы
		внутри rmfromdir() для каждого файла выводится
			rm сам файл, # его размер, хэш, 
			количество файлов, которые учавствуют в ГП, но не относятся к текущему файлу в данной ГП (еще ...)
			#rm -f файлы, которые принадлежат текущему ГФу, но не относятся ни к одной ГП
				- но только при первом разе // убрано
		потом выводятся вызовы #rmfromdir с указанием количества файлов в директории, которых нет в списке файлов этой ГП
			и если их меньше пяти, то они просто перечисляются дальше
потом выводятся все ГФы, в конце #--- --- со статистикой
	(им хэш не нужен, т.к. они все собраны в одном месте)
