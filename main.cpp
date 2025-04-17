#include <iostream>     
#include <fstream>      
#include <string>       
#include <memory>       
#include <vector>      

// === Интерфейс логгера ===
class ILogger {
public:
    virtual void Log(const std::string& message) = 0; // Чисто виртуальный метод логирования, реализуется в наследниках
    virtual ~ILogger() = default;                     // Виртуальный деструктор
};

// --- Реализация логгера: Консоль ---
class ConsoleLogger : public ILogger {
public:
    void Log(const std::string& message) override {
        std::cout << "[Console] " << message << "\n"; // Вывод сообщения в консоль с префиксом [Console]
    }
};

// --- Реализация логгера: Файл ---
class FileLogger : public ILogger {
private:
    std::ofstream _file; // Поток для записи в файл

public:
    FileLogger(const std::string& filename) {
        _file.open(filename, std::ios::app); // Открытие файла для дозаписи (append)
    }

    void Log(const std::string& message) override {
        if (_file.is_open()) {
            _file << "[File] " << message << "\n"; // Запись сообщения в файл с префиксом [File]
        }
    }

    ~FileLogger() {
        if (_file.is_open()) _file.close(); // Закрытие файла при уничтожении объекта
    }
};

// --- логгеры ---
class LoggerFactory {
public:
    enum LoggerType { Console, File }; // Перечисление возможных типов логгера

    static std::shared_ptr<ILogger> CreateLogger(LoggerType type) {
        if (type == Console) return std::make_shared<ConsoleLogger>();      // Возврат консольного логгера
        if (type == File) return std::make_shared<FileLogger>("log.txt");  // Возврат файлового логгера
        return nullptr; // В случае неизвестного типа возвращается nullptr
    }
};

// === Абстрактный класс электроприбора ===
class AbstractElectricDevice {
protected:
    std::string _name;  // Название устройства
    double _power;      // Потребляемая мощность
    bool _isOn;         // Состояние (включено/выключено)

public:
    AbstractElectricDevice(const std::string& name, double power)
        : _name(name), _power(power), _isOn(false) {} // Инициализация полей

    virtual void TurnOn() { _isOn = true; }           // Метод включения прибора
    virtual void TurnOff() { _isOn = false; }         // Метод выключения прибора
    virtual double GetPower() const { return _isOn ? _power : 0; } // Возвращает мощность, если включен
    virtual std::string GetInfo() const = 0;          // Абстрактный метод для получения инфо об устройстве
    virtual ~AbstractElectricDevice() = default;      // Виртуальный деструктор
};

// --- Бытовая техника ---
class HomeAppliance : public AbstractElectricDevice {
protected:
    std::string _brand; // Бренд устройства

public:
    HomeAppliance(const std::string& name, double power, const std::string& brand)
        : AbstractElectricDevice(name, power), _brand(brand) {} // Конструктор с инициализацией
};

// --- Электроинструмент ---
class PowerTool : public AbstractElectricDevice {
protected:
    double _voltage; // Напряжение питания

public:
    PowerTool(const std::string& name, double power, double voltage)
        : AbstractElectricDevice(name, power), _voltage(voltage) {} // Конструктор
};

// --- Холодильник ---
class Refrigerator : public HomeAppliance {
private:
    double _capacity; // Объем в литрах

public:
    Refrigerator(const std::string& name, double power, const std::string& brand, double capacity)
        : HomeAppliance(name, power, brand), _capacity(capacity) {} // Конструктор

    std::string GetInfo() const override {
        return "Refrigerator: " + _name + ", Brand: " + _brand +
               ", Capacity: " + std::to_string(_capacity) + "L, Power: " + std::to_string(_power);
    }
};

// --- Дрель ---
class Drill : public PowerTool {
private:
    int _rpm; // Обороты в минуту

public:
    Drill(const std::string& name, double power, double voltage, int rpm)
        : PowerTool(name, power, voltage), _rpm(rpm) {} // Конструктор

    std::string GetInfo() const override {
        return "Drill: " + _name + ", Voltage: " + std::to_string(_voltage) +
               "V, RPM: " + std::to_string(_rpm) + ", Power: " + std::to_string(_power);
    }
};

// === Интерфейс  устройств ===
class DeviceFactory {
public:
    virtual std::unique_ptr<AbstractElectricDevice> Create() const = 0; // Метод создания устройства
    virtual ~DeviceFactory() = default;
};

// ---  холодильников ---
class RefrigeratorFactory : public DeviceFactory {
public:
    std::unique_ptr<AbstractElectricDevice> Create() const override {
        return std::make_unique<Refrigerator>("Samsung Fridge", 150, "Samsung", 300); // Создание конкретного холодильника
    }
};

// ---  дрелей ---
class DrillFactory : public DeviceFactory {
public:
    std::unique_ptr<AbstractElectricDevice> Create() const override {
        return std::make_unique<Drill>("Bosch Drill", 800, 220, 3000); // Создание конкретной дрели
    }
};

// === Класс логики приложения ===
class DeviceManager {
private:
    std::vector<std::unique_ptr<AbstractElectricDevice>> _devices; // Список всех устройств
    std::shared_ptr<ILogger> _logger; // Логгер

public:
    DeviceManager(std::shared_ptr<ILogger> logger) : _logger(logger) {} // Конструктор

    void AddDevice(std::unique_ptr<AbstractElectricDevice> device) {
        _logger->Log("Добавлено устройство: " + device->GetInfo()); // Логируем добавление
        _devices.push_back(std::move(device)); // Добавляем в список
    }

    void TurnOnAll() {
        for (auto& device : _devices) {
            device->TurnOn();
            _logger->Log("Включено: " + device->GetInfo()); // Логируем включение
        }
    }

    double GetTotalPower() const {
        double total = 0;
        for (const auto& device : _devices) {
            total += device->GetPower(); // Суммируем мощность включённых устройств
        }
        return total;
    }

    const std::vector<std::unique_ptr<AbstractElectricDevice>>& GetDevices() const {
        return _devices; // Возвращаем список устройств (только для чтения)
    }
};

// === Интерфейс пользователя ===
class ConsoleUI {
private:
    DeviceManager& _manager;          // Ссылка на менеджер устройств
    std::shared_ptr<ILogger> _logger; // Логгер

public:
    ConsoleUI(DeviceManager& manager, std::shared_ptr<ILogger> logger)
        : _manager(manager), _logger(logger) {} // Конструктор

    void ShowDevices() const {
        const auto& devices = _manager.GetDevices();
        std::cout << "\nСписок устройств:\n";
        for (const auto& device : devices) {
            std::cout << device->GetInfo() << "\n"; // Вывод информации о каждом устройстве
        }
    }

    void ShowTotalPower() const {
        double total = _manager.GetTotalPower(); // Получаем общую мощность
        std::cout << "Общая мощность: " << total << " W\n";
        _logger->Log("Общая мощность потребления: " + std::to_string(total) + " W"); // Лог
    }
};

// === Точка входа (main) ===
int main() {
    auto logger = LoggerFactory::CreateLogger(LoggerFactory::Console); // Создаем логгер (можно File)

    DeviceManager manager(logger); // Менеджер логики с логгером

    RefrigeratorFactory fridgeFactory; // Фабрика холодильников
    DrillFactory drillFactory;         // Фабрика дрелей

    manager.AddDevice(fridgeFactory.Create()); // Добавляем холодильник
    manager.AddDevice(drillFactory.Create());  // Добавляем дрель

    manager.TurnOnAll(); // Включаем все устройства

    ConsoleUI ui(manager, logger); // UI

    ui.ShowDevices();    // Показываем устройства
    ui.ShowTotalPower(); // Показываем мощность

    return 0;
}
