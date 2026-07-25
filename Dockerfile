FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libboost-all-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/app

COPY chatRoom.cpp chatRoom.hpp message.hpp ./

# Compile the server
RUN g++ -std=c++17 chatRoom.cpp -o chatRoom -lboost_system -lpthread

# The port environment variable default
ENV PORT=8080

EXPOSE $PORT

CMD ["./chatRoom"]
