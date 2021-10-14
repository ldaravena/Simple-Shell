# Simple-Shell
Simple Shell
OS - UdeC 2021-2

Instrucciones para poder compilar y ejecutar este programa.
    1. En la terminal escribir: g++ test.cpp
    2. Luego: ./a.out
    3. Escribir el comando a ejecutar.
        3.1 Si lo que se quiere ejecutar tiene pipes, se deben separar con "|".
        3.2 Si se quiere crear un nuevo comando personalizado se debe escribir: cmdmonset nombre comandoAdministracion x z
            - nombre: nombre que se le quiere asignar al comando personalizado.
            - comandoAdministracion: Debe ser vmstat o netstat.
            - x: intervalo de tiempo que espera para volver a ejecutar el comandoAdministracion.
            - z: tiempo total en el que se ejecuta el comandoAdministracion (x < z).
