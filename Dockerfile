# Build stage
FROM emscripten/emsdk:4.0.3-arm64 AS builder

# Set working directory
WORKDIR /app

# Copy the project files
COPY . .

# Build the project
RUN mkdir -p build && \
    cd build && \
    emcmake cmake .. && \
    cmake --build .

# Serve stage
FROM nginx:stable-alpine

# Copy the built files to nginx
COPY --from=builder /app/build /usr/share/nginx/html

# Configure nginx for WebAssembly MIME types and default page
RUN echo 'server { \
    listen 80; \
    server_name localhost; \
    root /usr/share/nginx/html; \
    index index.html; \
    location / { \
        try_files $uri $uri/ /index.html; \
    } \
    types { \
        text/html html; \
        application/wasm wasm; \
        application/javascript js; \
    } \
}' > /etc/nginx/conf.d/default.conf

# Expose port 80
EXPOSE 80 