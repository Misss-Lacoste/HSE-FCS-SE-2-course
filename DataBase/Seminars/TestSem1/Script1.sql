create table SOTRUDNIKI
(id numeric (5) not null,
surname    VARCHAR(100),
firstname  VARCHAR(50),
secondname VARCHAR(50),
birth      DATE,
dept_id    NUMERIC(5));

create table OTDEL
(id   NUMERIC(5) not null,
name VARCHAR(100));

create table ADDRESS
(id        NUMERIC(5),
num_house VARCHAR(100),
emp_id    NUMERIC(5),
street    VARCHAR(20));