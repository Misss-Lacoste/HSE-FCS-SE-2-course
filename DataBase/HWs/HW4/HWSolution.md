### task1
```sql
SELECT * FROM Sales.OrderDetails WHERE qty = (SELECT MAX(qty) FROM Sales.OrderDetails);
```

### task2
```sql
SELECT * FROM Sales.OrderDetails 
WHERE unitprice > (
    SELECT AVG(OD.unitprice)
    FROM Sales.OrderDetails OD
    JOIN Sales.Orders O ON O.orderid = OD.orderid
    JOIN HR.Employees E ON E.empid = O.empid
    WHERE E.lastname = 'Peled'
);
```

### task3
```sql
SELECT * FROM Sales.Customers
WHERE custid IN (SELECT DISTINCT custid FROM Sales.Orders);
```

### task4
```sql
SELECT * FROM Sales.Customers C
WHERE EXISTS (
    SELECT 1
    FROM Sales.Orders O
    WHERE O.custid = C.custid
);
```

### task5
```sql
SELECT E.lastname, E.firstname,
    (SELECT O.orderid FROM Sales.Orders O WHERE O.empid = E.empid LIMIT 1) AS orderid,
    (SELECT O.orderdate FROM Sales.Orders O WHERE O.empid = E.empid LIMIT 1) AS orderdate
FROM HR.Employees E;
```

### task6
```sql
SELECT * FROM Sales.OrderDetails
WHERE (unitprice * qty * (1 - discount)) > (
    SELECT AVG(unitprice * qty * (1 - discount))
    FROM Sales.OrderDetails
);
```

### task7
```sql
SELECT C.custid, C.contactname, O.orderdate
FROM Sales.Customers C
JOIN Sales.Orders O ON O.custid = C.custid
WHERE O.orderdate = (
    SELECT MAX(O2.orderdate)
    FROM Sales.Orders O2
    WHERE O2.custid = C.custid
);
```

### task8
```sql
SELECT C.categoryid, C.categoryname,
    (SELECT COUNT(*) FROM Production.Products P WHERE P.categoryid = C.categoryid) AS total_products
FROM Production.Categories C;
```