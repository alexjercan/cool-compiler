FROM python:3.11-alpine

RUN apk add --no-cache make

WORKDIR /app
COPY requirements.txt /app

RUN pip install --no-cache-dir -r requirements.txt

COPY . /app
