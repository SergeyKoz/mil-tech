#include "UartReader.hpp"
#include "interfaces/IParser.hpp"
#include "common.hpp"
#include <iostream>
#include <fcntl.h>   // Помилки відкриття файлів (O_RDWR, O_NOCTTY, O_NDELAY)
#include <termios.h> // Структури та функції для налаштування системних портів
#include <unistd.h>  // UNIX стандартні функції (read, write, close)
#include <cstring>

UartReader::UartReader(const std::string &uartPort)
    : uartPort(uartPort)
{
}

auto UartReader::init() -> void
{
  int fd = open(uartPort.c_str(), O_RDWR | O_NOCTTY);

  if (fd < 0)
  {
    throw std::runtime_error("Помилка відкриття порту " + uartPort + "!");
  }

  // 2. Створюємо структуру налаштувань порту
  struct termios tty;

  if (tcgetattr(fd, &tty) != 0)
  {
    throw std::runtime_error("Помилка отримання параметрів порту!");
  }

  // 3. Налаштовуємо Baud Rate (переконайтеся, що на STM32 стоїть 115200)
  cfsetispeed(&tty, B115200);
  cfsetospeed(&tty, B115200);

  // 4. Налаштовуємо формат даних: 8 bits, no parity, 1 stop bit (8N1)
  tty.c_cflag &= ~PARENB; // Без парності (No parity)
  tty.c_cflag &= ~CSTOPB; // 1 стоп-біт
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;            // 8 біт даних
  tty.c_cflag &= ~CRTSCTS;       // Вимикаємо апаратний контроль потоку (RTS/CTS)
  tty.c_cflag |= CREAD | CLOCAL; // Увімкнути читання та ігнорувати лінії управління модемом

  // Налаштовуємо сирий (Raw) режим вводу
  tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);

  // Вимикаємо програмний контроль потоку та будь-яку трансляцію спецсимволів
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

  // Сирий вивід (без конвертації \n в \r\n)
  tty.c_oflag &= ~(OPOST | ONLCR);

  // 5. Таймаути читання (неблокуюче/мінімальне очікування)
  tty.c_cc[VMIN] = 1;  // Читати одразу, як з'явиться хоча б 1 байт
  tty.c_cc[VTIME] = 0; // Без додаткового таймауту

  // // Застосовуємо налаштування
  if (tcsetattr(fd, TCSANOW, &tty) != 0)
  {
    throw std::runtime_error("Помилка збереження налаштувань порту!");
  }

  uart = fd;
  isInited = true;
};

auto UartReader::receiveLoop() -> void
{
  // DEBUG("Mavlink RX Thread started.");

  char temp_buf[128];
  std::string input_buffer;

  while (isRunning)
  {

    std::memset(&temp_buf, 0, sizeof(temp_buf));
    int num_bytes = read(uart, temp_buf, sizeof(temp_buf));

    if (num_bytes < 0)
    {
      throw std::runtime_error("Помилка читання з порту!");

      break;
    }

    if (num_bytes > 0)
    {
      // Додаємо зчитані байти до накопичувального буфера
      input_buffer.append(temp_buf, num_bytes);

      // Перевіряємо, чи є в буфері символ перенесення рядка
      size_t pos;
      while ((pos = input_buffer.find('\n')) != std::string::npos)
      {
        std::string line = input_buffer.substr(0, pos);

        // Видаляємо \r, якщо STM32 надсилає \r\n
        if (!line.empty() && line.back() == '\r')
        {
          line.pop_back();
        }

        handleTelemetry(line);

        // Видаляємо виведений рядок з буфера
        input_buffer.erase(0, pos + 1);
      }
    }
  }

  DEBUG("Uart Thread stopped.");
}

auto UartReader::addParser(const std::shared_ptr<IParser> &parser) -> void
{
  parsers.push_back(parser);
}

auto UartReader::handleTelemetry(const std::string &telemetry) -> void
{
  auto it = parsers.begin();

  while (it != parsers.end())
  {
    if (auto parser = it->lock())
    {
      parser->parse(telemetry);

      ++it;
    }
    else
    {
      it = parsers.erase(it);
    }
  }
}

auto UartReader::start() -> void
{
  init();

  if (!isInited)
  {
    return;
  }

  isRunning = true;
  receiveThread = std::thread(&UartReader::receiveLoop, this);
}

auto UartReader::isThreadReady() const -> bool
{
  return isRunning;
}

auto UartReader::wait() -> void
{
  if (receiveThread.joinable())
  {
    receiveThread.join();
  }
}

auto UartReader::stop() -> void
{

  if (isRunning)
  {
    isRunning = false;

    if (uart >= 0)
    {
      close(uart);
      uart = -1;
    }
  }
}

UartReader::~UartReader()
{
  stop();
}
