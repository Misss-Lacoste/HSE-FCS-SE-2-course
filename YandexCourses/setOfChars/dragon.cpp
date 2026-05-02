//Базовый класс — это прадедушка-дракон. У него есть ДНК (поля) и навыки (методы)
//Класс-наследник — это внучок-дракончик. Он рождается с теми же генами, но может их улучшить

#include <iostream>
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

    void show() {
        cout << "Дракон" << (isMale ? " " : "ица ") << name << " (" << age << ")" << endl;
        cout << "Сила: " << force << " HP" << endl;
    }

    int useMagic() {
        cout << "Использована магия с силой " << force << endl;
        return force;
    }

    ~AbstractDragon() {
        cout << "Дракон " << name << " улетел во взрослую жизнь..." << endl;
    }
};

class FireBreathingDragon : public AbstractDragon {
public:
    FireBreathingDragon(
            string dragonName, int dragonAge, char dragonGender
    ) : AbstractDragon(dragonName, dragonAge, dragonGender) {
    }

    void show() {
        cout << "🔥";
        AbstractDragon::show();
    }

    int useMagic() {
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

int main() {
    srand(time(nullptr)); // Делаем ПСЕВДОслучайные числа случайнее
    FireBreathingDragon dragonToothless("Беззубик", 2, 'm');
    dragonToothless.show();
    Dish dragonSoulElixir = {"Эликсир пробуждения драконьей души", 123456789};
    Dish lightningInIce = {"Запечённая молния в кристаллах льда", 1200};
    dragonToothless.eat(lightningInIce);
    dragonToothless.eat(dragonSoulElixir);
    dragonToothless.show();
    dragonToothless.useMagic();
    return 0;
}