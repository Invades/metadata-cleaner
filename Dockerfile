FROM alpine:3.19 AS builder

RUN apk add --no-cache \
    clang \
    cmake \
    ninja \
    git \
    taglib-dev \
    musl-dev

WORKDIR /src
COPY . .

RUN cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_C_COMPILER=clang \
    && cmake --build build --config Release

FROM alpine:3.19

RUN apk add --no-cache taglib libstdc++

COPY --from=builder /src/build/metadata-cleaner /usr/local/bin/

RUN mkdir -p /data

WORKDIR /data

ENTRYPOINT ["metadata-cleaner", "--config", "/data/config.json"]
