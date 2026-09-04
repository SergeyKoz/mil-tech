#include <rclcpp/logging.hpp>
#include <rclcpp/rclcpp.hpp>
#include "ballistics_simulator/msg/cell_observation.hpp"
#include "ballistics_simulator/msg/enemy_down.hpp"
#include "ballistics_simulator/msg/local_scan.hpp"
#include "ballistics_simulator/msg/move_command.hpp"
#include "ballistics_simulator/msg/student_status.hpp"
#include "ballistics_simulator/srv/payload_trigger.hpp"
#include "ballistics_simulator/state_qos.hpp"
#include "ballistics_simulator/world_explorer.hpp"
#include "ballistics_simulator/checker_uart_listener.hpp"
#include "interfaces/uart_listener_interface.hpp"

// /home/dev/mil-tech/coursework/robot_ws/ballistics_simulator/include/interfaces/uart_listener_interface.hpp

namespace {

// constexpr auto kScanTopic = "/robot/local_scan";
// constexpr auto kMoveTopic = "/robot/cmd_move";
// constexpr auto kEnemyDownTopic = "/payload/enemy_down";
// constexpr auto kStatusTopic = "/student/status";
// constexpr auto kTriggerService = "/payload/trigger";

// constexpr auto kScanTopic = "/checker/local_scan";
// "--uart",
// "/dev/ttyAMA2",
// "--gpiochip",
// "gpiochip0",
// "--start-line",
// "24",
// "--drop-line",
// "23"
// socat -d -d pty,raw,echo=0,link=/tmp/ttyA pty,raw,echo=0,link=/tmp/ttyB

}  // namespace

class CheckerDataProviderNode final : public rclcpp::Node, public IUartListener {
  public:
    CheckerDataProviderNode()
        : Node("checker_data_provider_node")
        , uartListener(ballistics_simulator::CheckerUARTListener("/tmp/ttyA"))  /// dev/ttyAMA2
    {
        const auto qos = rclcpp::QoS{10};

        const auto state_qos = ballistics_simulator::make_state_qos();

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

        // DEBUG("TELEMETRY: " << " state=" << static_cast<int>(telemetry.state) << " x,y=" << telemetry.x << "," << telemetry.y << " "
        // vx,vy="
        //                     << telemetry.vx << "," << telemetry.vy << " dir=" << telemetry.dir << " speed=" << telemetry.speed);

        // currentTime = static_cast<float>(telemetry.t_ms) / 1000.0F;  // мілісекунди -> секунди

        // auto droneConfig = configLoader->getConfig();

        // if (!isConfigured) {
        //     droneConfig.startPos = {telemetry.x, telemetry.y};
        //     droneConfig.altitude = telemetry.z;
        //     droneConfig.initialDir = telemetry.dir;

        //     dynamic_cast<CheckerConfigLoader *>(configLoader.get())->setConfig(droneConfig);
        //     LOG("Drone is ready to start the mission!");

        //     isConfigured = isDroneConfigReady(droneConfig);

        //     if (!isConfigured) {
        //         return;
        //     }
        // }

        // if (!isTargetsDefined) {
        //     isTargetsDefined = dynamic_cast<CheckerTargetProvider *>(targetProvider.get())->isReady();

        //     if (!isTargetsDefined) {
        //         return;
        //     }
        // }

        // if (isConfigured && isTargetsDefined && !isDropParametersCalculated) {
        //     dropParams = solver->calcDropParameters(droneConfig.ammo, droneConfig.attackSpeed, droneConfig.altitude);
        //     isDropParametersCalculated = true;

        //     DEBUG("Drop parameters calculated: time=" << dropParams.time << ", distance=" << dropParams.distance);

        //     targetSelector->init(droneConfig);

        //     context = std::make_unique<DroneContext>(
        //         DroneContext{.currentTime = 0.F,
        //                      .simulationStep = {},
        //                      .droneConfig = &droneConfig,
        //                      .dronePhysics = {},
        //                      .droneTelemetry = {.state = STOPPED,
        //                                         .position = {telemetry.x, telemetry.y},
        //                                         .altitude = droneConfig.altitude,
        //                                         .speed = {telemetry.vx, telemetry.vy},
        //                                         .direction = telemetry.dir,
        //                                         .timeSinceStart = static_cast<float>(telemetry.t_ms) / 1000.0F},
        //                      .dropParams = &dropParams,
        //                      .selectedTarget = {},
        //                      .turnAngle = 0.F,
        //                      .acceleration = droneConfig.acceleration(),
        //                      .angleStep = droneConfig.angularSpeed,
        //                      .distanceToDropPoint = 0.F});
        // }

        // if (isConfigured && isTargetsDefined && isDropParametersCalculated) {
        //     context->droneTelemetry = DroneTelemetry{.state = context->droneTelemetry.state,
        //                                              .position = {telemetry.x, telemetry.y},
        //                                              .altitude = droneConfig.altitude,
        //                                              .speed = {telemetry.vx, telemetry.vy},
        //                                              .direction = telemetry.dir,
        //                                              .timeSinceStart = static_cast<float>(telemetry.t_ms) / 1000.0F};

        //     auto simulationStep = calculateSimulationStep();

        //     context->simulationStep = simulationStep.get();

        //     if (isTargetHit(*context)) {
        //         throw TargetHit(std::to_string(context->simulationStep->targetIdx));
        //     }

        //     auto command = states[context->droneTelemetry.state](*targetSelector)->threadExecute(*context);
        //     context->droneTelemetry.state = command.state;
        //     DEBUG("Command: " << command.state << " acc: " << command.acceleration << " ang: " << command.angleSpeed
        //                       << " max: " << command.maxSpeed);

        //     float turnPosition = command.angleSpeed > epsilon ? 1.0F : -1.0F;
        //     float turnRate = std::abs(command.angleSpeed) < epsilon ? 0.0F : turnPosition;

        //     float accelPosition = command.state == ACCELERATING ? 1.0F : -1.0F;
        //     float accel = command.state != ACCELERATING && command.state != DECELERATING ? 0.F : accelPosition;

        //     rpiCheckerUART->writeControl({.accel = accel, .turnRate = turnRate});
        // }
    }

    auto updateTargetPosition(const dlink::TargetPos &targetPosition) -> void
    {
        // PKT_TARGET — позиція цілі «зараз» (ціль може рухатися)
        // struct TargetPos {
        //     uint8_t id;  // індекс цілі
        //     float x, y;  // поточна позиція цілі, метри
        // };

        RCLCPP_INFO(get_logger(), "target id=%d x,y=%.2f,%.2f", targetPosition.id, targetPosition.x, targetPosition.y);

        // auto *targets = dynamic_cast<CheckerTargetProvider *>(targetProvider.get());

        // targets->setTarget(targetPosition.id, {targetPosition.x, targetPosition.y}, currentTime);
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
                    "ammo_config name=%s m,d,l=%.2f,%.2f,%.2f hitRadius=%.2f targets=%d",
                    ammoConfig.name,
                    ammoConfig.mass,
                    ammoConfig.drag,
                    ammoConfig.lift,
                    ammoConfig.hitRadius,
                    ammoConfig.nTargets);

        // if (isConfigured) {
        //     return;
        // }

        // auto droneConfig = configLoader->getConfig();

        // droneConfig.hitRadius = ammoConfig.hitRadius;
        // droneConfig.ammo = {.name = &ammoConfig.name[0], .mass = ammoConfig.mass, .drag = ammoConfig.drag, .lift = ammoConfig.lift};

        // dynamic_cast<CheckerConfigLoader *>(configLoader.get())->setConfig(droneConfig);
        // dynamic_cast<CheckerTargetProvider *>(targetProvider.get())->setTargetsCount(ammoConfig.nTargets);

        // isConfigured = isDroneConfigReady(droneConfig);
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

        // DEBUG("Result: (hit: " << result.hit << " miss: " << result.miss_m << " targetId: " << result.targetId << ")");
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

        // if (isConfigured) {
        //     return;
        // }

        // auto config = configLoader->getConfig();

        // config.attackSpeed = droneConfig.attackSpeed;
        // config.accelerationPath = droneConfig.accelerationPath;
        // config.angularSpeed = droneConfig.angularSpeed;
        // config.turnThreshold = droneConfig.turnThreshold;
        // config.simTimeStep = droneConfig.timeStep;
        // config.timeScale = droneConfig.timeScale;

        // dynamic_cast<CheckerConfigLoader *>(configLoader.get())->setConfig(config);

        // isConfigured = isDroneConfigReady(config);
    }

    auto updateControl(const dlink::Control &control) -> void
    {
        // PKT_CONTROL — команда керування дроном (студент шле чекеру кожен такт)
        // Нормовані значення; чекер множить на фізичні ліміти дрона (maxAccel, maxTurnRate).
        // struct Control {
        //     float accel;     // прискорення вздовж курсу, [-1..1] (1 = повний газ, -1 = гальмо)
        //     float turnRate;  // швидкість повороту, [-1..1] (1 = макс. вліво, -1 = вправо)
        // };

        RCLCPP_INFO(get_logger(), "control accel=%.2f turnRate=%.2f", control.accel, control.turnRate);

        // DEBUG("Control command: (accel:" << control.accel << " turnRate:" << control.turnRate << ")");
    }

    ~CheckerDataProviderNode() override { uartListener.stop(); }

  private:
    // void on_local_scan(const ballistics_simulator::msg::LocalScan& localScan)
    // {
    // }

    // rclcpp::Subscription<ballistics_simulator::msg::LocalScan>::SharedPtr localScanSubscription;
    // rclcpp::Publisher<ballistics_simulator::msg::MoveCommand>::SharedPtr movePublicher;
    // rclcpp::Publisher<ballistics_simulator::msg::EnemyDown>::SharedPtr enemyDownPublicher;
    // rclcpp::Publisher<ballistics_simulator::msg::StudentStatus>::SharedPtr statePublicher;
    // rclcpp::Client<ballistics_simulator::srv::PayloadTrigger>::SharedPtr triggerClient;

    // auto rpiCheckerUART = std::make_unique<RpiCheckerUART>(cliParams.uartPort);

    ballistics_simulator::CheckerUARTListener uartListener;

    // ballistics_simulator::WorldExplorer worldExplorer;
    // WorldExplorerState state = WorldExplorerState::EXPLORING;
    // std::queue<ballistics_simulator::MoveDirection> returnMovements;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CheckerDataProviderNode>());
    rclcpp::shutdown();
    return 0;
}
