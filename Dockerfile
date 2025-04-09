# Build stage
FROM ubuntu:22.04 AS builder

# Install build essentials and cmake
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy the source code
COPY . .

# Clean any existing build artifacts and build the application
RUN rm -rf build && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make -j$(nproc)

# Runtime stage
FROM ubuntu:22.04

# Install only runtime dependencies
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy all binaries from builder stage
COPY --from=builder /app/build/kvdb /app/kvdb
COPY --from=builder /app/build/kvdb_client /app/kvdb_client
COPY --from=builder /app/build/server_tests /app/server_tests
COPY --from=builder /app/build/treap_tests /app/treap_tests

# Expose the port the server runs on
EXPOSE 8080

# Default command (will be overridden by docker-compose)
CMD ["./kvdb"]