# Stage 1: Build the WebAssembly application
FROM --platform=linux/arm64 debian:bullseye-slim AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    git \
    cmake \
    python3 \
    nodejs \
    npm \
    wget \
    xz-utils \
    && rm -rf /var/lib/apt/lists/*

# Install Emscripten
WORKDIR /opt
RUN git clone https://github.com/emscripten-core/emsdk.git
WORKDIR /opt/emsdk
RUN ./emsdk install 3.1.45 && \
    ./emsdk activate 3.1.45

# Set up environment variables
ENV PATH="/opt/emsdk:/opt/emsdk/upstream/emscripten:/opt/emsdk/node/16.20.0_64bit/bin:${PATH}"

# Set working directory for the application
WORKDIR /app

# Copy the project files
COPY . .

# Create build directory and build the project
RUN mkdir -p build && \
    cd build && \
    emcmake cmake .. && \
    cmake --build .

# Stage 2: Create the runtime image
FROM --platform=linux/arm64 nginx:alpine

# Copy the built files from the builder stage
COPY --from=builder /app/build /usr/share/nginx/html

# Configure nginx
RUN echo 'server { \
    listen 80; \
    server_name localhost; \
    location / { \
        root /usr/share/nginx/html; \
        index index.html; \
        types { \
            application/wasm wasm; \
            application/javascript js; \
        } \
    } \
}' > /etc/nginx/conf.d/default.conf

# Expose port 80
EXPOSE 80

# Start nginx
CMD ["nginx", "-g", "daemon off;"] 