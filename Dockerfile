FROM alpine:latest

RUN apk add --no-cache clang make

WORKDIR /app
COPY . /app

RUN make
