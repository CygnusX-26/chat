FROM alpine:latest AS build

RUN apk add --no-cache build-base

COPY . /src
WORKDIR /src

RUN gcc -static chat.c user.c -o chat

FROM scratch

COPY --from=build /src/chat /chat
ENTRYPOINT ["/chat"]
