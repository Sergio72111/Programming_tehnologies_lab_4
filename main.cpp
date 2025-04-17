#include <iostream>     // Для вывода в консоль
#include <fstream>      // Для работы с файлами (лог в файл)
#include <string>       // Для работы со строками
#include <memory>       // Для использования std::shared_ptr и std::unique_ptr
#include <vector>       // Для хранения списка устройств

// === Интерфейс логгера ===
class ILogger {
public:
    virtual void Log(const std::string& message) = 0; // Чисто виртуальный метод логирования
    virtual ~ILogger() = default;                     // Виртуальный деструктор
};

// --- Реализация логгера: Консоль ---
class ConsoleLogger : public ILogger {
public:
    void Log(const std::string& message) override {
        std::cout << "[Console] " << message << "\n"; // Вывод сообщения в консоль
    }
};

// --- Реализация логгера: Файл ---
class FileLogger : public ILogger {
private:
    std::ofstream _file; // Поток для записи в файл

public:
    FileLogger(const std::string& filename) {
        _file.open(filename, std::ios::app); // Открываем файл для добавления (append)
    }

    void Log(const std::string& message) override {
        if (_file.is_open()) {
            _file << "[File] " << message << "\n"; // Записываем сообщение в файл
        }
    }

    ~FileLogger() {
        if (_file.is_open()) _file.close(); // Закрываем файл при уничтожении объекта
    }
};


class LoggerFactory {
public:
    // Перечисление типов логгеров
    enum LoggerType { Console, File };

    // Статический метод для создания нужного логгера
    static std::shared_ptr<ILogger> CreateLogger(LoggerType type) {
        if (type == Console) return std::make_shared<ConsoleLogger>();      // Консольный логгер
        if (type == File) return std::make_shared<FileLogger>("log.txt");   // Файловый логгер
        return nullptr; // Если неизвестный тип
    }
};

// === Модель предметной области ===


class AbstractElectricDevice {
protected:
    std::string _name;  // Название устройства
    double _power;      // Мощность устройства
    bool _isOn;         // Состояние (вкл/выкл)

public:
    AbstractElectricDevice(const std::string& name, double power)
        : _name(name), _power(power), _isOn(false) {} // Инициализация параметров

    virtual void TurnOn() { _isOn = true; }           
    virtual void TurnOff() { _isOn = false; }        
    virtual double GetPower() const { return _isOn ? _power : 0; } 
    virtual std::string GetInfo() const = 0;          // Абстрактный метод для получения информации
    virtual ~AbstractElectricDevice() = default;      // Виртуальный деструктор
};


class HomeAppliance : public AbstractElectricDevice {
protected:
    std::string _brand; // Бренд устройства

public:
    HomeAppliance(const std::string& name, double power, const std::string& brand)
        : AbstractElectricDevice(name, power), _brand(brand) {} // Конструктор с инициализацией
};


class PowerTool : public AbstractElectricDevice {
protected:
    double _voltage; // Напряжение питания

public:
    PowerTool(const std::string& name, double power, double voltage)
        : AbstractElectricDevice(name, power), _voltage(voltage) {} // Инициализация
};


class Refrigerator : public HomeAppliance {
private:
    double _capacity; // Объем холодильника

public:
    Refrigerator(const std::string& name, double power, const std::string& brand, double capacity)
        : HomeAppliance(name, power, brand), _capacity(capacity) {} // Конструктор

    std::string GetInfo() const override {
        // Возвращаем строку с полной информацией
        return "Refrigerator: " + _name + ", Brand: " + _brand +
               ", Capacity: " + std::to_string(_capacity) + "L, Power: " + std::to_string(_power);
    }
};


class Drill : public PowerTool {
private:
    int _rpm; // Обороты в минуту

public:
    Drill(const std::string& name, double power, double voltage, int rpm)
        : PowerTool(name, power, voltage), _rpm(rpm) {} // Конструктор

    std::string GetInfo() const override {
        // Возвращаем строку с информацией о дрели
        return "Drill: " + _name + ", Voltage: " + std::to_string(_voltage) +
               "V, RPM: " + std::to_string(_rpm) + ", Power: " + std::to_string(_power);
    }
};

// === Фабрика устройств (Factory Method) ===

// Интерфейс 
class DeviceFactory {
public:
    virtual std::unique_ptr<AbstractElectricDevice> Create() const = 0; // Метод создания устройства
    virtual ~DeviceFactory() = default;
};


class RefrigeratorFactory : public DeviceFactory {
public:
    std::unique_ptr<AbstractElectricDevice> Create() const override {
        // Создаем конкретный холодильник
        return std::make_unique<Refrigerator>("Samsung Fridge", 150, "Samsung", 300);
    }
};


class DrillFactory : public DeviceFactory {
public:
    std::unique_ptr<AbstractElectricDevice> Create() const override {
        // Создаем конкретную дрель
        return std::make_unique<Drill>("Bosch Drill", 800, 220, 3000);
    }
};

// === Логика приложения (не зависит от UI) ===

class DeviceManager {
private:
    std::vector<std::unique_ptr<AbstractElectricDevice>> _devices; // Список устройств
    std::shared_ptr<ILogger> _logger; // Логгер

public:
    DeviceManager(std::shared_ptr<ILogger> logger) : _logger(logger) {} // Конструктор

    void AddDevice(std::unique_ptr<AbstractElectricDevice> device) {
        _logger->Log("Добавлено устройство: " + device->GetInfo()); // Логируем добавление
        _devices.push_back(std::move(device)); // Добавляем в список
    }

    void TurnOnAll() {
        // Включаем все устройства
        for (auto& device : _devices) {
            device->TurnOn();
            _logger->Log("Включено: " + device->GetInfo()); // Логируем включение
        }
    }

    double GetTotalPower() const {
        // Считаем общую мощность включённых устройств
        double total = 0;
        for (const auto& device : _devices) {
            total += device->GetPower();
        }
        return total;
    }

    const std::vector<std::unique_ptr<AbstractElectricDevice>>& GetDevices() const {
        return _devices; // Возвращаем список устройств
    }
};

// === Интерфейс пользователя (отдельный класс) ===

class ConsoleUI {
private:
    DeviceManager& _manager;          // Ссылка на менеджер устройств
    std::shared_ptr<ILogger> _logger; // Логгер

public:
    ConsoleUI(DeviceManager& manager, std::shared_ptr<ILogger> logger)
        : _manager(manager), _logger(logger) {} // Конструктор

    void ShowDevices() const {
        // Показываем информацию об устройствах
        const auto& devices = _manager.GetDevices();
        std::cout << "Список устройств:\n";
        for (const auto& device : devices) {
            std::cout << device->GetInfo() << "\n";
        }
    }

    void ShowTotalPower() const {
        // Показываем суммарную мощность
        double total = _manager.GetTotalPower();
        std::cout << "Общая мощность: " << total << " W\n";
        _logger->Log("Общая мощность потребления: " + std::to_string(total) + " W"); // Лог
    }
};

// === Точка входа (main только делегирует) ===

int main() {
    // Выбор логгера (можно поменять на File)
    auto logger = LoggerFactory::CreateLogger(LoggerFactory::Console);

    // Создание менеджера логики
    DeviceManager manager(logger);

    // Используем фабрики
    RefrigeratorFactory fridgeFactory;
    DrillFactory drillFactory;

    // Добавление устройств
    manager.AddDevice(fridgeFactory.Create());
    manager.AddDevice(drillFactory.Create());

    // Включаем все устройства
    manager.TurnOnAll();

    // Интерфейс пользователя
    ConsoleUI ui(manager, logger);

    // Выводим информацию
    ui.ShowDevices();
    ui.ShowTotalPower();

    return 0; // Завершение программы
}
