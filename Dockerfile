FROM alpine:latest AS build

RUN apk add --no-cache build-base

COPY . /app
WORKDIR /app

RUN mkdir out

RUN gcc -static -Wall -Wextra -O2 -lsqlite3 src/chat.c src/user.c src/utils.c -o out/chat

FROM scratch

COPY --from=build /app/out/chat /chat
COPY --from=build /app/db/init.sql /init.sql
ENTRYPOINT ["/chat"]
