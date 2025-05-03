# Trabajo Práctico Número 3

## Unit testing en proyecto personal

Este repositorio utiliza las siguientes herramientas:

    ceedling para ejecutar las pruebas unitarias en forma automatizada
    lcov para generar los informes de cobertura de las pruebas unitarias

Para ejecutar las pruebas unitarias se utiliza el siguiente comando:

ceedling test:all

Para generar el informe de cobertura de las pruebas se utiliza el siguiente comando:

ceedling clobber gcov:all utils:gcov

# Resumen del proyecto

api_spi_flash es un driver para una memoria flash SPI. Es un módulo fundamental para el proyecto de grado: Implementación de un bootloader para el circuito integrado SiGW917.