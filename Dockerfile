# Build stage
FROM ubuntu:22.04 AS builder

# Install build essentials and cmake
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    && rm -rf /var/lib/apt/lists/*


WORKDIR /app

COPY . .

# Clean any existing build artifacts and build the application
RUN rm -rf build && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make -j$(nproc)

# Runtime stage
FROM ubuntu:22.04

# Install only runtime dependencies. Not installing entire ubuntu
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy all binaries from builder stage (Though we ended up needing this setup only for server)
COPY --from=builder /app/build/kvdb /app/kvdb
COPY --from=builder /app/build/kvdb_client /app/kvdb_client
COPY --from=builder /app/build/server_tests /app/server_tests
COPY --from=builder /app/build/treap_tests /app/treap_tests

# Expose the port the server runs on
EXPOSE 8080

# Default command (will be overridden by docker-compose. We are mostly interested in serve)
CMD ["./kvdb"]