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
    location / { \
        try_files $uri $uri/ /index.html; \
        types { \
            application/wasm wasm; \
            application/javascript js; \
        } \
    } \
    location ~ \.(wasm|js|data)$ { \
        add_header "Access-Control-Allow-Origin" "*"; \
        add_header "Access-Control-Allow-Methods" "GET, OPTIONS"; \
        add_header "Content-Type" $content_type; \
    } \
}' > /etc/nginx/conf.d/default.conf

# Remove default nginx configuration
RUN rm -f /etc/nginx/sites-enabled/default

# Expose port 80
EXPOSE 80

# Start nginx
CMD ["nginx", "-g", "daemon off;"] 