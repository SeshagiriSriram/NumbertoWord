# ==========================================
# STAGE 1: The Build and Development Arena
# ==========================================
FROM ubuntu:24.04 AS builder

# Prevent prompt hangs during build phases
ENV DEBIAN_FRONTEND=noninteractive

# Install essential compilers and tracking tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    gcc \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src

# Copy all C project source code into the build arena
COPY . .

# Compile the application cleanly 
# (This assumes a simple compilation. If they use Make or CMake, swap this out)
RUN gcc -o my_c_app main.c

# ==========================================
# STAGE 2: The Secure Production Target Layer
# ==========================================
# Use a minimal base image. We do not need compilers (gcc) in production!
FROM ubuntu:24.04 AS runtime

WORKDIR /app

# Securely pull ONLY the compiled executable binary out of Stage 1
# This keeps the final shipping image tiny and hides original source code from users
COPY --from=builder /src/my_c_app /app/my_c_app

# Run as a non-privileged system user for DevSecOps best practices
USER 1000

# Execute the application when the container fires up
CMD ["./my_c_app"]
