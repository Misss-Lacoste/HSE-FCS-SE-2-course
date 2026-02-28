

### **Лекция 6: SQL - Искусство декларативной трансформации данных**

## От `SELECT *` до аналитических окон и рекурсивных запросов

## План на сегодня

1.  Продвинутые `JOIN`'ы и cетовые операции.
2.  Подзапросы всех видов и **Common Table Expressions (CTE)**.
3.  Рекурсивные запросы для работы с иерархиями.
4.  **Оконные функции** - вычисления без `GROUP BY`.
5.  **Продвинутая модификация данных:** `MERGE` и `UPDATE FROM`.
### **1. Больше, чем просто `INNER JOIN`**

Представим две таблицы: `Employees(EmployeeID, Name, DepartmentID)` и `Departments(DepartmentID, DeptName)`.

*   **`FULL OUTER JOIN`**: "Покажи мне всё из обеих таблиц, соедини, что можешь, а для несовпадений подставь `NULL`".
    *   **Ключевой сценарий:** Поиск расхождений. "Найти всех сотрудников, у которых не указан или указан несуществующий отдел, А ТАКЖЕ все отделы, в которых нет ни одного сотрудника".
    ```sql
    SELECT E.Name, D.DeptName
    FROM Employees E
    FULL OUTER JOIN Departments D ON E.DepartmentID = D.DepartmentID;
    ```
    Это невозможно сделать одним `LEFT` или `RIGHT` JOIN'ом.

*   **`CROSS JOIN`**: Декартово произведение. "Соедини каждую строку из первой таблицы с каждой строкой из второй".
    *   **Ключевой сценарий:** Генерация данных. "Создать отчет, показывающий плановый KPI для каждого сотрудника на каждый месяц года".
    ```sql
    -- Months(MonthName) - простая таблица с 12 месяцами
    SELECT E.Name, M.MonthName, 10000 AS PlanKPI -- План 10000 для всех
    FROM Employees E
    CROSS JOIN Months M;
    ```
*   **Теоретико-множественные операции:**
    *   `UNION`: Объединяет результаты двух запросов, **удаляя дубликаты** (медленно).
    *   `UNION ALL`: Объединяет результаты, **оставляя все дубликаты** (очень быстро). Используйте его, если уверены в отсутствии дублей.
    *   `INTERSECT`: Возвращает только те строки, которые есть **в обоих** запросах.
    *   `EXCEPT`: Возвращает строки из первого запроса, которых **нет** во втором.
    *   **Сценарий:** "Найти всех клиентов из Москвы (`Customers_Moscow`) , которые при этом не являются VIP-клиентами (`VIP_Customers`)".
    ```sql
    SELECT CustomerID, Name FROM Customers_Moscow
    EXCEPT
    SELECT CustomerID, Name FROM VIP_Customers;
    ```
### **2. Подзапросы и CTE**

#### **Подзапросы**

*   **Скалярный подзапрос** (возвращает 1 строку, 1 столбец):
    *   Используется там, где ожидается одно значение.
    ```sql
    SELECT Name, Salary, (SELECT AVG(Salary) FROM Employees) AS AvgCompanySalary
    FROM Employees;
    ```
*   **Многострочный подзапрос** (возвращает 1 столбец, много строк):
    *   Используется с операторами `IN`, `ANY`, `ALL`.
    ```sql
    -- Найти все отделы, в которых есть сотрудники с зарплатой выше 100k
    SELECT DeptName FROM Departments
    WHERE DepartmentID IN (SELECT DepartmentID FROM Employees WHERE Salary > 100000);
    ```
*   **Коррелированный подзапрос:** Внутренний запрос, который зависит от внешнего. Выполняется **для каждой строки** внешнего запроса. **Очень медленный, избегайте его по возможности!**
    *   **Классическая задача:** "Найти всех сотрудников, получающих максимальную зарплату в своем отделе".
    ```sql
    SELECT e1.Name, e1.Salary, e1.DepartmentID
    FROM Employees e1
    WHERE e1.Salary = (SELECT MAX(e2.Salary)
                       FROM Employees e2
                       WHERE e2.DepartmentID = e1.DepartmentID); -- Корреляция
    ```
#### **Common Table Expressions (CTE) - именованные представления**

CTE (`WITH ... AS`) позволяет разбить сложный запрос на логические, читаемые шаги. Это как временные именованные таблицы, существующие только на время выполнения запроса.

*   **Цель:** Убрать пирамиду из вложенных подзапросов, улучшить читаемость и возможность повторного использования логики.
*   **Синтаксис:**
    ```sql
    WITH
      -- Шаг 1: Находим активных пользователей
      ActiveUsers AS (
        SELECT UserID FROM Orders WHERE OrderDate > '2023-01-01'
      ),
      -- Шаг 2: Считаем их заказы, ссылаясь на шаг 1
      UserOrderCounts AS (
        SELECT UserID, COUNT(*) as OrderCount
        FROM Orders
        WHERE UserID IN (SELECT UserID FROM ActiveUsers)
        GROUP BY UserID
      )
    -- Финальный запрос, использующий результаты шагов
    SELECT u.Name, uoc.OrderCount
    FROM Users u
    JOIN UserOrderCounts uoc ON u.UserID = uoc.UserID
    WHERE uoc.OrderCount > 5;
    ```
### **3. Рекурсивные CTE**

Как найти всех подчиненных конкретного менеджера на всю глубину иерархии одним запросом?

*   **Рекурсивные CTE** - это механизм для обхода иерархических или графовых структур.
*   **Структура:**
    1.  **Якорная часть:** Начальное условие, "корень" рекурсии.
    2.  `UNION ALL`.
    3.  **Рекурсивная часть:** Запрос, который ссылается на саму CTE, делая "шаг" по иерархии.
    4.  Условие остановки (в `WHERE` рекурсивной части).

*   **Задача:** "Найти всех сотрудников в подчинении у менеджера с `EmployeeID = 10`". (Таблица `Employees(EmployeeID, Name, ManagerID)`)

   ```sql
    WITH RECURSIVE Subordinates AS (
      -- 1. Якорь: Начинаем с прямых подчиненных менеджера 10
      SELECT EmployeeID, Name, ManagerID, 1 AS Level
      FROM Employees
      WHERE ManagerID = 10
      UNION ALL      -- 2. Рекурсия: Соединяем Employees с теми, кого мы уже нашли (Subordinates)
      SELECT e.EmployeeID, e.Name, e.ManagerID, s.Level + 1
      FROM Employees e
      JOIN Subordinates s ON e.ManagerID = s.EmployeeID -- Шаг вглубь
    )
    SELECT * FROM Subordinates;
    ```
### **4. Оконные функции**

**Проблема:** `GROUP BY` агрегирует множество строк в одну. А что, если мы хотим выполнить агрегацию, но сохранить все исходные строки? Например, для каждой строки сотрудника показать среднюю зарплату по его отделу?
**Оконные функции** выполняют вычисления на наборе строк ("окне"), но возвращают результат для **каждой** строки.

**Синтаксис:** `ФУНКЦИЯ() OVER (PARTITION BY ... ORDER BY ...)`
*   `PARTITION BY col`: Определяет "окно". Это как `GROUP BY`, но без сжатия строк.
*   `ORDER BY col`: Задает порядок строк внутри "окна", что важно для ранжирования и накопительных итогов.
#### **Типы и сценарии:**
*   **Ранжирование:** `ROW_NUMBER()`, `RANK()`, `DENSE_RANK()`, `NTILE(n)`.
    *   **Задача (решаем проблему с коррелированным подзапросом):** "Найти 3 самых высокооплачиваемых сотрудника в каждом отделе".
    ```sql
    WITH RankedSalaries AS (
      SELECT
        Name,
        Salary,
        DeptName,
        ROW_NUMBER() OVER (PARTITION BY DeptName ORDER BY Salary DESC) as rn
      FROM Employees e JOIN Departments d ON e.DepartmentID = d.DepartmentID
    )
    SELECT Name, Salary, DeptName
    FROM RankedSalaries
    WHERE rn <= 3;
    ```
*   **Агрегирующие:** `SUM()`, `AVG()`, `COUNT()`, `MAX()`, `MIN()`...
    *   **Задача:** "Для каждого сотрудника показать его зарплату, а также среднюю и максимальную по его отделу".
    ```sql
    SELECT
      Name,
      Salary,
      AVG(Salary) OVER (PARTITION BY DepartmentID) as AvgDeptSalary,
      MAX(Salary) OVER (PARTITION BY DepartmentID) as MaxDeptSalary
    FROM Employees;
    ```
*   **Смещения:** `LAG(col, n)` (взять значение из строки на `n` позиций **назад**), `LEAD(col, n)` (вперед).
    *   **Задача:** "Для каждого заказа показать его сумму и сумму **предыдущего** заказа от этого же клиента".
    ```sql
    SELECT
      OrderID,
      CustomerID,
      OrderAmount,
      LAG(OrderAmount, 1, 0) OVER (PARTITION BY CustomerID ORDER BY OrderDate) as PreviousOrderAmount
    FROM Orders;
    ```
### **5. Продвинутая модификация данных**

*   **`UPDATE ... FROM`**: Обновление одной таблицы на основе данных из другой.
    *   **Задача:** "Обновить поле `CustomerTotalOrders` в таблице `Customers` на основе данных из `Orders`".
    ```sql
    UPDATE Customers c
    SET CustomerTotalOrders = o.TotalCount
    FROM (SELECT CustomerID, COUNT(*) as TotalCount FROM Orders GROUP BY CustomerID) AS o
    WHERE c.CustomerID = o.CustomerID;
    ```

*   **`MERGE` (или `INSERT ... ON CONFLICT` в PostgreSQL):** Операция "UPSERT". "Если запись существует - обнови ее, если нет - вставь".
    *   **Задача:** "Загрузить ежедневные данные о просмотрах страниц. Если для страницы уже есть запись за сегодня, обновить счетчик. Если нет - создать новую".
    ```sql
    -- Синтаксис PostgreSQL
    INSERT INTO DailyPageViews (PageID, ViewDate, ViewCount)
    VALUES ('/home', '2023-10-27', 100)
    ON CONFLICT (PageID, ViewDate) DO UPDATE
      SET ViewCount = DailyPageViews.ViewCount + EXCLUDED.ViewCount;
    ```
### **Заключение**

Сегодня мы сделали шаг от простого извлечения данных к их сложной, декларативной обработке. Вы увидели, что SQL - это не просто язык запросов, а мощнейший инструмент для трансформации, анализа и манипуляции данными прямо "на лету".
*   **CTE** делают ваш код читаемым и структурированным.
*   **Рекурсивные CTE** позволяют работать с иерархиями без циклов.
*   **Оконные функции** открывают целый мир аналитики, недоступный при использовании `GROUP BY`.
