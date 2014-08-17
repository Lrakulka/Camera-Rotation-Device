/* Имя: requests.h
 * Проект:  пример hid-custom-rq
 * Автор: Christian Starkjohann
 * Перевод: microsin.ru
 * Дата создания: 2008-04-09
 * Табуляция: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * Лицензия: GNU GPL v2 (см. License.txt) или проприетарная (CommercialLicense.txt) 
 * Ревизия: $Id: requests.h 553 2008-04-17 19:00:20Z cs $
 */

/* Этот заголовочный файл является общим между firmware и ПО хоста. Он задает
 *  номера запросов USB numbers (и опционально типы данных), используемые для
 *  коммуникации между хостом и устройством USB.
 */

#ifndef __REQUESTS_H_INCLUDED__
#define __REQUESTS_H_INCLUDED__

#define CUSTOM_RQ_SET_STATUS    1
/* Установка состояния LED (светодиод). Control-OUT.
 * Запрашиваемый статус передается в поле "wValue" управляющей (control)
 *  передачи. Никаких данных OUT не посылается. Бит 0 младшего байта wValue 
 *  управляет LED.
 */

#define CUSTOM_RQ_GET_STATUS    2
/* Получение состояния LED. Control-IN.
 * Эта управляющая передача (control transfer) вовлекает 1 байт данных, где 
 *  устройство отправляет текущее состояние хосту. Состояние передается в 
 *  бите 0 байта.
 */

#endif /* __REQUESTS_H_INCLUDED__ */
