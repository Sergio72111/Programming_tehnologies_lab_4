#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <vector>

// === Интерфейс логгера ===
class ILogger {
public:
    virtual void Log(const std::string& message) = 0;
    virtual ~ILogger() = default;
};

// --- Реализация логгера: Консоль ---
class ConsoleLogger : public ILogger {
public:
    void Log(const std::string& message) override {
        std::cout << "[Console] " << message << "\n";
    }
};

// --- Реализация логгера: Файл ---
class FileLogger : public ILogger {
private:
    std::ofstream _file;

public:
    FileLogger(const std::string& filename) {
        _file.open(filename, std::ios::app);
    }

    void Log(const std::string& message) override {
        if (_file.is_open()) {
            _file << "[File] " << message << "\n";
        }
    }

    ~FileLogger() {
        if (_file.is_open()) _file.close();
    }
};

// --- Фабрика логгеров ---
class LoggerFactory {
public:
    enum LoggerType { Console, File };

    static std::shared_ptr<ILogger> CreateLogger(LoggerType type) {
        if (type == Console) return std::make_shared<ConsoleLogger>();
        if (type == File) return std::make_shared<FileLogger>("log.txt");
        return nullptr;
    }
};

// === Абстрактный класс электроприбора ===
class AbstractElectricDevice {
protected:
    std::string _name;
    int _power;
    bool _isOn;

public:
    AbstractElectricDevice(const std::string& name, int power)
        : _name(name), _power(power), _isOn(false) {}

    virtual void TurnOn() { _isOn = true; }
    virtual void TurnOff() { _isOn = false; }
    virtual int GetPower() const { return _isOn ? _power : 0; }
    virtual std::string GetInfo() const = 0;
    virtual ~AbstractElectricDevice() = default;
};

// --- Бытовая техника ---
class HomeAppliance : public AbstractElectricDevice {
protected:
    std::string _brand;

public:
    HomeAppliance(const std::string& name, int power, const std::string& brand)
        : AbstractElectricDevice(name, power), _brand(brand) {}
};

// --- Электроинструмент ---
class PowerTool : public AbstractElectricDevice {
protected:
    int _voltage;

public:
    PowerTool(const std::string& name, int power, int voltage)
        : AbstractElectricDevice(name, power), _voltage(voltage) {}
};

// --- Холодильник ---
class Refrigerator : public HomeAppliance {
private:
    int _capacity;

public:
    Refrigerator(const std::string& name, int power, const std::string& brand, int capacity)
        : HomeAppliance(name, power, brand), _capacity(capacity) {}

    std::string GetInfo() const override {
        return "Refrigerator: " + _name + ", Brand: " + _brand +
               ", Capacity: " + std::to_string(_capacity) + "L, Power: " + std::to_string(_power) + "W";
    }
};

// --- Дрель ---
class Drill : public PowerTool {
private:
    int _rpm;

public:
    Drill(const std::string& name, int power, int voltage, int rpm)
        : PowerTool(name, power, voltage), _rpm(rpm) {}

    std::string GetInfo() const override {
        return "Drill: " + _name + ", Voltage: " + std::to_string(_voltage) +
               "V, RPM: " + std::to_string(_rpm) + ", Power: " + std::to_string(_power) + "W";
    }
};

// === Интерфейс фабрики устройств ===
class DeviceFactory {
public:
    virtual std::unique_ptr<AbstractElectricDevice> Create() const = 0;
    virtual ~DeviceFactory() = default;
};

// --- Фабрика холодильников ---
class RefrigeratorFactory : public DeviceFactory {
public:
    std::unique_ptr<AbstractElectricDevice> Create() const override {
        return std::make_unique<Refrigerator>("Samsung Fridge", 150, "Samsung", 300);
    }
};

// --- Фабрика дрелей ---
class DrillFactory : public DeviceFactory {
public:
    std::unique_ptr<AbstractElectricDevice> Create() const override {
        return std::make_unique<Drill>("Bosch Drill", 800, 220, 3000);
    }
};

// === Класс логики приложения ===
class DeviceManager {
private:
    std::vector<std::unique_ptr<AbstractElectricDevice>> _devices;
    std::shared_ptr<ILogger> _logger;

public:
    DeviceManager(std::shared_ptr<ILogger> logger) : _logger(logger) {}

    void AddDevice(std::unique_ptr<AbstractElectricDevice> device) {
        _logger->Log("Добавлено устройство: " + device->GetInfo());
        _devices.push_back(std::move(device));
    }

    void TurnOnAll() {
        for (auto& device : _devices) {
            device->TurnOn();
            _logger->Log("Включено: " + device->GetInfo());
        }
    }

    int GetTotalPower() const {
        int total = 0;
        for (const auto& device : _devices) {
            total += device->GetPower();
        }
        return total;
    }

    const std::vector<std::unique_ptr<AbstractElectricDevice>>& GetDevices() const {
        return _devices;
    }
};

// === Интерфейс пользователя ===
class ConsoleUI {
private:
    DeviceManager& _manager;
    std::shared_ptr<ILogger> _logger;

public:
    ConsoleUI(DeviceManager& manager, std::shared_ptr<ILogger> logger)
        : _manager(manager), _logger(logger) {}

    void ShowDevices() const {
        const auto& devices = _manager.GetDevices();
        std::cout << "\nСписок устройств:\n";
        for (const auto& device : devices) {
            std::cout << device->GetInfo() << "\n";
        }
    }

    void ShowTotalPower() const {
        int total = _manager.GetTotalPower();
        std::cout << "Общая мощность: " << total << " W\n";
        _logger->Log("Общая мощность потребления: " + std::to_string(total) + " W");
    }
};

// === Точка входа (main) ===
int main() {
    auto logger = LoggerFactory::CreateLogger(LoggerFactory::Console);

    DeviceManager manager(logger);

    RefrigeratorFactory fridgeFactory;
    DrillFactory drillFactory;

    manager.AddDevice(fridgeFactory.Create());
    manager.AddDevice(drillFactory.Create());

    manager.TurnOnAll();

    ConsoleUI ui(manager, logger);
    ui.ShowDevices();
    ui.ShowTotalPower();

    return 0;
}
