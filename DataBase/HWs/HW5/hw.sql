USE testdatabase
GO

--основная таблица
IF OBJECT_ID('dbo.AfterTest', 'U') IS NOT NULL
DROP TABLE dbo.AfterTest
GO

CREATE TABLE dbo.AfterTest
(
    id INT,
    value DECIMAL(10,2),
    change_date DATETIME DEFAULT(GETDATE())
)
GO

-- логирование инсёрт
IF OBJECT_ID('dbo.AfterLogInsert', 'U') IS NOT NULL
DROP TABLE dbo.AfterLogInsert
GO

CREATE TABLE dbo.AfterLogInsert
(
    new_id INT,
    new_value DECIMAL(10,2),
    insert_date DATETIME DEFAULT(GETDATE()),
    [user] VARCHAR(70) DEFAULT(ORIGINAL_LOGIN()),
    action_type VARCHAR(20) DEFAULT 'INSERT'
)
GO

--логирования апдейт с вычисляемой колонкой
IF OBJECT_ID('dbo.AfterLogUpdate', 'U') IS NOT NULL
DROP TABLE dbo.AfterLogUpdate
GO

CREATE TABLE dbo.AfterLogUpdate
(
    old_id INT,
    old_value DECIMAL(10,2),
    new_id INT,
    new_value DECIMAL(10,2),
    value_difference DECIMAL(10,2),  --разница между старым и новым
    value_sum DECIMAL(10,2),          --сумма старого и нового
    update_date DATETIME DEFAULT(GETDATE()),
    [user] VARCHAR(70) DEFAULT(ORIGINAL_LOGIN()),
    action_type VARCHAR(20) DEFAULT 'UPDATE'
)
GO

--логирование DELETE
IF OBJECT_ID('dbo.AfterLogDelete', 'U') IS NOT NULL
DROP TABLE dbo.AfterLogDelete
GO

CREATE TABLE dbo.AfterLogDelete
(
    deleted_id INT,
    deleted_value DECIMAL(10,2),
    delete_date DATETIME DEFAULT(GETDATE()),
    [user] VARCHAR(70) DEFAULT(ORIGINAL_LOGIN()),
    action_type VARCHAR(20) DEFAULT 'DELETE'
)
GO
--TRIGGER AFTER INSERT
IF OBJECT_ID('dbo.AfterInsertTrigger', 'TR') IS NOT NULL
DROP TRIGGER dbo.AfterInsertTrigger
GO

CREATE TRIGGER dbo.AfterInsertTrigger ON dbo.AfterTest
AFTER INSERT
AS
BEGIN
    INSERT INTO dbo.AfterLogInsert (new_id, new_value)
    SELECT id, value FROM inserted
END
GO

-- TRIGGER AFTER UPDATE (с вычисляемыми колонками)
IF OBJECT_ID('dbo.AfterUpdateTrigger', 'TR') IS NOT NULL
DROP TRIGGER dbo.AfterUpdateTrigger
GO

CREATE TRIGGER dbo.AfterUpdateTrigger ON dbo.AfterTest
AFTER UPDATE
AS
BEGIN
    INSERT INTO dbo.AfterLogUpdate (old_id, old_value, new_id, new_value, value_difference, value_sum)
    SELECT 
        d.id,
        d.value,
        i.id,
        i.value,
        ABS(i.value - d.value),  --разница между новым и старым значением
        i.value + d.value         --сумма старого и нового значения
    FROM deleted d
    INNER JOIN inserted i ON d.id = i.id
END
GO

-- TRIGGER AFTER DELETE
IF OBJECT_ID('dbo.AfterDeleteTrigger', 'TR') IS NOT NULL
DROP TRIGGER dbo.AfterDeleteTrigger
GO

CREATE TRIGGER dbo.AfterDeleteTrigger ON dbo.AfterTest
AFTER DELETE
AS
BEGIN
    INSERT INTO dbo.AfterLogDelete (deleted_id, deleted_value)
    SELECT id, value FROM deleted
END
GO

-- 1. Проверка
INSERT INTO dbo.AfterTest (id, value) VALUES (1, 10.5)
INSERT INTO dbo.AfterTest (id, value) VALUES (2, 20.7)
INSERT INTO dbo.AfterTest (id, value) VALUES (3, 30.2)

SELECT * FROM dbo.AfterTest
SELECT * FROM dbo.AfterLogInsert
GO

-- 2.Проверка UPDATE
UPDATE dbo.AfterTest SET value = 15.5 WHERE id = 1
UPDATE dbo.AfterTest SET value = 45.9 WHERE id = 3

SELECT * FROM dbo.AfterTest
SELECT * FROM dbo.AfterLogUpdate
GO

DELETE FROM dbo.AfterTest WHERE id = 2

SELECT * FROM dbo.AfterTest
SELECT * FROM dbo.AfterLogDelete
GO

INSERT INTO dbo.AfterTest (id, value) VALUES (4, 100.0)
UPDATE dbo.AfterTest SET value = 75.0 WHERE id = 4

SELECT * FROM dbo.AfterLogUpdate WHERE old_id = 4
GO