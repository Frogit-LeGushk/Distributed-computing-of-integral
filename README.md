# Distributed-computing-integral
Запустим один главный сервер (master) и несколько вычислительных узлов (slaves), соединяем вычислительные узлы с главным сервером при помощи сокетов (по локальной сети), вводим пределы интегрирования и запускаем вычисления. Функцию, по которой ведется интегрирование можно менять `see FUNCTION.cpp`
### Screencast:
![alt text](https://github.com/Acool4ik/Distributed-computing-system/blob/master/images/Screencast.gif)
### Then to run main node (server):
- `make run_server`
### Then to run calculating node (client):
- `make run_client`    
### If you want calculate castom integral:
- Open file `FUNCTION.cpp` in any text redactor
- Change body of function on other expression
- Use only `math.h` lib function for build castom expression
- Process errors yourself (for example devide by zero)
### Note:
- calculate available only in local area network 
- IP of server need input on client manually (see screencast)
- Make sure what you use the same network in other devices
