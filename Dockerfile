# Use ARM64 Emscripten image
FROM emscripten/emsdk:4.0.3-arm64

# Install additional dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    nginx \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy the project files
COPY . .

# Create build directory
RUN mkdir -p build

# Build the project
RUN cd build && \
    emcmake cmake .. && \
    cmake --build .

# Configure nginx
RUN echo 'server { \
    listen 80; \
    server_name localhost; \
    root /app/build; \
    index index.html; \
    \
    location / { \
        include /etc/nginx/mime.types; \
        default_type application/octet-stream; \
        \
        types { \
            text/html html htm; \
            application/wasm wasm; \
            application/javascript js; \
            text/css css; \
        } \
        \
        try_files $uri $uri/ /index.html; \
    } \
    \
    location ~ \.(wasm|js|data)$ { \
        include /etc/nginx/mime.types; \
        add_header "Access-Control-Allow-Origin" "*"; \
        add_header "Access-Control-Allow-Methods" "GET, OPTIONS"; \
    } \
}' > /etc/nginx/conf.d/default.conf

# Remove default nginx configuration
RUN rm -f /etc/nginx/sites-enabled/default

# Create nginx cache directory
RUN mkdir -p /var/cache/nginx && \
    chown -R nginx:nginx /var/cache/nginx

# Expose port 80
EXPOSE 80

# Start nginx
CMD ["nginx", "-g", "daemon off;"] 