### task 1
```sql
CREATE TABLE Authors (
    author_id SERIAL PRIMARY KEY,
    full_name VARCHAR(150) NOT NULL,
    birth_year INTEGER
);
```
### task2

```sql
CREATE TABLE Books(
    book_id SERIAL PRIMARY KEY,
    title VARCHAR(200), NOT NULL,
    publication_year INTEGER,
    author_id INTEGER NOT NULL,
    FOREIGN KEY (author_id) REFERENCES Authors(author_id) ON DELETE CASCADE --чоб при удалении записи author, удалить и дочернюю запись book
)
```
*Теперь запрос сделаем*
```sql
INSERT INTO Authors (full_name, birth_year) VALUES ('Александр Пушкин', 1799);

INSERT INTO Books (title, publication_year, author_id)
VALUES ('Евгений Онегин', 1833, (SELECT author_id FROM Authors WHERE full_name = 'Александр Пушкин'));
```
### task3
**
```sql
ALTER TABLE Books DROP CONSTRAINT books_author_id_fkey;

ALTER TABLE Books ALTER COLUMN author_id DROP NOT NULL;
```
*Новая колонка genre*
```sql
ALTER TABLE Books ADD COLUMN genre VARCHAR(50);
```
*INSERT запрос для добавления обеих книг*
```sql
INSERT INTO Books (title, author_id, genre, publication_year)
VALUES
    ('Сказки народов мира', NULL, 'Фольклор', 1980),
    ('Пиковая дама', (SELECT author_id FROM Authors WHERE full_name = 'Александр Пушкин'), 'Повесть', 1834);
```

### task4
*Создадим таблицу*
```sql
CREATE TABLE Reviews (
    review_id SERIAL PRIMARY KEY,
    book_id INTEGER NOT NULL,
    reviewer_name VARCHAR(100),
    rating INTEGER NOT NULL CHECK (rating >= 1 AND rating <= 5),
    review_text TEXT,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    FOREIGN KEY (book_id) REFERENCES Books(book_id) ON DELETE CASCADE
);
```
*Запрос с insert*
```sql
INSERT INTO Reviews (book_id, reviewer_name, rating, review_text)
VALUES (
    (SELECT book_id FROM Books WHERE title = 'Евгений Онегин'),
    'Белинский',
    5,
    'Энциклопедия русской жизни'
)
RETURNING review_id, created_at;

--SELECT review_id, created_at
--FROM Reviews
--WHERE reviewer_name = 'Белинский' AND book_id = 5;
--вместо returninig можно
```
### task5
*Добавление столбца average_rating в таблицу Authors*
```sql
ALTER TABLE Authors ADD COLUMN average_rating NUMERIC(4,2);
```
*Транзакция с UPDATE для расчёта и обновления рейтинга:*
```sql
BEGIN TRANSACTION;
UPDATE Authors
SET average_rating = subquery.avg_rating
FROM (
    SELECT
        a.author_id,
        AVG(r.rating) AS avg_rating
    FROM Authors a
    LEFT JOIN Books b ON a.author_id = b.author_id
    LEFT JOIN Reviews r ON b.book_id = r.book_id
    GROUP BY a.author_id
) AS subquery
WHERE Authors.author_id = subquery.author_id;
COMMIT;
```

