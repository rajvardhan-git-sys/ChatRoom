# Deploy Chatroom Server

This plan outlines how we will containerize your C++ Boost.Asio chat server and deploy it to a live cloud provider so that anyone can connect to it from over the internet.

## User Review Required

> [!IMPORTANT]
> The most robust way to deploy a C++ application across different environments is using **Docker**. I propose writing a `Dockerfile` that compiles your server using a Linux environment (Ubuntu) and runs it.
> Once containerized, we can deploy this image to almost any modern cloud provider.

## Open Questions

> [!WARNING]
> Please let me know your preferences on the following:
> 1. **Cloud Provider:** Do you have a preferred hosting provider (e.g., Railway, Render, DigitalOcean, AWS, Google Cloud)? *If you don't have a preference, I recommend **Railway** or **Render** for a quick and free/cheap deployment.*
> 2. **Code modifications:** Some cloud providers automatically assign a port via an environment variable (e.g., `PORT`). Currently, your server expects the port as a command-line argument. I recommend slightly modifying `chatRoom.cpp` to check for the `PORT` environment variable as a fallback. Does that sound good?

## Proposed Changes

### Docker Configuration
We will add a `Dockerfile` and a `.dockerignore` file.

#### [NEW] [Dockerfile](file:///c:/Users/rajva/Desktop/chatroom/Dockerfile)
A configuration file to build a Linux container. It will:
- Use an `ubuntu` base image.
- Install `g++` and `libboost-all-dev`.
- Copy your source code (`chatRoom.cpp`, `chatRoom.hpp`, `message.hpp`).
- Compile the server executable.
- Set the entrypoint to run the server.

#### [NEW] [.dockerignore](file:///c:/Users/rajva/Desktop/chatroom/.dockerignore)
To avoid copying unnecessary files (like Windows `.exe` and `.obj` files) into the Docker build context.

### Server Application
#### [MODIFY] [chatRoom.cpp](file:///c:/Users/rajva/Desktop/chatroom/chatRoom.cpp)
We will add a small check in `main()`: if no port argument is passed in `argv`, it will attempt to read the `PORT` environment variable. If neither is present, it will default to `8080` (or exit). This makes it seamlessly compatible with cloud platforms.

## Verification Plan

### Local Verification
- We will build the Docker image locally.
- We will run the container locally mapping a port (e.g., `docker run -p 8080:8080 chatroom-server`).
- We will connect to it using your local `client.exe` to verify it works exactly as before.

### Live Verification
- Once deployed to a provider (e.g., Railway), we will get a live URL/IP and port.
- We will use the `client.exe` to connect to that live remote address and verify messages are broadcast successfully over the internet.
