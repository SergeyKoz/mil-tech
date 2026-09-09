# Базовий образ ROS 2 Jazzy для linux/arm64
FROM ros:jazzy-ros-core

# Аргументи для налаштування користувача та робочої папки (передаються з devcontainer.json)
ARG CONTAINER_USER=pi
ARG USER_UID=1000
ARG USER_GID=1000
ARG WORKSPACE_FOLDER=/home/pi/mil-tech

# Встановлюємо неінтерактивний режим для apt
ENV DEBIAN_FRONTEND=noninteractive

# Встановлення базових пакетів розробки та утиліт
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    gdb \
    clang-format \
    clangd \
    python3-colcon-common-extensions \
    python3-rosdep \
    python3-pip \
    sudo \
    curl \
    mc \
    libgpiod-dev \
    gpiod \
    # ros-jazzy-rqt \
    # ros-jazzy-rqt-common-plugins \
    && rm -rf /var/lib/apt/lists/*

# Якщо користувача pi немає, перейменовуємо стандартного ubuntu (UID 1000) на pi
RUN if ! id -u ${CONTAINER_USER} >/dev/null 2>&1; then \
        usermod -l ${CONTAINER_USER} -d /home/${CONTAINER_USER} -m ubuntu && \
        groupmod -n ${CONTAINER_USER} ubuntu; \
    fi && \
    echo "${CONTAINER_USER} ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Автоматичний сорсинг середовища ROS 2 під час відкриття нових терміналів
RUN echo "source /opt/ros/jazzy/setup.bash" >> /etc/bash.bashrc

# Ініціалізація rosdep (якщо ще не ініціалізовано)
RUN if [ ! -f /etc/ros/rosdep/sources.list.d/20-default.list ]; then \
        rosdep init; \
    fi

RUN mkdir -p ${WORKSPACE_FOLDER} && chown -R ${USER_UID}:${USER_GID} ${WORKSPACE_FOLDER}

# Робоча директорія всередині контейнера
WORKDIR ${WORKSPACE_FOLDER}

# Перемикаємося на не-root користувача за замовчуванням
USER ${CONTAINER_USER}

# За замовчуванням запускаємо bash
CMD ["/bin/bash"]