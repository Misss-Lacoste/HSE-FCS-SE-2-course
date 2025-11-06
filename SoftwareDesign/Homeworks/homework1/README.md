# Software Design | HW1
Author: Margarita Gorbacheva
### Description of the project "Bank account"
The task is to develop domain model classes for the key module of the future application, the Financial Accounting module. The domain model of classes should be implemented in compliance with the principles of SOLID, the key ideas of GRASP: High Cohesion and Low Coupling, as well as a number of GoF patterns: generative, structural and behavioral.
This project focuses on the financial accounting module. This section will include tools to simplify personal finance management: income and expense tracking, spending analysis, and budget planning. This approach will help users manage their funds more consciously and achieve their financial goals.

___Functions of the program:___
1. Create account
2. Delete account
3. Show accounts
4. Create category
5. Delete category
6. Show categories
7. Create operation
8. Delete operation
9. Show operations
10. Show operations with account id
11. Export data
12. Import data
13. Undo command
14. Redo command
15. Exit

### Project launch:
1. Open git repository
2. Clone repository
```bash
git clone https://github.com/Misss-Lacoste/Software-Design2
cd Programming/HSE-FCS-SE-2-year/SoftwareDesign/HW2
```
3. Open your IDE, launch the project using
```bash
g++ -std=c++17 *.cpp -o app.exe
```
and then
```bash
./app.exe
```
4. Enjoy the console output :)

### OOP & SOLID
The project follows SOLID and GRASP principles.
1. SRP(Single responsibility Principle) have been implemented in domain classes `BankAccount`, `Category`, `Operation`; in facades: `AccountFacade`, `CategoryService`, `OperationService`, `AnalyticsService`; and in pattern `DomainFactory`.
2. OCP (Open Closed Principle) in `Command`, `DataExportVisitor`, `DataImporter`.
3. LSP (Liskov Substitution Principle) different commands can be used everywhere where the `Command` is expected: `CreateAccountcommand`, `CreateCategoryCommand`.
4. ICP (Interface Segreagtion Principle) have been implemented in `Command`, `DataExportVisitor`, `AccountFacade`.
5. DIP(Dependency Inversion Principle) is implemented in dependency injections in `OperationService`, `CreateAccountCommand`.
### GRASP
1. __High Cohesion__ is realised in all operations of `AccountFacade`, `OperationService`, `AnalyticsService`.
`DomainFactory`, for instance, is responsible only for creating objects, while other methods are closely related to the operations with those objects.
The whole code is logically grouped.
2. __Low Coupling__ is for weak connections between services.

### Patterns:
1. __Facade__: there are 4 facades:
1.1.  `AccountFacade` - helps to simplify the accounts management. The class AccountFacade helps to create accounts, to get access to accounts via IDs, to delete accounts and to manage all accounts.
1.2. `CategoryService` - helps to simplify the operation categories. We can create categories, get category by type and to manage access to all categories.
1.3. `OperationService` - the facade for managing operations. We can coordinate complex interactions between accounts, categories and operations.
1.4. `AnalyticsService`- provides easy access to sophisticated financial analytics.
2. __Command__: there is realisation of several Command patterns when each user scenario is represented by a separate command. `Command`, 3`CreateAccountCommand`, `CreateCategoryCommand`, `CreateOperationCommand`, `AnalyzeFinancesCommand`.
3. __Decorator__: Wrapper for logging data during export. 
4. __Factory__: `Domainfactory` is to create accounts, categories, and operations.
5. __Visitor__: `DataExportVisitor` is to add new operations to objects without their classes modifications.
6. __Template Method__: `DataImporter` is for importing and exporting data in different formats.

__Functionality:__
1. Create accounts, delete accounts, and view all accounts (id, name, balance).
2. Create categories, delete categories, and view all categories (id, name, type). It is important to create at least one category before creating an operation.
3. Create operations, delete operations, view all operations, and view operations related to a specific account.
Export data in CSV, JSON, and YAML formats.
Import data in CSV and JSON formats.
Undo command.
Redo command.