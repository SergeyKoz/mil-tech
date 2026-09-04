#include <rclcpp/logging.hpp>
#include <rclcpp/rclcpp.hpp>
#include "ballistics_simulator/msg/drone_config.hpp"
#include "ballistics_simulator/msg/ammo_config.hpp"
#include "ballistics_simulator/msg/telemetry.hpp"
#include "ballistics_simulator/msg/target.hpp"
#include "ballistics_simulator/msg/control_command.hpp"
#include "ballistics_simulator/msg/drop_command.hpp"
#include "ballistics_simulator/msg/start_command.hpp"
#include "ballistics_simulator/state_qos.hpp"
#include "ballistics_simulator/checker_uart_listener.hpp"
#include "ballistics_simulator/checker_gpio_controller.hpp"
#include "interfaces/uart_listener_interface.hpp"

namespace
{
    constexpr auto kDroneConfigTopic = "/checker/drone_config";
    constexpr auto kAmmoConfigTopic = "/checker/ammo_config";
    constexpr auto kTelementyTopic = "/checker/telemetry";
    constexpr auto kTargetTopic = "/checker/target";
    constexpr auto kControlCommandTopic = "/checker/control_command";
    constexpr auto kDropCommandTopic = "/checker/drop_command";
    constexpr auto kStartCommandTopic = "/checker/start_command";
} // namespace

class CheckerDataProviderNode final : public rclcpp::Node, public IUartListener
{
public:
    CheckerDataProviderNode()
        : Node("checker_data_provider_node"), uartListener(ballistics_simulator::CheckerUARTListener(declare_parameter<std::string>("uart_port", ""))), gpioController(ballistics_simulator::CheckerGPIOController(declare_parameter<std::string>("gpio_chip_name", ""), declare_parameter<int>("start_line"), declare_parameter<int>("drop_line")))
    {
        const auto qos = rclcpp::QoS{10};

        const auto state_qos = ballistics_simulator::make_state_qos();

        droneConfigPublicher = create_publisher<ballistics_simulator::msg::DroneConfig>(kDroneConfigTopic, state_qos);
        ammoConfigPublicher = create_publisher<ballistics_simulator::msg::AmmoConfig>(kAmmoConfigTopic, state_qos);
        telemetryPublicher = create_publisher<ballistics_simulator::msg::Telemetry>(kTelementyTopic, state_qos);
        targetPublicher = create_publisher<ballistics_simulator::msg::Target>(kTargetTopic, state_qos);

        controlCommandSubscription = create_subscription<ballistics_simulator::msg::ControlCommand>(
            kControlCommandTopic, state_qos, [this](const ballistics_simulator::msg::ControlCommand &command)
            { on_control_command(command); });

        dropCommandSubscription = create_subscription<ballistics_simulator::msg::DropCommand>(
            kDropCommandTopic, state_qos, [this](const ballistics_simulator::msg::DropCommand &command)
            { on_drop_command(command); });

        startCommandSubscription = create_subscription<ballistics_simulator::msg::StartCommand>(
            kStartCommandTopic, state_qos, [this](const ballistics_simulator::msg::StartCommand &command)
            { on_start_command(command); });

        uartListener.setLogger([this](const std::string &msg)
                               { RCLCPP_INFO(this->get_logger(), "%s", msg.c_str()); });
        gpioController.setLogger([this](const std::string &msg)
                                 { RCLCPP_INFO(this->get_logger(), "%s", msg.c_str()); });

        // gpioset gpiochip0 24=1 --mode=time --sec=10
        gpioController.init();
        uartListener.init();
        uartListener.addListener(*this);
        uartListener.start();
    }

    auto updateTelemetry(const dlink::Telemetry &telemetry) -> void
    {
        // // PKT_TELEMETRY — те, що читає і парсить студент
        // struct Telemetry {
        //     uint32_t t_ms;  // час від старту, мілісекунди (таймстемп)
        //     float x, y;     // позиція дрона в площині, метри
        //     float z;        // висота (altitude), метри
        //     float vx, vy;   // швидкість у площині, м/с
        //     float speed;    // модуль горизонтальної швидкості, м/с
        //     float dir;      // курс (напрямок польоту), радіани
        //     uint8_t state;  // стан стейт-машини (0..4, як у DZ3)
        // };

        ballistics_simulator::msg::Telemetry msg;
        msg.t_ms = telemetry.t_ms;
        msg.x = telemetry.x;
        msg.y = telemetry.y;
        msg.z = telemetry.z;
        msg.vx = telemetry.vx;
        msg.vy = telemetry.vy;
        msg.speed = telemetry.speed;
        msg.dir = telemetry.dir;
        msg.state = telemetry.state;
        telemetryPublicher->publish(msg);

        RCLCPP_INFO(get_logger(),
                    "telemetry t=%d x,y,z=%.2f,%.2f,%.2f vx,vy,speed=%.2f,%.2f,%.2f dir=%.2f state=%d",
                    telemetry.t_ms,
                    telemetry.x,
                    telemetry.y,
                    telemetry.z,
                    telemetry.vx,
                    telemetry.vy,
                    telemetry.speed,
                    telemetry.dir,
                    telemetry.state);
    }

    auto updateTargetPosition(const dlink::TargetPos &targetPosition) -> void
    {
        // PKT_TARGET — позиція цілі «зараз» (ціль може рухатися)
        // struct TargetPos {
        //     uint8_t id;  // індекс цілі
        //     float x, y;  // поточна позиція цілі, метри
        // };

        ballistics_simulator::msg::Target msg;
        msg.id = targetPosition.id;
        msg.x = targetPosition.x;
        msg.y = targetPosition.y;
        targetPublicher->publish(msg);
    }

    auto updateAmmoConfig(const dlink::AmmoCfg &ammoConfig) -> void
    {
        // PKT_AMMO — конфіг пострілу (надсилається раз на старті)
        // struct AmmoCfg {
        //     char name[16];     // напр. "VOG-17"
        //     float mass;        // m
        //     float drag;        // d
        //     float lift;        // l
        //     float hitRadius;   // радіус успішного влучання, метри
        //     uint8_t nTargets;  // скільки цілей у місії
        // };

        RCLCPP_INFO(get_logger(),
                    "ammo_config m,d,l=%.2f,%.2f,%.2f hitRadius=%.2f targets=%d",
                    // ammoName,
                    ammoConfig.mass,
                    ammoConfig.drag,
                    ammoConfig.lift,
                    ammoConfig.hitRadius,
                    ammoConfig.nTargets);

        ballistics_simulator::msg::AmmoConfig msg;
        msg.name = ammoConfig.name;
        msg.mass = ammoConfig.mass;
        msg.drag = ammoConfig.drag;
        msg.lift = ammoConfig.lift;
        msg.hit_radius = ammoConfig.hitRadius;
        msg.targets = ammoConfig.nTargets;
        ammoConfigPublicher->publish(msg);
    }

    auto updateResult(const dlink::Result &result) -> void
    {
        // PKT_RESULT — вердикт (зворотний канал на залізі)
        // struct Result {
        //     uint8_t hit;         // 1 = влучив, 0 = промах
        //     uint8_t targetId;    // у яку ціль (або 0xFF)
        //     float miss_m;        // відстань промаху, метри
        //     uint32_t drop_t_ms;  // коли спрацював скид
        // };

        RCLCPP_INFO(get_logger(),
                    "result hit=%d targetId=%d miss_m=%.2f drop_t_ms=%d",
                    result.hit,
                    result.targetId,
                    result.miss_m,
                    result.drop_t_ms);
    }

    auto updateDroneConfig(const dlink::DroneCfg &droneConfig) -> void
    {
        // PKT_CONFIG — параметри місії з config (чекер шле студенту раз на старті, як AMMO).
        // Це ті поля config ДЗ9, яких немає в TELEMETRY/AMMO. position/altitude/dir беруться
        // з телеметрії, hitRadius і параметри боєприпасу — з AMMO.
        // struct DroneCfg {
        //     float attackSpeed;       // макс. швидкість дрона, м/с
        //     float accelerationPath;  // шлях розгону до attackSpeed, м (прискорення = v^2/(2*path))
        //     float angularSpeed;      // макс. кутова швидкість повороту, рад/с
        //     float turnThreshold;     // поріг кута повороту, рад
        //     float timeStep;          // крок симуляції, с
        //     float timeScale;  // прискорення симуляції (1 = реальний час; задається аргументом чекера)
        // };

        RCLCPP_INFO(get_logger(),
                    "drone config attackSpeed=%.2f accelerationPath=%.2f angularSpeed=%.2f turnThreshold=%.2f timeStep=%.2f timeScale=%.2f",
                    droneConfig.attackSpeed,
                    droneConfig.accelerationPath,
                    droneConfig.angularSpeed,
                    droneConfig.turnThreshold,
                    droneConfig.timeStep,
                    droneConfig.timeScale);

        ballistics_simulator::msg::DroneConfig msg;
        msg.attack_speed = droneConfig.attackSpeed;
        msg.acceleration_path = droneConfig.accelerationPath;
        msg.angular_speed = droneConfig.angularSpeed;
        msg.turn_threshold = droneConfig.turnThreshold;
        msg.time_step = droneConfig.timeStep;
        msg.time_scale = droneConfig.timeScale;
        droneConfigPublicher->publish(msg);
    }

    auto updateControl(const dlink::Control &control) -> void
    {
        // PKT_CONTROL — команда керування дроном (студент шле чекеру кожен такт)
        // Нормовані значення; чекер множить на фізичні ліміти дрона (maxAccel, maxTurnRate).
        // struct Control {
        //     float accel;     // прискорення вздовж курсу, [-1..1] (1 = повний газ, -1 = гальмо)
        //     float turnRate;  // швидкість повороту, [-1..1] (1 = макс. вліво, -1 = вправо)
        // };
    }

    void on_control_command(const ballistics_simulator::msg::ControlCommand &command)
    {
        // struct Control {
        //     float accel;     // прискорення вздовж курсу, [-1..1] (1 = повний газ, -1 = гальмо)
        //     float turnRate;  // швидкість повороту, [-1..1] (1 = макс. вліво, -1 = вправо)
        // };
        dlink::Control control = {.accel = command.acceleration, .turnRate = command.turn_rate};
        uartListener.writeControl(control);

        RCLCPP_INFO(this->get_logger(), "acceleration=%.2f turnRate=%.2f", command.acceleration, command.turn_rate);
    }

    void on_drop_command(const ballistics_simulator::msg::DropCommand &command)
    {
        gpioController.drop();
    }

    void on_start_command(const ballistics_simulator::msg::StartCommand &command)
    {
        gpioController.start();
    }

    ~CheckerDataProviderNode() override { uartListener.stop(); }

private:
    void execute_after_delay(int seconds)
    {
        std::thread([this, seconds]()
                    {
            std::this_thread::sleep_for(std::chrono::seconds(seconds));

            gpioController.start(); })
            .detach();
    }

    ballistics_simulator::CheckerUARTListener uartListener;
    ballistics_simulator::CheckerGPIOController gpioController;
    rclcpp::Publisher<ballistics_simulator::msg::DroneConfig>::SharedPtr droneConfigPublicher;
    rclcpp::Publisher<ballistics_simulator::msg::AmmoConfig>::SharedPtr ammoConfigPublicher;
    rclcpp::Publisher<ballistics_simulator::msg::Telemetry>::SharedPtr telemetryPublicher;
    rclcpp::Publisher<ballistics_simulator::msg::Target>::SharedPtr targetPublicher;
    rclcpp::Subscription<ballistics_simulator::msg::ControlCommand>::SharedPtr controlCommandSubscription;
    rclcpp::Subscription<ballistics_simulator::msg::DropCommand>::SharedPtr dropCommandSubscription;
    rclcpp::Subscription<ballistics_simulator::msg::StartCommand>::SharedPtr startCommandSubscription;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CheckerDataProviderNode>());
    rclcpp::shutdown();
    return 0;
}
