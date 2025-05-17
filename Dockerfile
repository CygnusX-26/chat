FROM gcc:latest

RUN apt-get update && apt-get install -y make libsqlite3-dev && rm -rf /var/lib/apt/lists/*
WORKDIR /app

COPY . .

RUN make

CMD ["./out/chat"]
