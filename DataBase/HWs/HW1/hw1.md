### 1. ER-diagram
**Сущности:**
Branch - branch_id, city.
Employee - employee_id, branch_id (связь с филиалом).
Trip - trip_id, employee_id, branch_id (начальный филиал), planned_start_date, fact_start_date, planned_end_date, fact_end_date, task, cancel_date.
Flight - flight_id, trip_id (связь с поездкой), origin_city, destination_city, airline, flight_number, departure_datetime, ticket_price.
Hotel - hotel_id, name, address, city, room_charge.
Hotel_reservation -  reservation_id, trip_id (связь с поездкой), hotel_id (связь с отелем), checkin_date, checkout_date.

**Связи:**
Branch 1–M Employee (один филиал — много сотрудников).
Branch 1–M Trip (один филиал — много поездок из него).
Employee 1–M Trip (один сотрудник — много поездок).
Trip 1–M Flight (одна поездка — два рейса: вылет и прилёт).
Trip 1–1 Hotel_reservation (одна поездка — одно бронирование отеля).
Hotel_reservation 1–1 Hotel (одно бронирование — один отель).

### 2. DDL
```sql
CREATE TABLE Branch (
    branch_id INT PRIMARY KEY,
    city VARCHAR(100) NOT NULL
);

CREATE TABLE Employee (
    employee_id INT PRIMARY KEY,
    branch_id INT,
    FOREIGN KEY (branch_id) REFERENCES Branch(branch_id)
);

CREATE TABLE Trip (
    trip_id INT PRIMARY KEY,
    employee_id INT,
    branch_id INT,
    planned_start_date DATE,
    fact_start_date DATE,
    planned_end_date DATE,
    fact_end_date DATE,
    task TEXT,
    cancel_date DATE,
    FOREIGN KEY (employee_id) REFERENCES Employee(employee_id),
    FOREIGN KEY (branch_id) REFERENCES Branch(branch_id)
);

CREATE TABLE Flight (
    flight_id INT PRIMARY KEY,
    trip_id INT,
    origin_city VARCHAR(100),
    destination_city VARCHAR(100),
    airline VARCHAR(100),
    flight_number VARCHAR(20),
    departure_datetime DATETIME,
    ticket_price DECIMAL(10, 2),
    FOREIGN KEY (trip_id) REFERENCES Trip(trip_id)
);

CREATE TABLE Hotel (
    hotel_id INT PRIMARY KEY,
    name VARCHAR(100),
    address TEXT,
    city VARCHAR(100),
    room_charge DECIMAL(10, 2)
);

CREATE TABLE Hotel_reservation (
    reservation_id INT PRIMARY KEY,
    trip_id INT,
    hotel_id INT,
    checkin_date DATE,
    checkout_date DATE,
    FOREIGN KEY (trip_id) REFERENCES Trip(trip_id),
    FOREIGN KEY (hotel_id) REFERENCES Hotel(hotel_id)
);
```

### 3. SQL - Paris - Berlin
```sql
SELECT h.hotel_id, h.name, h.address
FROM Hotel h
JOIN Hotel_reservation hr ON h.hotel_id = hr.hotel_id
JOIN Trip t ON hr.reservation_id = t.trip_id
JOIN Employee e ON t.employee_id = e.employee_id
JOIN Branch b ON e.branch_id = b.branch_id
WHERE h.city = 'Paris'
  AND b.city = 'Berlin'
  AND YEAR(t.planned_start_date) = YEAR(CURDATE());
```
### 4.
```sql
SELECT h.city, SUM(h.room_charge * DATEDIFF(hr.checkout_date, hr.checkin_date)) AS total_spent
FROM Hotel h
JOIN Hotel_reservation hr ON h.hotel_id = hr.hotel_id
JOIN Trip t ON hr.reservation_id = t.trip_id
WHERE MONTH(t.planned_start_date) = MONTH(CURDATE()) - 1
  AND YEAR(t.planned_start_date) = YEAR(CURDATE())
GROUP BY h.city;
```
### 5.
```sql
SELECT b.city, SUM(h.room_charge * DATEDIFF(hr.checkout_date, hr.checkin_date)) AS total_spent
FROM Hotel h
JOIN Hotel_reservation hr ON h.hotel_id = hr.hotel_id
JOIN Trip t ON hr.reservation_id = t.trip_id
JOIN Employee e ON t.employee_id = e.employee_id
JOIN Branch b ON e.branch_id = b.branch_id
WHERE MONTH(t.planned_start_date) = MONTH(CURDATE()) - 1
  AND YEAR(t.planned_start_date) = YEAR(CURDATE())
GROUP BY b.city;
```
### 6.
```sql
WITH cancelled_trips AS (
    SELECT b.branch_id, b.city, COUNT(*) AS cancellation_count
    FROM Trip t
    JOIN Branch b ON t.branch_id = b.branch_id
    WHERE DATEDIFF(t.cancel_date, t.planned_start_date) <= 3
      AND t.cancel_date IS NOT NULL
    GROUP BY b.branch_id, b.city
)
SELECT city, cancellation_count
FROM cancelled_trips
ORDER BY cancellation_count DESC
LIMIT 3;
```
### 7.
```sql
SELECT DISTINCT e1.employee_id AS employee1, e2.employee_id AS employee2
FROM Employee e1
JOIN Trip t1 ON e1.employee_id = t1.employee_id
JOIN Flight f1 ON t1.trip_id = f1.trip_id
JOIN Employee e2 ON e2.employee_id <> e1.employee_id
JOIN Trip t2 ON e2.employee_id = t2.employee_id
JOIN Flight f2 ON t2.trip_id = f2.trip_id
WHERE f1.origin_city = f2.origin_city
  AND f1.destination_city = f2.destination_city
  AND DATE(f1.departure_datetime) = DATE(f2.departure_datetime)
  AND f1.flight_number <> f2.flight_number
  AND t1.planned_start_date = t2.planned_start_date;
```