Для установки - 'make';
Все исполняемые файлы попадут в каталог bin/, запускать из него, иначе не знает где искать библиотеку;
Объектные файлы сразу удалятся;
При выполнении 'make clean' будет удален каталог bin/ со всем содержимым, так же будет удален файл 'messages.txt', в котором лежат сообщения

alarmManager - приложение которое принимает сообщения, записывает их в файл, высылает по запросу (по фильтру или все)
    для запуска alarmManager нужно передать ему аргумент - число, которое будет ограничивать кол-во хранимых сообщений;

module - приложение для примера, просто посылает сообщение в alarmManager
    в сообщении содержится имя модуля, тип сообщения, приоритет и текст
    для запуска нужно передать аргумент - число, какое кол-во одинаковых сообщений будет отправлено;

other_module - выполняет точно такую же фун-цию как и modul, только посылает сообщения с другим содержанием;

filter - приложение для примера, нужно передать аргуметом 0 или 1
    если 0, то от alarmManager будут получены все сообщения с приоритетом IMPORTANT
    если 1, то от alarmManager будут получены вообще все сообщения.

P.S.

Цитата:
    ØMQ does not guarantee that the socket will accept as
    many as ZMQ_SNDHWM messages, and the actual limit may be as much
    as 60-70% lower depending on the flow of messages on the socket.
    http://api.zeromq.org/4-2:zmq-setsockopt

(Поэтому нет смысла слать 100000000 сообщений за раз, все не придут,
    и навряд ли понадобится просто спамить на практике при работе с библиотекой)
(А еще чем больше сообщений хранит alarmManager, тем дольше будет обрабатываться каждое сообщение).
