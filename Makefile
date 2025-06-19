# --- CÁC BIẾN ĐỊNH NGHĨA ---
TARGET = shooter_game
CXX = g++

# --- CỜ BIÊN DỊCH VÀ LIÊN KẾT ---

# Lấy các cờ cho trình biên dịch (đường dẫn include) từ pkg-config
# Ví dụ: -I/usr/include/SDL2
SDL_CFLAGS = $(shell pkg-config --cflags sdl2 SDL2_image SDL2_mixer)

# Lấy các cờ cho trình liên kết (thư viện cần liên kết) từ pkg-config
# Ví dụ: -lSDL2 -lSDL2_image -lSDL2_mixer
SDL_LIBS = $(shell pkg-config --libs sdl2 SDL2_image SDL2_mixer)

# Cờ chung cho trình biên dịch C++
# SỬA ĐỔI QUAN TRỌNG: Thêm $(SDL_CFLAGS) vào đây
CXXFLAGS = -std=c++11 -Wall -Iinclude $(SDL_CFLAGS)

# Các thư viện khác cần liên kết
LIBS = -lm


# --- TỰ ĐỘNG TÌM KIẾM TỆP ---
SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)


# --- CÁC QUY TẮC BIÊN DỊCH ---

all: $(TARGET)

# Quy tắc để xây dựng tệp thực thi cuối cùng
# SỬA ĐỔI QUAN TRỌNG: Sử dụng $(SDL_LIBS) thay vì $(SDL_FLAGS)
$(TARGET): $(OBJECTS)
	@echo "Linking..."
	$(CXX) $(OBJECTS) -o $(TARGET) $(SDL_LIBS) $(LIBS)
	@echo "Build complete! Run with ./${TARGET}"

# Quy tắc mẫu để biên dịch một tệp .cpp thành một tệp .o
# Quy tắc này giờ đã đúng vì $(CXXFLAGS) chứa cờ của SDL
src/%.o: src/%.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Quy tắc để dọn dẹp dự án
clean:
	@echo "Cleaning up..."
	rm -f $(TARGET) $(OBJECTS)
	@echo "Cleanup complete!"