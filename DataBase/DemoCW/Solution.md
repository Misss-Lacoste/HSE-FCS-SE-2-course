#### Решение демо-версии контрольной работы Горбачеваой Маргарита БПИ-245 (задания 1-6)
#### **Задание 1: Концептуальная модель (ER-диаграмма)**

*   **Сущности:**
    *   `Пользователь` (супертип)
    *   `Заказчик` (подтип `Пользователя`)
    *   `Фрилансер` (подтип `Пользователя`)
    *   `Проект`
    *   `Заявка` (Ставка)
    *   `Отзыв`

*   **Атрибуты:**
    *   `Пользователь`: <u>UserID</u>, Name, RegistrationDate
    *   `Заказчик`: (наследует от Пользователя)
    *   `Фрилансер`: (наследует от Пользователя), Specialization, HourlyRate
    *   `Проект`: <u>ProjectID</u>, UserCustomerID, Title, Description, Budget, Status
    *   `Заявка`: <u>BidID</u>, UserFreelancerID, ProjectID, ProposedCost, ProposedCompletionDate, Status, SubmissionDate, AcceptanceDate, RejectionDate
    *   `Отзыв`: <u>ReviewID</u>, ProjectID, Rating, CommentText

*   **Связи:**
    *   `Заказчик` <  `Проект` (1:N)
    *   `Фрилансер` < `Заявку` (1:N)
    *   `Проект` < `Заявки` (1:N)
    *   `Заказчик` < `Отзыв` (1:N)
    *   `Проект` - `Отзыв` (1:1, отзыв можно оставить только один раз)
    *   `Фрилансер` < `Отзыв` (1:N)

**Диаграмма (текстовое описание):**
*   Иерархия ISA от `Пользователя` к `Заказчику` и `Фрилансеру`.
*   Прямоугольник `Заказчик` связан линией (с кардинальностью "1" у заказчика и "N" у проекта) с ромбом "публикует", который связан с прямоугольником `Проект`.
*   `Проект` и `Фрилансер` сходятся в ромб "подает заявку", который представляет собой сущность `Заявка` (ассоциативная сущность, ставшая сильной). Связи от `Проекта` и `Фрилансера` к `Заявке` — обе 1:N.
*   Сущность `Отзыв` связана с `Заказчиком`, `Проектом` и `Фрилансером`.

#### **Задание 2: Логическая модель (SQL DDL)**

```sql
CREATE TABLE Users (
    user_id SERIAL PRIMARY KEY,
    user_name VARCHAR(150) NOT NULL,
    registration_date DATE NOT NULL DEFAULT CURRENT_DATE,
    user_type VARCHAR(20) NOT NULL CHECK (user_type IN ('customer', 'freelancer'))
);

CREATE TABLE Customers (
    user_id INT PRIMARY KEY REFERENCES Users(user_id)
);

CREATE TABLE Freelancers (
    user_id INT PRIMARY KEY REFERENCES Users(user_id),
    specialization VARCHAR(100) NOT NULL,
    hourly_rate NUMERIC(10, 2) CHECK (hourly_rate >= 0)
);

CREATE TABLE Projects (
    project_id SERIAL PRIMARY KEY,
    customer_id INT NOT NULL REFERENCES Customers(user_id),
    title VARCHAR(200) NOT NULL,
    description TEXT,
    budget NUMERIC(12, 2) NOT NULL CHECK (budget >= 0),
    status VARCHAR(50) NOT NULL DEFAULT 'open' CHECK (status IN ('open', 'in_progress', 'completed'))
);

CREATE TABLE Bids (
    bid_id SERIAL PRIMARY KEY,
    project_id INT NOT NULL REFERENCES Projects(project_id),
    freelancer_id INT NOT NULL REFERENCES Freelancers(user_id),
    proposed_cost NUMERIC(12, 2) NOT NULL CHECK (proposed_cost >= 0),
    proposed_completion_date DATE,
    status VARCHAR(50) NOT NULL DEFAULT 'submitted' CHECK (status IN ('submitted', 'accepted', 'rejected')),
    submission_date TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    decision_date TIMESTAMPTZ, -- Дата принятия или отклонения
    UNIQUE (project_id, freelancer_id) -- Один фрилансер - одна заявка на проект
);

CREATE TABLE Reviews (
    review_id SERIAL PRIMARY KEY,
    project_id INT NOT NULL UNIQUE REFERENCES Projects(project_id), -- Уникально, т.к. 1 отзыв на 1 проект
    customer_id INT NOT NULL REFERENCES Customers(user_id),
    freelancer_id INT NOT NULL REFERENCES Freelancers(user_id),
    rating INT NOT NULL CHECK (rating >= 1 AND rating <= 5),
    comment_text TEXT,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);
```
#### **Задание 3: Фрилансеры-'Python' с более чем 3 заявками**

```sql
SELECT
    u.user_name,
    COUNT(b.bid_id) AS bid_count
FROM Freelancers AS f
JOIN Users AS u ON f.user_id = u.user_id
JOIN Bids AS b ON f.user_id = b.freelancer_id
WHERE
    f.specialization = 'Python'
    AND EXTRACT(YEAR FROM b.submission_date) = 2023
GROUP BY u.user_id, u.user_name
HAVING COUNT(b.bid_id) > 3
ORDER BY bid_count DESC;

#### **Задание 4: Проекты только от "дорогих" фрилансеров**

``sql
SELECT p.project_id, p.title, p.budget
FROM Projects p
WHERE p.budget > 50000
  AND NOT EXISTS (
    -- Подзапрос для поиска "плохих" заявок на этот проект
    SELECT 1
    FROM Bids b
    JOIN Freelancers f ON b.freelancer_id = f.user_id
    WHERE b.project_id = p.project_id
      AND f.hourly_rate < 1500
);

-- Альтернативное решение с GROUP BY и HAVING, менее интуитивное, но рабочее
SELECT p.project_id, p.title
FROM Projects p
JOIN Bids b ON p.project_id = b.project_id
JOIN Freelancers f ON b.freelancer_id = f.user_id
WHERE p.budget > 50000
GROUP BY p.project_id, p.title
HAVING MIN(f.hourly_rate) >= 1500;
``

#### **Задание 5: Среднее количество заявок по специализациям**

``sql
-- Считаем общее количество заявок для каждой специализации
WITH SpecializationBidCounts AS (
    SELECT
        f.specialization,
        COUNT(b.bid_id) AS total_bids
    FROM Freelancers f
    JOIN Bids b ON f.user_id = b.freelancer_id
    GROUP BY f.specialization
),
-- Считаем количество уникальных проектов для каждой специализации
SpecializationProjectCounts AS (
    SELECT
        f.specialization,
        COUNT(DISTINCT b.project_id) AS total_projects
    FROM Freelancers f
    JOIN Bids b ON f.user_id = b.freelancer_id
    GROUP BY f.specialization
)
-- Делим одно на другое
SELECT
    sbc.specialization,
    CAST(sbc.total_bids AS NUMERIC) / spc.total_projects AS avg_bids_per_project
FROM SpecializationBidCounts sbc
JOIN SpecializationProjectCounts spc ON sbc.specialization = spc.specialization
ORDER BY avg_bids_per_project DESC;
```

#### **Задание 6: Отчет по завершенным проектам за прошлый месяц**

```sql
SELECT
    f.specialization,
    COUNT(DISTINCT p.project_id) AS completed_projects_count,
    SUM(p.budget) AS total_budget
FROM Projects AS p
-- Находим исполнителя через принятую заявку
JOIN Bids AS b ON p.project_id = b.project_id AND b.status = 'accepted'
JOIN Freelancers AS f ON b.freelancer_id = f.user_id
WHERE
    p.status = 'completed'
    -- Логика для "прошлого месяца". DATE_TRUNC обрезает дату до начала месяца.
    AND p.completion_date >= DATE_TRUNC('month', NOW()) - INTERVAL '1 month'
    AND p.completion_date < DATE_TRUNC('month', NOW())
GROUP BY f.specialization
ORDER BY total_budget DESC;
```
