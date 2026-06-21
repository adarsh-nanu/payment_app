FROM ubuntu:24.04

RUN apt update && apt install -y \
    cmake \
    build-essential \
    libdrogon-dev \
    libjsoncpp-dev \
    uuid-dev \
    libmysqlclient-dev \
    libyaml-cpp-dev \
    libpq-dev \
    libsqlite3-dev

RUN sed -i '/find_dependency(MySQL)/s/^/#/' \
    /usr/lib/aarch64-linux-gnu/cmake/Drogon/DrogonConfig.cmake && \
    sed -i '/find_dependency(Hiredis)/s/^/#/' \
    /usr/lib/aarch64-linux-gnu/cmake/Drogon/DrogonConfig.cmake && \
    sed -i '/find_dependency(Brotli)/s/^/#/' \
    /usr/lib/aarch64-linux-gnu/cmake/Drogon/DrogonConfig.cmake

WORKDIR /app

COPY . .

RUN cmake -B build
RUN cmake --build build

RUN ls -lrt /app

CMD ["./build/payment_app"]
