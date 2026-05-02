/*vector<AbstractDragon *> dragons = {
            new FireBreathingDragon("Беззубик", 2, 'm'),
            new IceDragon("Ледогрив", 4, 'f'),
            new FireBreathingDragon("Пламенный Шторм", 5, 'm')
    };*/
/*Указать в базовом классе AbstractDragon, что методы .show() и .useMagic(), а также деструктор — виртуальные
Мы как бы говорим компилятору: Если по указателю лежит класс-наследник, попробуй использовать его метод

В классах-наследниках указать, что мы перегружаем (override) эти родительские функции*/
/*полиморфизм - virtual и override. Как животное издаетт звук -> а уже разные виды животных разный звук*/
#define task1
#ifdef task1
#include <iostream>
#include <vector>
#include <string>

using namespace std;

struct Dish {
    string name;                 // Название блюда
    int calories;                // Калории
};

class AbstractDragon {
protected:
    string name;
    int age;
    int force;
    bool isMale;

public:
    AbstractDragon(string dragonName, int dragonAge, char dragonGender) {
        name = dragonName;
        age = dragonAge;
        force = 80 + rand() % 50;
        isMale = dragonGender == 'm';
    }

    virtual void show() {
        cout << "Дракон" << (isMale ? " " : "ица ") << name << " (" << age << ")" << endl;
        cout << "Сила: " << force << " HP" << endl;
    }

    virtual int useMagic() {
        cout << "Использована магия с силой " << force << endl;
        return force;
    }

    virtual ~AbstractDragon() {
        cout << "Дракон " << name << " улетел во взрослую жизнь..." << endl;
    }
};

class FireBreathingDragon : public AbstractDragon {
public:
    FireBreathingDragon(
            string dragonName, int dragonAge, char dragonGender
    ) : AbstractDragon(dragonName, dragonAge, dragonGender) {
    }

    void show() override {
        cout << "🔥";
        AbstractDragon::show();
    }

    int useMagic() override {
        cout << "Огненная атака! Очки урона: " << force << endl;
        return force;
    }

    void eat(Dish &dish) {
        cout << name << " eст " << dish.name << endl;
        if (dish.calories >= 1000000) {
            age++;
        }
    }
};


class IceDragon : public AbstractDragon {
public:
    IceDragon(string dragonName, int dragonAge, char dragonGender
    ) : AbstractDragon(dragonName, dragonAge, dragonGender) {
    };

    void show() override {
        cout << "❄️";
        AbstractDragon::show();
    }

    int useMagic() override {
        cout << "Ледяная атака! Очки урона: " << force << endl;
        return force;
    }
};

int main() {
    srand(time(nullptr)); // Делаем ПСЕВДОслучайные числа случайнее
    vector<AbstractDragon *> dragons = {
            new FireBreathingDragon("Беззубик", 2, 'm'),
            new IceDragon("Ледогрив", 4, 'f'),
            new FireBreathingDragon("Пламенный Шторм", 5, 'm')
    };

    for (AbstractDragon *dragon: dragons) {
        dragon->show();
        cout << endl;
    }

    for (AbstractDragon* dragon : dragons) {
        delete dragon; // Не забываем убирать за собой
    }

    return 0;
}

#endif

#ifdef task2
class FireBreathingDragon : public AbstractDragon {
public:
    FireBreathingDragon(
            string dragonName, int dragonAge, char dragonGender
    ) : AbstractDragon(dragonName, dragonAge, dragonGender) {
    }

    void show() override {
        cout << "🔥";
        AbstractDragon::show();
    }

    int useMagic() override {
        cout << "Огненная атака! Очки урона: " << force << endl;
        return force;
    }

    void eat(Dish &dish) {
        cout << name << " eст " << dish.name << endl;
        if (dish.calories >= 1000000) {
            age++;
        }
    }

    FireBreathingDragon operator+(FireBreathingDragon &anotherDragon) {
        if (isMale != anotherDragon.isMale) {
            string halfFatherName = name.substr(0, name.length() / 2 - 1);
            string halfMotherName = anotherDragon.name.substr(anotherDragon.name.length() / 2 + 1,
                                                              anotherDragon.name.length() / 2);
            char gender = (rand() % 2 == 0) ? 'm' : 'f';
            return FireBreathingDragon(halfFatherName + halfMotherName, 0, gender);
        }
    }

};

int main() {
    srand(time(nullptr)); // Делаем ПСЕВДОслучайные числа случайнее
    FireBreathingDragon dragonFierton("Фаэртон", 2, 'm');
    FireBreathingDragon dragonLuminia("Люминия", 2, 'f');
    dragonFierton.show();
    dragonLuminia.show();

    FireBreathingDragon dragonBaby = dragonFierton + dragonLuminia;

    dragonBaby.show();
    return 0;
}
#endif