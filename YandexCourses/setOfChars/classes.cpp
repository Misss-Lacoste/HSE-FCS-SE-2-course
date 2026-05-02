#include <string>

class ElectronicDevice {
private:
    bool isOn_ = false;

public:
    void turnOn() {
        isOn_ = true;
    }

    void turnOff() {
        isOn_ = false;
    }

    bool isOn() const {
        return isOn_;
    }

    virtual ~ElectronicDevice() = default;
};

class Phone : public ElectronicDevice {
private:
    std::string phoneNumber_;

public:
    Phone(const std::string& phoneNumber) : phoneNumber_(phoneNumber) {}

    std::string call(const std::string& number) {
        if (isOn()) {
            return "Calling " + number + " from " + phoneNumber_;
        } else {
            return "Device is off";
        }
    }

    std::string getPhoneNumber() const {
        return phoneNumber_;
    }
};

class Smartphone : public Phone {
private:
    std::string operatingSystem_;

public:
    Smartphone(const std::string& phoneNumber, const std::string& operatingSystem)
        : Phone(phoneNumber), operatingSystem_(operatingSystem) {}

    std::string installApp(const std::string& appName) {
        if (isOn()) {
            return "Installing " + appName + " on " + operatingSystem_;
        } else {
            return "Device is off";
        }
    }

    std::string getOS() const {
        return operatingSystem_;
    }
};
