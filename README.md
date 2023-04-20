# Thermometer_esp8266

Небольшой проект домашнего термометра.

Основные функции:
1) Вывод температуры на экран
2) Вывод влажности воздуха на экран
3) Запись данных на сервер https://thingspeak.com/

Микроконтроллер, после проверки заряда аккумулятора, автоматически просыпается каждые 20 минут и отправляет данные на сервер.
По нажатию на кнопку выводит последовательно данные на экран. Также выводит актуальный заряд аккумулятора.

Фото:
</br>
<img src="Files/IMG_20230319_174846_1.jpg" alt="drawing" width="200"/>
<img src="Files/IMG_20230319_174854_1.jpg" alt="drawing" width="200"/>
<img src="Files/IMG_20230319_174858.jpg" alt="drawing" width="200"/>

Корпус был напечатан на 3D принтере.</br>

Схема устройства:
</br>
<img src="Files/Scheme.png" alt="drawing" width="200"/>

Список компонентов:
+ Esp8266
+ Аккумулятор 18650
+ Кнопка
+ Резистор на 100 кОм
+ Датчик AHT20
+ Дисплей SSD1306_128x32
+ Плата зарядки аккумулятора TP4056 

Пример работы на thingspeak.com:
</br>
<img src="Files/Screenshot_20230320-152139.png" alt="drawing" width="200"/>
<img src="Files/Screenshot_20230320-152159.png" alt="drawing" width="200"/>
