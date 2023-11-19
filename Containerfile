FROM docker.io/rustlang/rust:nightly-alpine AS builder
RUN apk add bash

COPY src /build/src
COPY ["LICENSE", "Cargo.lock", "Cargo.toml", "container/build.sh", "/build"]
WORKDIR /build
RUN bash ./build.sh

FROM docker.io/library/alpine
RUN apk add --no-cache musl dcron
COPY --from=builder /dist /dist
COPY container /dist
WORKDIR /dist
ENTRYPOINT ["/dist/run.sh"]
