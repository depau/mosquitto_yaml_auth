FROM eclipse-mosquitto:2.0
RUN apk add --no-cache make cmake g++ musl-dev binutils llvm13-dev llvm13-static mosquitto-dev pkgconf ncurses-dev ncurses-static

COPY . /sources
WORKDIR /sources

RUN rm -Rf build || true && \
    mkdir build && \
    cd build && \
    cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_SHARED_LINKER_FLAGS="-static-libgcc -static-libstdc++ -L/usr/lib/llvm13/lib" \
      .. && \
    make -j$(nproc)
